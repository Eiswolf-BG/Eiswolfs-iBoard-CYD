"""Luefter-Boost-Client (Mac-App-Seite) fuer den Auftrag "Luefter-
Boost". Spricht NICHT selbst mit dem SMC - das macht ausschliesslich
der Root-Hilfsdienst (tools/fan-helper/fand.py) ueber seine
Unix-Socket-Datei. Fehlt der Dienst oder die Socket-Datei, ist der
Boost schlicht "nicht verfuegbar" - jede Funktion hier gibt dann einen
klaren Fehler zurueck, nichts stuerzt ab oder haengt.

Zwei unabhaengige Wege koennen einen Boost anstossen:
- Das Mac-Menue (IBoardApp in sender.py) - Dauer/Drehzahl kommen dann
  aus der config.json ("fan_boost_sec"/"fan_boost_rpm").
- Das CYD per UDP (Port 4211, NICHT der Datenport) oder per USB-Zeile
  (siehe CydCommandListener) - Dauer/Drehzahl stehen direkt im Befehl,
  beide Wege sind bewusst unabhaengig von der Mac-Menue-Einstellung.
"""
import json
import re
import socket
import threading
import time

import config
import i18n

SOCK_PATH = "/var/run/eiswolfs-iboard-fan.sock"
PING_INTERVAL_S = 2.0
MAX_BOOST_SECONDS = 4 * 3600  # muss zur Obergrenze in fand.py passen
CYD_CMD_PORT = 4211
CYD_CMD_MIN_INTERVAL_S = 1.0
CYD_LINK_TIMEOUT_S = 60.0  # siehe FanBoostClient._run()

RPM_LEVELS = ("2000", "3000", "4000", "MAX")
DURATIONS = (30, 60, 120, 300, 600, 0)  # 0 = "Bis Stopp"

_BOOST_RE = re.compile(r"^FAN BOOST (\d+) (2000|3000|4000|MAX) (\S+)$")
_AUTO_RE = re.compile(r"^FAN AUTO (\S+)$")
_SLEEP_RE = re.compile(r"^SLEEP (\S+)$")
CYD_SLEEP_CMD_MIN_INTERVAL_S = 1.0


def _log(msg):
    print(f"[Eiswolfs iBoard-Fan] {msg}", flush=True)


def duration_label(seconds):
    if seconds <= 0:
        return i18n.t("duration_until_stop")
    if seconds < 60:
        return i18n.t("duration_seconds", s=seconds)
    minutes = seconds // 60
    return i18n.t("duration_minute", m=minutes) if minutes == 1 else i18n.t("duration_minutes", m=minutes)


def rpm_label(level):
    return i18n.t("rpm_max") if level == "MAX" else i18n.t("rpm_level", level=level)


def _send_command(line, timeout=1.5):
    """Oeffnet eine neue Verbindung zum Hilfsdienst, schickt EINE
    Befehlszeile, liest EINE Antwortzeile, schliesst wieder. Gibt
    (ok, dict) zurueck - ok ist False bei jedem Verbindungsproblem
    (Dienst nicht installiert, Socket weg, Zeitueberschreitung), wirft
    nie eine Exception nach aussen."""
    try:
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect(SOCK_PATH)
        sock.sendall((line + "\n").encode("utf-8"))
        data = b""
        while b"\n" not in data and len(data) < 4096:
            chunk = sock.recv(4096)
            if not chunk:
                break
            data += chunk
        sock.close()
        resp = json.loads(data.decode("utf-8", errors="ignore").strip())
        return bool(resp.get("ok")), resp
    except Exception:
        return False, {}


def is_available():
    ok, _ = _send_command("PING")
    return ok


def reset_to_auto_on_startup():
    """Sicherer Anfangszustand beim App-Start (Auftrag): ein evtl. noch
    laufender Boost (z.B. vom CYD gestartet, bevor die App neu
    gestartet wurde) wird sofort beendet. Kein Fehler, falls der
    Hilfsdienst gar nicht installiert ist."""
    _send_command("AUTO")


