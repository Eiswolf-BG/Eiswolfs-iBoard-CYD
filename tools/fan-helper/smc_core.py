"""Minimale SMC-Luefter-Zugriffsschicht fuer den Fan-Helper-Dienst
(fand.py). Struct-Layout und Ablauf 1:1 aus tools/fan-test/fan_control.py
uebernommen (dort wiederum gegen den offenen Referenzcode von
MacFanControl verifiziert, siehe CLAUDE.md) - bewusst eine eigene Kopie
statt eines Imports aus tools/fan-test, damit der Hilfsdienst
unabhaengig von diesem Test-/Scratch-Ordner bleibt.
"""
import ctypes
import ctypes.util
import time

KERNEL_INDEX_SMC = 2
CMD_READ_BYTES = 5
CMD_WRITE_BYTES = 6
CMD_READ_KEYINFO = 9
RESULT_KEY_NOT_FOUND = 0x84
KIO_NOT_PRIVILEGED = 0xE00002C1 - (1 << 32)  # als signed int32


class SMCVersion(ctypes.Structure):
    _fields_ = [
        ("major", ctypes.c_uint8),
        ("minor", ctypes.c_uint8),
        ("build", ctypes.c_uint8),
        ("reserved", ctypes.c_uint8),
        ("release", ctypes.c_uint16),
    ]


class SMCPLimitData(ctypes.Structure):
    _fields_ = [
        ("version", ctypes.c_uint16),
        ("length", ctypes.c_uint16),
        ("cpu_plimit", ctypes.c_uint32),
        ("gpu_plimit", ctypes.c_uint32),
        ("mem_plimit", ctypes.c_uint32),
    ]


class SMCKeyInfoData(ctypes.Structure):
    _fields_ = [
        ("data_size", ctypes.c_uint32),
        ("data_type", ctypes.c_uint32),
        ("data_attributes", ctypes.c_uint8),
    ]


class SMCKeyData(ctypes.Structure):
    _fields_ = [
        ("key", ctypes.c_uint32),
        ("vers", SMCVersion),
        ("p_limit", SMCPLimitData),
        ("key_info", SMCKeyInfoData),
        ("result", ctypes.c_uint8),
        ("status", ctypes.c_uint8),
        ("data8", ctypes.c_uint8),
        ("data32", ctypes.c_uint32),
        ("bytes", ctypes.c_uint8 * 32),
    ]


assert ctypes.sizeof(SMCKeyData) == 80, (
    f"SMCKeyData ist {ctypes.sizeof(SMCKeyData)} Bytes, erwartet 80 - "
    "Struct-Layout stimmt auf diesem System nicht, ABBRUCH ohne jeden SMC-Zugriff."
)


def fourcc(s):
    b = s.encode("ascii")
    if len(b) != 4:
        raise ValueError(f"SMC-Schluessel muss 4 Zeichen haben: {s!r}")
    return int.from_bytes(b, "big")


def fourcc_str(v):
    return v.to_bytes(4, "big").decode("ascii", errors="replace")


class SMCError(Exception):
    pass


class NotPrivileged(SMCError):
    pass


class KeyNotFound(SMCError):
    pass


