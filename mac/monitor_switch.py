"""Monitor-Eingangsumschaltung per DDC/CI (Auftrag "Monitor-Umschaltung",
siehe CLAUDE.md fuer den vorangegangenen Machbarkeitstest). Nutzt das
quelloffene m1ddc (MIT-Lizenz, github.com/waydabber/m1ddc, siehe
THIRD-PARTY-LICENSES.md/licenses/), als arm64-Binary im App-Bundle
eingebettet (bundle_resources/m1ddc, siehe eiswolfs_iboard.spec) -
KEIN Homebrew/Systemordner-Zwang fuer Alex.

Sicherheitsgrenzen (Auftrag, bewusst strikt eingehalten):
- Es wird NUR VCP-Code 0x60 (Eingangswahl) geschrieben, nie Helligkeit/
  Lautstaerke/Kontrast o.ae.
- Es werden NUR die vier konfigurierten Rohwerte (logische Eingaenge
  1-4) verwendet, nie ein beliebiger/erratener Wert.
- Der Befehl geht AUSSCHLIESSLICH an den in den Einstellungen explizit
  ausgewaehlten Bildschirm (per UUID, siehe config.py) - ist keiner
  ausgewaehlt oder in der aktuellen m1ddc-Bildschirmliste nicht mehr
  auffindbar, wird NICHTS getan (kein Fallback auf "alle Bildschirme"
  oder "den ersten gefundenen"). Alex' zweiter Mac-Monitor kann dadurch
  strukturell nie betroffen sein.
- Hoechstens ein Befehl alle MONITOR_CMD_MIN_INTERVAL_S Sekunden (auch
  wenn das CYD/Alexa schneller/doppelt anfragt).
"""
import os
import subprocess
import sys
import threading
import time

import config

MONITOR_CMD_MIN_INTERVAL_S = 3.0
DDC_TIMEOUT_S = 3.0


def _log(msg):
    print(f"[Eiswolfs iBoard-Monitor] {msg}", flush=True)


def _is_frozen():
    return getattr(sys, "frozen", False)


def _m1ddc_path():
    """Gleiches Muster wie sender.py::resource_path() (hier dupliziert,
    um keinen Kreis-Import mit sender.py zu erzeugen - dieses Modul
    soll unabhaengig von sender.py testbar/importierbar bleiben)."""
    if _is_frozen():
        contents_dir = os.path.dirname(os.path.dirname(sys.executable))  # .../Contents
        return os.path.join(contents_dir, "Resources", "m1ddc")
    return os.path.join(os.path.dirname(os.path.abspath(__file__)), "bundle_resources", "m1ddc")


def is_available():
    return os.path.exists(_m1ddc_path())


def list_screens(timeout=DDC_TIMEOUT_S):
    """Gibt eine Liste von (uuid, name) zurueck - parst die Ausgabe von
    `m1ddc display list` (Format "[n] Name (UUID)" pro Zeile). Leere
    Liste bei jedem Fehler (Tool fehlt, Zeitueberschreitung, keine
    Bildschirme) - wirft nie eine Exception nach aussen."""
    path = _m1ddc_path()
    if not os.path.exists(path):
        return []
    try:
        result = subprocess.run(
            [path, "display", "list"],
            capture_output=True, timeout=timeout, text=True,
        )
    except Exception as exc:
        _log(f"Bildschirmliste konnte nicht abgerufen werden: {exc!r}")
        return []

    screens = []
    for line in result.stdout.splitlines():
        line = line.strip()
        if not line.startswith("["):
            continue
        # "[1] AW2720HFA (F68270EC-6F76-46B8-9CFE-EA38FF89BE40)"
        close = line.find("]")
        if close == -1:
            continue
        rest = line[close + 1:].strip()
        if not rest.endswith(")") or "(" not in rest:
            continue
        open_paren = rest.rfind("(")
        name = rest[:open_paren].strip()
        uuid = rest[open_paren + 1:-1].strip()
        if name and uuid:
            screens.append((uuid, name))
    return screens