class FanBoostClient:
    """Verwaltet einen laufenden Boost von der Mac-App-Seite aus:
    sendet BOOST einmalig, danach alle PING_INTERVAL_S Sekunden einen
    Ping, verwaltet die Dauer SELBST (der Hilfsdienst kennt nur seinen
    eigenen 10s-Ping-Wachhund und die 4h-Obergrenze) und beendet mit
    AUTO, sobald eine Stop-Bedingung zutrifft."""

    def __init__(self, on_change=None, cyd_idle_seconds=lambda: 0.0):
        self._lock = threading.Lock()
        self._active = False
        self._stop_flag = False
        self._started_monotonic = 0.0
        self._duration_s = 0
        self._rpm_level = "MAX"
        self._started_by_cyd = False
        self._on_change = on_change
        self._cyd_idle_seconds = cyd_idle_seconds

    def is_active(self):
        with self._lock:
            return self._active

    def started_by_cyd(self):
        with self._lock:
            return self._started_by_cyd

    def remaining_seconds(self):
        """-1 = unbegrenzt ("Bis Stopp"), sonst verbleibende Sekunden
        (nie negativ), 0 wenn kein Boost aktiv."""
        with self._lock:
            if not self._active:
                return 0
            if self._duration_s <= 0:
                return -1
            left = self._duration_s - (time.monotonic() - self._started_monotonic)
            return max(0, int(left))

    def start(self, rpm_level, duration_s, started_by_cyd=False):
        """True/False + Fehlertext. duration_s <= 0 bedeutet "Bis Stopp"."""
        with self._lock:
            if self._active:
                return False, "Boost laeuft bereits"
        if rpm_level not in RPM_LEVELS:
            return False, "ungueltige Drehzahlstufe"
        ok, resp = _send_command(f"BOOST {rpm_level}")
        if not ok:
            return False, resp.get("error", "Hilfsdienst nicht verfuegbar")
        with self._lock:
            self._active = True
            self._duration_s = max(0, duration_s)
            self._rpm_level = rpm_level
            self._started_monotonic = time.monotonic()
            self._started_by_cyd = started_by_cyd
            self._stop_flag = False
        threading.Thread(target=self._run, daemon=True).start()
        self._notify()
        return True, None

    def stop(self):
        """Fordert ein sofortiges Boost-Ende an (naechster Schleifen-
        durchlauf in _run() beendet ihn, i.d.R. binnen 0.2s)."""
        with self._lock:
            self._stop_flag = True

    def _end(self):
        _send_command("AUTO")
        with self._lock:
            self._active = False
            self._started_by_cyd = False
        self._notify()

    def _notify(self):
        if self._on_change:
            try:
                self._on_change()
            except Exception:
                pass

    def _run(self):
        last_ping = 0.0
        while True:
            with self._lock:
                if self._stop_flag or not self._active:
                    break
                elapsed = time.monotonic() - self._started_monotonic
                duration = self._duration_s
                cyd_started = self._started_by_cyd
            if duration > 0 and elapsed >= duration:
                break
            if elapsed >= MAX_BOOST_SECONDS:
                _log("Boost erreicht die 4h-Obergrenze, beende ihn.")
                break
            # CYD-Verbindungs-Timeout gilt NUR fuer vom CYD gestartete
            # Boosts (Auftrag) - ein am Mac-Menue gestarteter Boost
            # laeuft unabhaengig davon weiter.
            if cyd_started and self._cyd_idle_seconds() > CYD_LINK_TIMEOUT_S:
                _log("Vom CYD gestarteter Boost: Verbindung >60s weg, beende ihn.")
                break
            now = time.monotonic()
            if now - last_ping >= PING_INTERVAL_S:
                ok, _ = _send_command("PING")
                last_ping = now
                if not ok:
                    _log("Hilfsdienst antwortet nicht mehr, beende Boost.")
                    break
            time.sleep(0.2)
        self._end()