class Smc:
    def __init__(self):
        iokit = ctypes.CDLL("/System/Library/Frameworks/IOKit.framework/IOKit")
        self._iokit = iokit
        iokit.IOServiceMatching.restype = ctypes.c_void_p
        iokit.IOServiceMatching.argtypes = [ctypes.c_char_p]
        iokit.IOServiceGetMatchingService.restype = ctypes.c_uint32
        iokit.IOServiceGetMatchingService.argtypes = [ctypes.c_uint32, ctypes.c_void_p]
        iokit.IOServiceOpen.restype = ctypes.c_int32
        iokit.IOServiceOpen.argtypes = [ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(ctypes.c_uint32)]
        iokit.IOServiceClose.restype = ctypes.c_int32
        iokit.IOServiceClose.argtypes = [ctypes.c_uint32]
        iokit.IOObjectRelease.restype = ctypes.c_int32
        iokit.IOObjectRelease.argtypes = [ctypes.c_uint32]
        iokit.IOConnectCallStructMethod.restype = ctypes.c_int32
        iokit.IOConnectCallStructMethod.argtypes = [
            ctypes.c_uint32, ctypes.c_uint32,
            ctypes.c_void_p, ctypes.c_size_t,
            ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t),
        ]

        libsystem_path = ctypes.util.find_library("System")
        libsystem = ctypes.CDLL(libsystem_path)
        self._mach_task_self = ctypes.c_uint32.in_dll(libsystem, "mach_task_self_").value

        matching = iokit.IOServiceMatching(b"AppleSMC")
        if not matching:
            raise SMCError("AppleSMC-Service nicht gefunden")
        service = iokit.IOServiceGetMatchingService(0, matching)
        if not service:
            raise SMCError("AppleSMC-Service nicht gefunden (GetMatchingService)")
        conn = ctypes.c_uint32(0)
        kr = iokit.IOServiceOpen(service, self._mach_task_self, 0, ctypes.byref(conn))
        iokit.IOObjectRelease(service)
        if kr != 0:
            raise SMCError(f"IOServiceOpen fehlgeschlagen (kern {kr:#x}) - laeuft der Dienst als root?")
        self.conn = conn.value
        self._info_cache = {}

    def close(self):
        if getattr(self, "conn", None):
            self._iokit.IOServiceClose(self.conn)
            self.conn = None

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()

    def _call(self, inp: SMCKeyData) -> SMCKeyData:
        out = SMCKeyData()
        out_len = ctypes.c_size_t(ctypes.sizeof(SMCKeyData))
        kr = self._iokit.IOConnectCallStructMethod(
            self.conn, KERNEL_INDEX_SMC,
            ctypes.byref(inp), ctypes.sizeof(inp),
            ctypes.byref(out), ctypes.byref(out_len),
        )
        if kr == KIO_NOT_PRIVILEGED:
            raise NotPrivileged("Zugriff verweigert - braucht Root")
        if kr != 0:
            raise SMCError(f"SMC-Aufruf fehlgeschlagen (kern {kr:#x})")
        if out.result == RESULT_KEY_NOT_FOUND:
            raise KeyNotFound()
        if out.result != 0:
            raise SMCError(f"SMC-Fehlercode {out.result:#x}")
        return out

    def key_info(self, key_str):
        if key_str in self._info_cache:
            return self._info_cache[key_str]
        inp = SMCKeyData()
        inp.key = fourcc(key_str)
        inp.data8 = CMD_READ_KEYINFO
        out = self._call(inp)
        info = (out.key_info.data_size, fourcc_str(out.key_info.data_type))
        self._info_cache[key_str] = info
        return info

    def exists(self, key_str):
        try:
            self.key_info(key_str)
            return True
        except SMCError:
            return False

    def read(self, key_str):
        size, dtype = self.key_info(key_str)
        inp = SMCKeyData()
        inp.key = fourcc(key_str)
        inp.key_info.data_size = size
        inp.data8 = CMD_READ_BYTES
        out = self._call(inp)
        return bytes(out.bytes)[:size], dtype

    def write(self, key_str, data: bytes):
        size, dtype = self.key_info(key_str)
        if len(data) != size:
            raise SMCError(f"Schluessel {key_str}: erwartet {size} Bytes, bekommen {len(data)}")
        inp = SMCKeyData()
        inp.key = fourcc(key_str)
        inp.key_info.data_size = size
        inp.data8 = CMD_WRITE_BYTES
        for i, b in enumerate(data):
            inp.bytes[i] = b
        self._call(inp)

    def read_f32(self, key_str):
        data, dtype = self.read(key_str)
        if dtype == "flt " and len(data) >= 4:
            return ctypes.c_float.from_buffer_copy(data[:4]).value
        if dtype == "fpe2" and len(data) >= 2:
            return int.from_bytes(data[:2], "big") / 4.0
        if dtype == "ui8 ":
            return float(data[0])
        if dtype == "ui16" and len(data) >= 2:
            return float(int.from_bytes(data[:2], "big"))
        raise SMCError(f"Unbekannter Zahlentyp {dtype!r} fuer {key_str}")

    def read_u8(self, key_str):
        data, dtype = self.read(key_str)
        return data[0] if data else 0

    def encode_rpm(self, tg_key, rpm):
        _, dtype = self.key_info(tg_key)
        if dtype == "flt ":
            return bytes(ctypes.c_float(rpm))
        if dtype == "fpe2":
            v = int(max(0.0, min(16383.0, rpm)) * 4.0)
            return v.to_bytes(2, "big")
        raise SMCError(f"Unbekannter Zieldrehzahl-Typ {dtype!r} fuer {tg_key}")


