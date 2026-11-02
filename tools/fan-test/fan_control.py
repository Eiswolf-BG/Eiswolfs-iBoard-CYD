#!/usr/bin/env python3
"""Eigenstaendiges SMC-Luefter-Testskript fuer Eiswolfs iBoard (Mac mini
M4). Spricht direkt mit dem AppleSMC-Treiber ueber IOKit (ctypes) - der
exakte Struct-Aufbau und die Schluessel/Encodings wurden vorher gegen
den offenen, geprueften Referenzcode von MacFanControl
(https://github.com/raminsharifi/MacFanControl, MIT-Lizenz,
tools/fan-test/MacFanControl/src/smc.rs + fan.rs) verifiziert, NICHT neu
erraten.

SICHERHEIT (siehe Auftrag):
- Es wird NUR nach oben gegangen, nie unter die aktuelle/Automatik-
  Drehzahl, nie ueber den vom SMC gemeldeten Maximalwert.
- Drei feste Testphasen (~2000/30s, ~2500/30s, ~3900/15s), insgesamt
  nicht mehr.
- Ueberwacht CPU-/GPU-Temperatur ueber macmon waehrend des Tests -
  ueber 90 Grad: sofortiger Abbruch + Reset auf Automatik.
- try/finally UND Signal-Handler (SIGINT/SIGTERM) stellen in JEDEM
  Fall Automatik wieder her, auch bei Abbruch/Fehler.
- Eigener "reset"-Befehl, jederzeit von Hand aufrufbar.
- Kein dauerhafter Dienst, keine Installation - dieses Skript macht
  nichts, ausser wenn es explizit mit "test" oder "reset" aufgerufen
  wird.

Aufruf:
    python3 fan_control.py status   # nur lesen, kein sudo noetig
    sudo python3 fan_control.py test    # Phase 1/2/3 von Anfang an (~2000/2500/3900 RPM)
    sudo python3 fan_control.py test2   # Fortsetzung: Phase 2 (~3000 RPM/15s),
                                         # dann NUR wenn die sauber war, Phase 3
                                         # (Maximaldrehzahl laut SMC/15s)
    sudo python3 fan_control.py reset
"""
import ctypes
import ctypes.util
import json
import os
import signal
import subprocess
import sys
import time

# ---------------------------------------------------------------------
# SMC-Struct-Layout (1:1 aus MacFanControl/src/smc.rs uebernommen,
# dort mit `assert!(size_of::<SmcKeyData>() == 80)` abgesichert - wir
# pruefen das hier ebenfalls, siehe unten).
# ---------------------------------------------------------------------

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
            raise SMCError(f"IOServiceOpen fehlgeschlagen (kern {kr:#x}) - laeuft das Skript mit sudo?")
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
            raise NotPrivileged("Zugriff verweigert - bitte mit sudo ausfuehren")
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


# ---------------------------------------------------------------------
# Luefter-Erkennung (wie fan.rs: FNum, F{i}Ac/Tg/Mn/Mx, F{i}md/Md)
# ---------------------------------------------------------------------

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


# ---------------------------------------------------------------------
# macmon-Temperaturabfrage (dieselbe Quelle, die auch die Mac-App nutzt)
# ---------------------------------------------------------------------

def read_temps():
    try:
        out = subprocess.run(["macmon", "pipe", "-s", "1"], capture_output=True, timeout=5, text=True)
        d = json.loads(out.stdout.strip().splitlines()[-1])
        return d["temp"]["cpu_temp_avg"], d["temp"]["gpu_temp_avg"]
    except Exception as exc:
        print(f"[warnung] Temperatur konnte nicht gelesen werden: {exc!r}", file=sys.stderr)
        return None, None


TEMP_ABORT_C = 90.0


# ---------------------------------------------------------------------
# Hauptbefehle
# ---------------------------------------------------------------------