class CydCommandListener:
    """Nimmt Befehle vom CYD entgegen - per UDP (eigener Port 4211,
    NICHT der Datenport) im Hintergrund-Thread, und per USB-Zeile
    (ueber handle_line(), von SenderThread aus usb_link.read_lines()
    weitergereicht). Jeder Befehl braucht den gueltigen, gemeinsamen
    Token (siehe secrets.h/config.json) - ungueltige/fehlende Token
    sowie mehr als ein Befehl pro Sekunde werden kommentarlos ignoriert
    (kein Fehler-Echo ans CYD). Urspruenglich nur fuer Luefter-Boost-
    Befehle gedacht (Klassenname/Dateiname historisch), seit Auftrag
    "Ruhemodus" auch fuer den SLEEP-Befehl zustaendig - EIN
    gemeinsamer Listener auf demselben Port, statt eines zweiten
    (der Port kann nur einmal gebunden werden)."""

    def __init__(self, on_boost, on_auto, on_sleep=None):
        self.on_boost = on_boost
        self.on_auto = on_auto
        self.on_sleep = on_sleep
        self._last_cmd_monotonic = 0.0
        self._last_sleep_cmd_monotonic = 0.0
        self._sock = None
        self._stop = False
        self._thread = threading.Thread(target=self._udp_loop, daemon=True)
        self._thread.start()

    def stop(self):
        self._stop = True
        if self._sock is not None:
            try:
                self._sock.close()
            except Exception:
                pass

    def _udp_loop(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.bind(("", CYD_CMD_PORT))
            sock.settimeout(1.0)
            self._sock = sock
        except Exception as exc:
            _log(f"FAN-Befehlsport {CYD_CMD_PORT} konnte nicht geoeffnet werden: {exc!r}")
            return
        while not self._stop:
            try:
                data, _addr = sock.recvfrom(256)
            except socket.timeout:
                continue
            except OSError:
                break
            try:
                self.handle_line(data.decode("utf-8", errors="ignore").strip())
            except Exception as exc:
                _log(f"Fehler beim Verarbeiten eines FAN-Befehls: {exc!r}")

    def handle_line(self, line):
        """Von aussen auch direkt fuer USB-Zeilen aufgerufen (siehe
        sender.py::SenderThread.run())."""
        if line.startswith("SLEEP "):
            self._handle_sleep(line)
            return
        if not line.startswith("FAN "):
            return
        now = time.monotonic()
        if now - self._last_cmd_monotonic < CYD_CMD_MIN_INTERVAL_S:
            return
        token = config.get_fan_boost_token()
        if not token:
            return

        m = _BOOST_RE.match(line)
        if m:
            seconds_str, rpm_level, got_token = m.groups()
            if got_token != token:
                return
            seconds = int(seconds_str)
            if seconds != 0 and not (10 <= seconds <= 3600):
                return
            self._last_cmd_monotonic = now
            self.on_boost(seconds, rpm_level)
            return

        m = _AUTO_RE.match(line)
        if m:
            got_token = m.group(1)
            if got_token != token:
                return
            self._last_cmd_monotonic = now
            self.on_auto()

    def _handle_sleep(self, line):
        """Eigene, von FAN-Befehlen unabhaengige Ratenbegrenzung (Auftrag:
        hoechstens ein SLEEP-Befehl pro Sekunde) - nutzt dasselbe Token
        wie der Luefter-Boost (config.get_fan_boost_token()), keine
        zweite Geheimnis-Datei."""
        if self.on_sleep is None:
            return
        now = time.monotonic()
        if now - self._last_sleep_cmd_monotonic < CYD_SLEEP_CMD_MIN_INTERVAL_S:
            return
        token = config.get_fan_boost_token()
        if not token:
            return
        m = _SLEEP_RE.match(line)
        if not m:
            return
        if m.group(1) != token:
            return
        self._last_sleep_cmd_monotonic = now
        self.on_sleep()
