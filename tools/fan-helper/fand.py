#!/usr/bin/env python3
"""Eiswolfs iBoard Fan-Helper - kleiner LaunchDaemon (Root), der NUR
die Luefter-SMC-Zugriffe fuer den "Luefter-Boost" macht. Laeuft separat
von der Mac-App (die braucht keinen Root), siehe CLAUDE.md Abschnitt
"Luefter-Boost".

Lauscht auf einer lokalen Unix-Socket-Datei (SOCK_PATH), die nach dem
ersten Bind auf den Besitzer des per --user angegebenen Benutzers
(chown) und Modus 0600 (nur dieser Benutzer) eingeschraenkt wird - nie
auf dem Netz erreichbar.

Protokoll: eine Zeile pro Verbindung, ASCII, Request -> eine
JSON-Antwortzeile, Verbindung wird danach geschlossen:
    PING                  -> {"ok": true, ...status...}
    STATUS                -> {"ok": true, ...status...}
    BOOST <rpm|MAX>       -> {"ok": true/false, ...status...}
    AUTO                  -> {"ok": true, ...status...}
Alles andere wird mit {"ok": false, "error": "..."} abgelehnt.

Sicherheit (siehe Auftrag):
- BOOST begrenzt sich selbst: nie unter den vom SMC gemeldeten
  Minimalwert (= "nie unter die Automatik"), nie ueber den Maximalwert.
- Wachhund-Thread: Boost aktiv + >10s kein PING/BOOST-Refresh von der
  Mac-App -> automatisch zurueck auf Automatik.
- Absolute Obergrenze von 4 Stunden pro Boost, auch im Modus "bis Stopp".
- Beim Start, bei SIGTERM und beim Beenden (atexit) wird in jedem Fall
  auf Automatik gestellt.
"""
import atexit
import json
import os
import pwd
import signal
import socket
import sys
import threading
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from smc_core import Smc, SMCError, discover_fans, fan_status, set_auto, set_target_rpm

SOCK_PATH = "/var/run/eiswolfs-iboard-fan.sock"
PING_TIMEOUT_S = 10.0
MAX_BOOST_SECONDS = 4 * 3600
ALLOWED_RPM_KEYWORDS = ("2000", "3000", "4000", "MAX")

_lock = threading.Lock()
_smc = None
_fan = None
_boost_active = False
_boost_started = 0.0
_last_keepalive = 0.0
_stop = False


def log(msg):
    print(f"[fan-helper] {msg}", flush=True)


def _ensure_smc():
    global _smc, _fan
    if _smc is None:
        _smc = Smc()
    if _fan is None:
        fans = discover_fans(_smc)
        if not fans:
            raise SMCError("kein Luefter gefunden")
        _fan = fans[0]
    return _smc, _fan


def _status_locked():
    try:
        smc, fan = _ensure_smc()
        st = fan_status(smc, fan)
        return {
            "rpm": int(st["actual"]),
            "target_rpm": int(st["target"]),
            "mode": st["mode"],
            "max_rpm": int(fan["max"]),
            "min_rpm": int(fan["min"]),
            "boost_active": _boost_active,
        }
    except SMCError as exc:
        return {"error": f"SMC-Fehler: {exc}"}


def cmd_status():
    with _lock:
        return {"ok": True, **_status_locked()}


def cmd_ping():
    global _last_keepalive
    with _lock:
        if _boost_active:
            _last_keepalive = time.monotonic()
        return {"ok": True, **_status_locked()}


def cmd_boost(arg):
    global _boost_active, _boost_started, _last_keepalive
    arg = (arg or "").strip().upper()
    if arg not in ALLOWED_RPM_KEYWORDS:
        return {"ok": False, "error": f"ungueltige Drehzahlstufe {arg!r}"}
    with _lock:
        try:
            smc, fan = _ensure_smc()
            target = fan["max"] if arg == "MAX" else float(arg)
            applied = set_target_rpm(smc, fan, target)
            _boost_active = True
            _boost_started = time.monotonic()
            _last_keepalive = _boost_started
            log(f"Boost gestartet: Ziel {applied:.0f} RPM (angefordert {arg})")
            return {"ok": True, "applied_rpm": int(applied), **_status_locked()}
        except SMCError as exc:
            return {"ok": False, "error": f"SMC-Fehler: {exc}"}


def cmd_auto():
    global _boost_active
    with _lock:
        try:
            smc, fan = _ensure_smc()
            set_auto(smc, fan)
            was_active = _boost_active
            _boost_active = False
            if was_active:
                log("Boost beendet, Automatik wiederhergestellt.")
            return {"ok": True, **_status_locked()}
        except SMCError as exc:
            return {"ok": False, "error": f"SMC-Fehler: {exc}"}