class MonitorSwitcher:
    """Nimmt Umschaltanfragen entgegen (vom CYD-Befehl ODER vom
    Test-Knopf im Menue) - jede Anfrage startet einen eigenen,
    kurzlebigen Hintergrund-Thread (Timeout 3s + 1 Wiederholung,
    blockiert nie den aufrufenden Code (CydCommandListener/Mac-Menue)."""

    def __init__(self):
        self._last_cmd_monotonic = 0.0

    def request(self, logical_input):
        """Fuer den normalen Betrieb (CYD-Befehl) - schaltet einmalig
        auf den angegebenen logischen Eingang, KEIN automatisches
        Zurueckschalten (das macht beim Ausschalten der Windows-
        Listener, siehe CLAUDE.md)."""
        now = time.monotonic()
        if now - self._last_cmd_monotonic < MONITOR_CMD_MIN_INTERVAL_S:
            _log(f"Befehl fuer Eingang {logical_input} ignoriert (Ratenbegrenzung, <3s seit letztem Befehl)")
            return
        self._last_cmd_monotonic = now
        threading.Thread(target=self._run_single, args=(logical_input,), daemon=True).start()

    def _run_single(self, logical_input):
        if not config.get_monitor_switch_enabled():
            _log("Monitor-Umschaltung ist AUS - Befehl wird ignoriert")
            return
        uuid, value = self._resolve(logical_input)
        if uuid is None:
            return
        ok = self._write_vcp60(uuid, value)
        if not ok:
            _log(f"Erster Versuch fehlgeschlagen, ein Wiederholungsversuch fuer Eingang {logical_input} (Rohwert {value})")
            ok = self._write_vcp60(uuid, value)
        _log(f"Eingang {logical_input} (Rohwert {value}): {'erfolgreich gesendet' if ok else 'FEHLGESCHLAGEN'}")
        self._append_event_log(logical_input, value, ok)

    def _resolve(self, logical_input):
        uuid = config.get_monitor_switch_screen_uuid()
        if not uuid:
            _log("Kein Bildschirm ausgewaehlt - nichts getan")
            return None, None
        value = config.get_monitor_vcp_value(logical_input)
        if value is None:
            _log(f"Ungueltiger logischer Eingang {logical_input!r} - nichts getan")
            return None, None
        if not is_available():
            _log("m1ddc nicht im App-Bundle gefunden - nichts getan")
            return None, None
        return uuid, value

    def _write_vcp60(self, uuid, value):
        try:
            result = subprocess.run(
                [_m1ddc_path(), "display", uuid, "set", "input", str(value)],
                capture_output=True, timeout=DDC_TIMEOUT_S, text=True,
            )
            return result.returncode == 0
        except Exception as exc:
            _log(f"DDC-Schreibfehler: {exc!r}")
            return False

    def _append_event_log(self, logical_input, value, ok):
        try:
            import sender  # spaeter Import, um Kreis-Import beim Modul-Laden zu vermeiden
            sender.log_event(
                f"monitor_switch logical={logical_input} raw={value} ok={1 if ok else 0}"
            )
        except Exception:
            pass

    def test_switch_to_hdmi2(self, revert_after_s=15.0):
        """Fuer den Test-Knopf im Menue: liest den AKTUELLEN Eingang
        EINMAL vorher (zuverlaessig, siehe Machbarkeitstest - nur
        Lesewerte DIREKT NACH einem Schreibvorgang waren dort
        unzuverlaessig), schaltet auf HDMI2 (logischer Wert 2) und nach
        revert_after_s Sekunden automatisch zurueck auf den vorher
        gelesenen Rohwert. Gibt (ok, fehlertext) zurueck - fehlertext
        ist None bei Erfolg."""
        if not config.get_monitor_switch_enabled():
            return False, "monitor_test_no_screen"
        uuid = config.get_monitor_switch_screen_uuid()
        if not uuid:
            return False, "monitor_test_no_screen"
        if not is_available():
            return False, "monitor_test_no_m1ddc"

        target_value = config.get_monitor_vcp_value(2)
        threading.Thread(
            target=self._run_test, args=(uuid, target_value, revert_after_s), daemon=True
        ).start()
        return True, None

    def _run_test(self, uuid, target_value, revert_after_s):
        previous_value = self._read_vcp60(uuid)
        self._write_vcp60(uuid, target_value)
        _log(f"Test: auf Rohwert {target_value} geschaltet (vorheriger Wert: {previous_value})")
        time.sleep(revert_after_s)
        if previous_value is not None:
            self._write_vcp60(uuid, previous_value)
            _log(f"Test: zurueckgeschaltet auf Rohwert {previous_value}")
        else:
            _log("Test: vorheriger Wert konnte nicht gelesen werden - KEIN automatisches Zurueckschalten, bitte am Monitor-Joystick pruefen")

    def _read_vcp60(self, uuid):
        try:
            result = subprocess.run(
                [_m1ddc_path(), "display", uuid, "get", "input"],
                capture_output=True, timeout=DDC_TIMEOUT_S, text=True,
            )
            if result.returncode != 0:
                return None
            return int(result.stdout.strip())
        except Exception:
            return None