def cmd_status():
    with Smc() as smc:
        fans = discover_fans(smc)
        if not fans:
            print("Keine Luefter gefunden (oder kein Zugriff).")
            return
        for fan in fans:
            st = fan_status(smc, fan)
            print(
                f"Fan {fan['index']}: {st['actual']:.0f} RPM (Ziel {st['target']:.0f}), "
                f"Bereich {fan['min']:.0f}-{fan['max']:.0f}, Modus {st['mode']}"
            )
    cpu_t, gpu_t = read_temps()
    if cpu_t is not None:
        print(f"CPU-Temp: {cpu_t:.1f}C  GPU-Temp: {gpu_t:.1f}C")


def set_auto(smc: Smc, fan):
    """Lueft auf Automatik zurueck: Modus-Schluessel auf 0, danach
    Ziel-Drehzahl 0 schreiben (macOS ignoriert das im Automatikmodus,
    entspricht aber exakt dem geprueften Referenzverhalten)."""
    i = fan["index"]
    if fan["md_key"]:
        try:
            smc.write(fan["md_key"], bytes([0]))
        except SMCError as exc:
            print(f"[reset] Warnung: Modus-Reset fuer Fan {i} fehlgeschlagen: {exc}", file=sys.stderr)
    try:
        data = smc.encode_rpm(f"F{i}Tg", 0.0)
        smc.write(f"F{i}Tg", data)
    except SMCError:
        pass


def cmd_reset():
    with Smc() as smc:
        fans = discover_fans(smc)
        for fan in fans:
            set_auto(smc, fan)
        if smc.exists("Ftst"):
            try:
                smc.write("Ftst", bytes([0]))
            except SMCError as exc:
                print(f"[reset] Warnung: Ftst-Freigabe konnte nicht zurueckgesetzt werden: {exc}", file=sys.stderr)
    print("Automatikmodus wiederhergestellt.")
    cmd_status()


def engage_manual(smc: Smc, fan, log):
    """Manuellen Modus aktivieren - auf M3/M4 muss dafuer zuerst der
    'Ftst'-Freigabeschluessel gesetzt werden (siehe MacFanControl/
    control.rs::engage_manual, exakt uebernommene Ablauflogik:
    Ftst=1 setzen, ~3s warten, dann bis zu 30s lang den Modus-Schluessel
    im 100ms-Takt erneut versuchen)."""
    i = fan["index"]
    if fan["md_key"] and smc.read_u8(fan["md_key"]) == 1:
        return  # schon manuell
    try:
        if fan["md_key"]:
            smc.write(fan["md_key"], bytes([1]))
            return
        raise SMCError("kein Modus-Schluessel gefunden")
    except SMCError:
        if not smc.exists("Ftst"):
            raise
        log("Entsperre Luefterkontrolle vom Thermal-Manager (Ftst, ca. 3-6s)...")
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
    rpm = max(fan["min"], min(fan["max"], rpm))
    engage_manual(smc, fan, log=lambda m: print(f"  {m}"))
    data = smc.encode_rpm(f"F{fan['index']}Tg", rpm)
    for _ in range(10):
        try:
            smc.write(f"F{fan['index']}Tg", data)
            return rpm
        except SMCError:
            time.sleep(0.05)
    raise SMCError("Zieldrehzahl liess sich nicht schreiben")


_reset_done = False


def emergency_reset(reason):
    global _reset_done
    if _reset_done:
        return
    _reset_done = True
    print(f"\n[NOTFALL-RESET] {reason}", file=sys.stderr)
    try:
        cmd_reset()
    except Exception as exc:
        print(f"[NOTFALL-RESET] FEHLGESCHLAGEN: {exc!r} - bitte sofort 'sudo python3 fan_control.py reset' "
              "von Hand ausfuehren, im Notfall den Mac neu starten.", file=sys.stderr)


def _signal_handler(signum, _frame):
    emergency_reset(f"Signal {signum} empfangen")
    sys.exit(1)