def handle_line(line):
    parts = line.strip().split(None, 1)
    if not parts:
        return {"ok": False, "error": "leerer Befehl"}
    cmd = parts[0].upper()
    arg = parts[1] if len(parts) > 1 else None
    if cmd == "PING":
        return cmd_ping()
    if cmd == "STATUS":
        return cmd_status()
    if cmd == "BOOST":
        return cmd_boost(arg)
    if cmd == "AUTO":
        return cmd_auto()
    return {"ok": False, "error": f"unbekannter Befehl {cmd!r}"}


def _watchdog_loop():
    global _boost_active
    while not _stop:
        time.sleep(1.0)
        with _lock:
            if not _boost_active:
                continue
            now = time.monotonic()
            since_ping = now - _last_keepalive
            since_start = now - _boost_started
            reason = None
            if since_ping > PING_TIMEOUT_S:
                reason = f"kein Ping seit {since_ping:.0f}s"
            elif since_start > MAX_BOOST_SECONDS:
                reason = f"Obergrenze von {MAX_BOOST_SECONDS // 3600}h erreicht"
            if reason:
                try:
                    smc, fan = _ensure_smc()
                    set_auto(smc, fan)
                except SMCError as exc:
                    log(f"Wachhund: Automatik-Reset fehlgeschlagen: {exc!r}")
                _boost_active = False
                log(f"Wachhund: Boost beendet ({reason}).")


def _restrict_socket(path, username):
    try:
        pw = pwd.getpwnam(username)
        os.chown(path, pw.pw_uid, pw.pw_gid)
    except (KeyError, OSError) as exc:
        log(f"Warnung: Socket-Besitzrechte fuer {username!r} konnten nicht gesetzt werden: {exc!r}")
    os.chmod(path, 0o600)


def _reset_on_exit():
    log("Beende mich, stelle sicherheitshalber auf Automatik...")
    try:
        with _lock:
            smc, fan = _ensure_smc()
            set_auto(smc, fan)
    except Exception as exc:
        log(f"Automatik-Reset beim Beenden fehlgeschlagen: {exc!r}")


def _signal_handler(signum, _frame):
    global _stop
    log(f"Signal {signum} empfangen, beende mich.")
    _stop = True


def serve(username):
    global _stop
    if os.path.exists(SOCK_PATH):
        try:
            os.remove(SOCK_PATH)
        except OSError:
            pass

    srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    srv.bind(SOCK_PATH)
    _restrict_socket(SOCK_PATH, username)
    srv.listen(8)
    srv.settimeout(1.0)

    # Sicherer Ausgangszustand - unabhaengig davon, was vor einem
    # (Neu-)Start des Dienstes im SMC stand.
    try:
        with _lock:
            smc, fan = _ensure_smc()
            set_auto(smc, fan)
        log("Gestartet, Luefter steht auf Automatik.")
    except SMCError as exc:
        log(f"Warnung: Automatik-Reset beim Start fehlgeschlagen: {exc!r}")

    watchdog = threading.Thread(target=_watchdog_loop, daemon=True)
    watchdog.start()

    while not _stop:
        try:
            conn, _addr = srv.accept()
        except socket.timeout:
            continue
        except OSError:
            break
        try:
            conn.settimeout(2.0)
            data = b""
            while b"\n" not in data and len(data) < 256:
                chunk = conn.recv(256)
                if not chunk:
                    break
                data += chunk
            line = data.decode("utf-8", errors="ignore")
            try:
                response = handle_line(line)
            except Exception as exc:
                response = {"ok": False, "error": f"interner Fehler: {exc!r}"}
            conn.sendall((json.dumps(response) + "\n").encode("utf-8"))
        except Exception as exc:
            log(f"Verbindungsfehler: {exc!r}")
        finally:
            conn.close()

    srv.close()
    try:
        os.remove(SOCK_PATH)
    except OSError:
        pass


def main():
    if os.geteuid() != 0:
        print("fand.py braucht Root-Rechte (LaunchDaemon) - bitte nicht direkt aufrufen, "
              "siehe install.sh.", file=sys.stderr)
        sys.exit(1)
    if len(sys.argv) != 3 or sys.argv[1] != "--user":
        print("Aufruf: fand.py --user <benutzername>", file=sys.stderr)
        sys.exit(1)
    username = sys.argv[2]

    signal.signal(signal.SIGTERM, _signal_handler)
    signal.signal(signal.SIGINT, _signal_handler)
    atexit.register(_reset_on_exit)

    serve(username)


if __name__ == "__main__":
    main()
