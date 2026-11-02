"""USB-Verbindung zum Eiswolfs-iBoard-CYD als Zusatz-Weg neben WLAN-Broadcast.

Sucht alle paar Sekunden passende serielle Ports (usbserial/
wchusbserial/usbmodem), sendet jedem kurz "IBOARD?\n" und haelt nur den
Port offen, der mit "IBOARD_OK" antwortet - andere Geraete (z.B. ein
anderer ESP32 mit anderer Firmware) bekommen dadurch nie Daten. Das
Scannen laeuft in einem eigenen Hintergrund-Thread und ruehrt den Port
nur an, solange keiner bestaetigt ist; das eigentliche Schreiben der
Datenzeile passiert direkt im Aufrufer (SenderThread), damit es im
festen 1-Sekunden-Takt bleibt statt in einem eigenen Thread zu laufen."""
import threading
import time

import serial
import serial.tools.list_ports

CANDIDATE_NAME_PARTS = ("usbserial", "wchusbserial", "usbmodem")
HANDSHAKE_REQUEST = b"IBOARD?\n"
HANDSHAKE_REPLY = b"IBOARD_OK"
SCAN_INTERVAL_SECONDS = 5.0


def _log(msg):
    print(f"[Eiswolfs iBoard-USB] {msg}", flush=True)


class UsbLink:
    def __init__(self):
        self.lock = threading.Lock()
        self.ser = None
        self.released = False
        self.released_until = 0.0
        self._stop = False
        self._read_buf = b""
        self._thread = threading.Thread(target=self._scan_loop, daemon=True)
        self._thread.start()

    def stop(self):
        self._stop = True
        with self.lock:
            if self.ser is not None:
                try:
                    self.ser.close()
                except Exception:
                    pass
                self.ser = None

    def is_connected(self):
        with self.lock:
            return self.ser is not None

    def is_released(self):
        with self.lock:
            return self.released

    def release(self, seconds=60):
        """Menuepunkt "USB freigeben" - Port schliessen und Suche fuer
        `seconds` Sekunden (oder bis resume()) pausieren, damit z.B.
        PlatformIO den Port zum Flashen benutzen kann."""
        with self.lock:
            if self.ser is not None:
                try:
                    self.ser.close()
                except Exception:
                    pass
                self.ser = None
            self.released = True
            self.released_until = time.monotonic() + seconds

    def resume(self):
        with self.lock:
            self.released = False
            self.released_until = 0.0

    def write_line(self, line):
        """True bei Erfolg. Schlaegt der Schreibvorgang fehl (Kabel
        gezogen etc.), wird der Port geschlossen und die Suche setzt
        beim naechsten Scan-Durchlauf automatisch wieder ein."""
        with self.lock:
            ser = self.ser
        if ser is None:
            return False
        try:
            ser.write((line + "\n").encode("utf-8"))
            return True
        except Exception as exc:
            _log(f"Schreibfehler, Verbindung wird getrennt: {exc!r}")
            with self.lock:
                if self.ser is ser:
                    self.ser = None
            try:
                ser.close()
            except Exception:
                pass
            return False

    def read_lines(self):
        """Liest nicht-blockierend alle gerade verfuegbaren, vollstaendigen
        Zeilen vom CYD (z.B. "FAN BOOST ..."/"FAN AUTO ...", siehe
        fan_boost.py) - gibt eine (ggf. leere) Liste zurueck, wirft nie."""
        with self.lock:
            ser = self.ser
        if ser is None:
            return []
        try:
            waiting = ser.in_waiting
            if waiting:
                self._read_buf += ser.read(waiting)
        except Exception:
            return []
        lines = []
        while b"\n" in self._read_buf:
            line, self._read_buf = self._read_buf.split(b"\n", 1)
            text = line.decode("utf-8", errors="ignore").strip()
            if text:
                lines.append(text)
        return lines

    def _scan_loop(self):
        while not self._stop:
            with self.lock:
                if self.released and time.monotonic() >= self.released_until:
                    self.released = False
                connected = self.ser is not None
                paused = self.released

            if connected or paused:
                time.sleep(1.0)
                continue

            self._try_connect_once()
            time.sleep(SCAN_INTERVAL_SECONDS)

    def _candidate_ports(self):
        try:
            ports = serial.tools.list_ports.comports()
        except Exception:
            return []
        names = []
        for p in ports:
            dev = (p.device or "").lower()
            if any(part in dev for part in CANDIDATE_NAME_PARTS):
                names.append(p.device)
        return names

    def _try_connect_once(self):
        for portname in self._candidate_ports():
            s = None
            try:
                s = serial.Serial()
                s.port = portname
                s.baudrate = 115200
                s.timeout = 1.0
                # Port darf das CYD beim Oeffnen NICHT neu starten -
                # DTR/RTS deshalb vor dem Oeffnen explizit aus.
                s.dtr = False
                s.rts = False
                s.open()
                time.sleep(0.2)
                s.reset_input_buffer()
                s.write(HANDSHAKE_REQUEST)

                deadline = time.monotonic() + 1.0
                buf = b""
                ok = False
                while time.monotonic() < deadline:
                    chunk = s.read(64)
                    if chunk:
                        buf += chunk
                        if HANDSHAKE_REPLY in buf:
                            ok = True
                            break
                    else:
                        break

                if ok:
                    _log(f"Eiswolfs iBoard gefunden auf {portname}")
                    with self.lock:
                        self.ser = s
                    return
                s.close()
            except Exception:
                if s is not None:
                    try:
                        s.close()
                    except Exception:
                        pass