def discover_fans(smc: Smc):
    try:
        data, dtype = smc.read("FNum")
        count = data[0] if dtype == "ui8 " else int.from_bytes(data, "big")
    except SMCError:
        count = 0

    fans = []
    for i in range(count):
        md_key = None
        for cand in (f"F{i}md", f"F{i}Md"):
            if smc.exists(cand):
                md_key = cand
                break
        try:
            mn = smc.read_f32(f"F{i}Mn")
        except SMCError:
            mn = 0.0
        try:
            mx = smc.read_f32(f"F{i}Mx")
        except SMCError:
            mx = 6000.0
        if not (0.0 <= mn < mx):
            mn, mx = 0.0, 6000.0
        fans.append({"index": i, "min": mn, "max": mx, "md_key": md_key})
    return fans


def fan_status(smc: Smc, fan):
    i = fan["index"]
    actual = smc.read_f32(f"F{i}Ac")
    target = smc.read_f32(f"F{i}Tg")
    mode = "AUTO"
    if fan["md_key"]:
        try:
            mode = "MANUAL" if smc.read_u8(fan["md_key"]) == 1 else "AUTO"
        except SMCError:
            pass
    return {"actual": actual, "target": target, "mode": mode}


def set_auto(smc: Smc, fan):
    """Lueft auf Automatik zurueck: Modus-Schluessel auf 0, danach
    Ziel-Drehzahl 0 schreiben (macOS ignoriert das im Automatikmodus)."""
    i = fan["index"]
    if fan["md_key"]:
        try:
            smc.write(fan["md_key"], bytes([0]))
        except SMCError:
            pass
    try:
        data = smc.encode_rpm(f"F{i}Tg", 0.0)
        smc.write(f"F{i}Tg", data)
    except SMCError:
        pass
    if smc.exists("Ftst"):
        try:
            smc.write("Ftst", bytes([0]))
        except SMCError:
            pass


def engage_manual(smc: Smc, fan):
    """Manuellen Modus aktivieren - auf M3/M4 muss dafuer zuerst der
    'Ftst'-Freigabeschluessel gesetzt werden (siehe ERGEBNISSE.md)."""
    i = fan["index"]
    if fan["md_key"] and smc.read_u8(fan["md_key"]) == 1:
        return
    try:
        if fan["md_key"]:
            smc.write(fan["md_key"], bytes([1]))
            return
        raise SMCError("kein Modus-Schluessel gefunden")
    except SMCError:
        if not smc.exists("Ftst"):
            raise
        for _ in range(100):
            try:
                smc.write("Ftst", bytes([1]))
                break
            except SMCError:
                time.sleep(0.05)
        time.sleep(3.0)
        for _ in range(300):
            try:
                if fan["md_key"]:
                    smc.write(fan["md_key"], bytes([1]))
                return
            except SMCError:
                time.sleep(0.1)
        raise SMCError("manueller Modus liess sich nach Freigabe nicht aktivieren")


def set_target_rpm(smc: Smc, fan, rpm):
    """Nur nach oben (siehe fand.py::cmd_boost()), nie ueber fan['max']."""
    rpm = max(fan["min"], min(fan["max"], rpm))
    engage_manual(smc, fan)
    data = smc.encode_rpm(f"F{fan['index']}Tg", rpm)
    for _ in range(10):
        try:
            smc.write(f"F{fan['index']}Tg", data)
            return rpm
        except SMCError:
            time.sleep(0.05)
    raise SMCError("Zieldrehzahl liess sich nicht schreiben")