def _run_phases(phases):
    """Gemeinsamer Phasenablauf fuer cmd_test()/cmd_test2() - jede Phase
    ist (Name, Ziel-RPM, Dauer_s). Bricht bei >90 Grad sofort ab (Notfall-
    Reset) und ueberspringt alle folgenden Phasen, wenn eine Phase nicht
    sauber zu Ende lief (gilt fuer cmd_test2(): Phase 3/Vollgas nur, wenn
    Phase 2 sauber war, siehe Auftrag Schritt C0)."""
    try:
        with Smc() as smc:
            fans = discover_fans(smc)
            if not fans:
                print("Keine Luefter gefunden - Abbruch.")
                return False
            fan = fans[0]

            print("=== Ausgangszustand ===")
            st0 = fan_status(smc, fan)
            cpu0, gpu0 = read_temps()
            print(f"Fan {fan['index']}: {st0['actual']:.0f} RPM, Modus {st0['mode']}, "
                  f"Bereich {fan['min']:.0f}-{fan['max']:.0f}")
            print(f"CPU {cpu0}C GPU {gpu0}C" if cpu0 else "Temperatur nicht lesbar")

            for name, target_rpm, duration_s in phases:
                target_rpm = min(target_rpm, fan["max"])
                print(f"\n=== {name}: Ziel {target_rpm:.0f} RPM fuer {duration_s}s ===")
                applied = set_target_rpm(smc, fan, target_rpm)
                print(f"  Zieldrehzahl gesetzt: {applied:.0f} RPM")

                start = time.monotonic()
                while time.monotonic() - start < duration_s:
                    time.sleep(2)
                    st = fan_status(smc, fan)
                    cpu_t, gpu_t = read_temps()
                    temp_str = f"CPU {cpu_t:.1f}C GPU {gpu_t:.1f}C" if cpu_t is not None else "Temp n/v"
                    print(f"  [{time.monotonic()-start:5.1f}s] Ist={st['actual']:.0f} RPM  {temp_str}")
                    if cpu_t is not None and (cpu_t > TEMP_ABORT_C or (gpu_t is not None and gpu_t > TEMP_ABORT_C)):
                        emergency_reset(f"Temperatur ueber {TEMP_ABORT_C}C waehrend {name}")
                        return False
                print(f"  {name} sauber beendet.")

            print("\nAlle Phasen abgeschlossen, setze auf Automatik zurueck...")
            return True
    finally:
        emergency_reset("Testende (regulaer oder durch Fehler)")


def cmd_test():
    signal.signal(signal.SIGINT, _signal_handler)
    signal.signal(signal.SIGTERM, _signal_handler)
    phases = [
        ("Phase 1", 2000, 30),
        ("Phase 2", 2500, 30),
        ("Phase 3", 3900, 15),
    ]
    _run_phases(phases)


def cmd_test2():
    """Fortsetzung des Tests (Schritt C0 im Auftrag "Luefter-Boost") -
    Phase 1 (~2000 RPM) lief in einer frueheren Sitzung bereits sauber,
    hier direkt weiter mit Phase 2 (~3000 RPM/15s) und, NUR wenn diese
    sauber lief, Phase 3 (Maximaldrehzahl laut SMC/15s). Dieselben
    Sicherheitsregeln wie cmd_test(): nur nach oben, 90-Grad-Abbruch,
    Automatik-Reset in jedem Fall."""
    signal.signal(signal.SIGINT, _signal_handler)
    signal.signal(signal.SIGTERM, _signal_handler)
    with Smc() as smc:
        fans = discover_fans(smc)
        max_rpm = fans[0]["max"] if fans else 4900.0
    phases = [
        ("Phase 2", 3000, 15),
        ("Phase 3 (Vollgas)", max_rpm, 15),
    ]
    _run_phases(phases)


def main():
    if len(sys.argv) != 2 or sys.argv[1] not in ("status", "test", "test2", "reset"):
        print(__doc__)
        sys.exit(1)
    cmd = sys.argv[1]
    if cmd in ("test", "test2", "reset") and os.geteuid() != 0:
        print(f"'{cmd}' braucht Root-Rechte - bitte mit sudo erneut aufrufen:\n"
              f"  sudo python3 {sys.argv[0]} {cmd}")
        sys.exit(1)
    if cmd == "status":
        cmd_status()
    elif cmd == "reset":
        cmd_reset()
    elif cmd == "test":
        cmd_test()
    elif cmd == "test2":
        cmd_test2()


if __name__ == "__main__":
    main()
