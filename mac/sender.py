#!/usr/bin/env python3
"""Eiswolfs iBoard Sender - Menueleisten-App, die Mac-Systemwerte per UDP
an ein CYD-Anzeigegeraet schickt. Siehe ../CLAUDE.md fuer die Architektur.

Der eigentliche Versand laeuft in SenderThread mit einer festen, absolut
getakteten Uhr (naechster Zeitpunkt = letzter Zeitpunkt + Intervall),
komplett unabhaengig vom rumps/Cocoa-Runloop und von macmon - "Messung +
Pause" wuerde bei einer einzelnen langsamen Messung das Intervall nach
hinten verschieben, die feste Uhr nicht. macmon selbst laeuft in einem
eigenen Hintergrund-Thread (MacmonReader) und liefert nur einen
zwischengespeicherten Schnappschuss - haengt macmon kurz, sendet der
SenderThread einfach die letzten bekannten Werte weiter, ohne zu warten."""
import fcntl
import json
import logging
import logging.handlers
import os
import socket
import subprocess
import sys
import threading
import time

import psutil
import rumps

import config
import fan_boost
import i18n
import login_item
import mac_sleep
import monitor_switch
import token_share
import wake_watch
from usb_link import UsbLink


APP_VERSION = "1.2.0"

# Application-Support-Ordner: fuer Logs/Lock-Datei, GEMEINSAM genutzt
# von der Entwickler-Fassung (dieses venv-Projekt) und der gebauten
# .app, damit niemals beide gleichzeitig unbemerkt senden koennten.
APP_SUPPORT_DIR = config.APP_SUPPORT_DIR

# Letzter Kontakt zum CYD (USB verbunden ODER eine WLAN-Antwort/USB-Zeile
# tatsaechlich empfangen) - unabhaengig vom eigenen Sendetakt. Dient dem
# FanBoostClient, um einen vom CYD gestarteten Boost nach 60s ohne
# Gegenstelle automatisch zu beenden (siehe fan_boost.py).
_cyd_contact_lock = threading.Lock()
_cyd_last_contact_monotonic = 0.0


def mark_cyd_contact():
    global _cyd_last_contact_monotonic
    with _cyd_contact_lock:
        _cyd_last_contact_monotonic = time.monotonic()


def cyd_idle_seconds():
    with _cyd_contact_lock:
        last = _cyd_last_contact_monotonic
    if last == 0.0:
        return float("inf")
    return time.monotonic() - last


def is_frozen():
    """True, wenn als gebuendelte .app laeuft (PyInstaller), False im
    venv/Entwickler-Betrieb."""
    return getattr(sys, "frozen", False)


def resource_path(name):
    """Pfad zu einer mitgelieferten Ressource (z.B. macmon) - im
    gebuendelten .app-Bundle legt PyInstaller binaries tatsaechlich in
    Contents/Frameworks/ ab, erreichbar ueber den Symlink
    Contents/Resources/<name> (gleiche Ebene wie Contents/MacOS/, wo
    sys.executable liegt) - NICHT direkt neben der ausfuehrbaren Datei.
    Im Entwickler-Betrieb liegt sie in mac/bundle_resources/."""
    if is_frozen():
        contents_dir = os.path.dirname(os.path.dirname(sys.executable))  # .../Contents
        return os.path.join(contents_dir, "Resources", name)
    return os.path.join(os.path.dirname(os.path.abspath(__file__)), "bundle_resources", name)


def _log(msg):
    print(f"[Eiswolfs iBoard] {msg}", flush=True)


# Dauerhafte Ereignis-Log-Datei (Auftrag "Ruhezustand-Fix", Schritt 3) -
# macmon-Neustarts, Schlaf-/Aufwach-Uebergaenge usw., damit sich das
# Verhalten rund um Ruhezustand/DarkWake auch nachtraeglich nachvollziehen
# laesst, OHNE je eine Meldung auf dem Bildschirm zu zeigen. Groesse auf
# 1 MB begrenzt, eine rotierte Altdatei (RotatingFileHandler-Standard).
_event_logger = None


def _get_event_logger():
    global _event_logger
    if _event_logger is not None:
        return _event_logger
    logger = logging.getLogger("eiswolfs_iboard_events")
    logger.setLevel(logging.INFO)
    try:
        log_dir = os.path.join(APP_SUPPORT_DIR, "logs")
        os.makedirs(log_dir, exist_ok=True)
        handler = logging.handlers.RotatingFileHandler(
            os.path.join(log_dir, "events.log"), maxBytes=1_000_000, backupCount=1,
        )
        handler.setFormatter(logging.Formatter("%(asctime)s %(message)s"))
        logger.addHandler(handler)
    except OSError:
        pass
    _event_logger = logger
    return logger


def log_event(msg):
    try:
        _get_event_logger().info(msg)
    except Exception:
        pass
    _log(msg)


wake_watch.set_logger(log_event)


# Waehlbare Werte fuer die Menueleisten-Anzeige (Auftrag "waehlbarer
# Wert in der Menueleiste") - Reihenfolge hier bestimmt die Reihenfolge
# im Untermenue. Schluessel muessen zu den Feldern im Sende-Payload
# passen (siehe SenderThread._collect()) und zu config.MENUBAR_ITEMS.
MENUBAR_CHOICES = [
    ("cpu_pct", "label_cpu"),
    ("ram_pct", "label_ram"),
    ("disk_pct", "label_disk"),
    ("cpu_temp_c", "label_cpu_temp"),
    ("gpu_temp_c", "label_gpu_temp"),
    ("gpu_pct", "label_gpu"),
    ("power_w", "label_power"),
    ("fan_rpm", "label_fan"),
]


def format_menubar_title(item_key, payload):
    """Kurze Anzeige fuer das Menueleisten-Icon - liefert immer "--"
    statt einer Zahl, wenn der gewaehlte Wert (noch) nicht verfuegbar
    ist, nie einen Absturz (z.B. bei fehlendem macmon/Sensor). Die
    Temperatur-Werte laufen durch i18n.format_temp() (Auftrag
    "Celsius/Fahrenheit") - respektiert die eingestellte Einheit."""
    v = payload.get(item_key) if payload else None
    if item_key == "cpu_pct":
        return f"CPU {v:.0f}%" if v is not None else "CPU --%"
    if item_key == "ram_pct":
        return f"RAM {v:.0f}%" if v is not None else "RAM --%"
    if item_key == "disk_pct":
        return f"Disk {v:.0f}%" if v is not None else "Disk --%"
    if item_key == "cpu_temp_c":
        return f"CPU {i18n.format_temp(v)}" if v is not None else "CPU --°"
    if item_key == "gpu_temp_c":
        return f"GPU {i18n.format_temp(v)}" if v is not None else "GPU --°"
    if item_key == "gpu_pct":
        return f"GPU {v:.0f}%" if v is not None else "GPU --%"
    if item_key == "power_w":
        return f"{v:.1f} W" if v is not None else "-- W"
    if item_key == "fan_rpm":
        return f"Fan {v:.0f}" if v is not None else "Fan --"
    return "--"


# Auftrag "Ruhezustand-Fix", Schritt 3: wie lange ohne frischen macmon-
# Wert gilt als "haengt" (-> Neustart), und wie oft der Watchdog das
# hoechstens von sich aus tun darf (die explizite restart()-Methode beim
# Aufwachen ist davon ausgenommen, siehe unten).
MACMON_STALE_SEC = 5.0
MACMON_RESTART_MIN_GAP_SEC = 10.0


class MacmonReader:
    """Liest macmon im Hintergrund aus (ein dauerhafter Subprozess statt
    einem Aufruf pro Sekunde) und haelt die letzten Werte bereit. Startet
    den Prozess bei Bedarf neu, falls er abstuerzt ODER haengt (siehe
    _watchdog()) - ein Fehler hier darf die App nie zum Absturz bringen.
    self.latest bleibt beim Neustart erhalten (wird nie zurueckgesetzt),
    damit kurze Aussetzer nicht zu einem ploetzlichen Verlust aller
    Temperatur-/GPU-/Power-/Fan-Werte fuehren."""

    def __init__(self):
        self.lock = threading.Lock()
        self.latest = {}
        self._stop = False
        self._proc = None
        self._last_update_monotonic = time.monotonic()
        self._last_restart_monotonic = 0.0
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()
        self._watchdog_thread = threading.Thread(target=self._watchdog, daemon=True)
        self._watchdog_thread.start()

    def _run(self):
        macmon_bin = resource_path("macmon")
        while not self._stop:
            try:
                proc = subprocess.Popen(
                    [macmon_bin, "pipe", "-i", "1000"],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL,
                    text=True,
                )
                with self.lock:
                    self._proc = proc
                for line in proc.stdout:
                    if self._stop:
                        break
                    try:
                        data = json.loads(line)
                        with self.lock:
                            self.latest = data
                            self._last_update_monotonic = time.monotonic()
                    except (json.JSONDecodeError, ValueError):
                        continue
                with self.lock:
                    self._proc = None
                try:
                    proc.kill()
                except Exception:
                    pass
                proc.wait(timeout=1)
            except FileNotFoundError:
                _log(f"macmon nicht gefunden unter {macmon_bin!r} - "
                     "sende Temperatur/GPU/Power/Fan als Platzhalter")
                return
            except Exception as exc:
                _log(f"macmon-Subprozess-Fehler: {exc!r} - Neustart in 3s")
                with self.lock:
                    self._proc = None
                time.sleep(3)

    def _watchdog(self):
        """Erkennt einen haengenden macmon-Prozess (laeuft noch, liefert
        aber seit MACMON_STALE_SEC keine neue Zeile mehr) und erzwingt
        per proc.kill() ein EOF auf dem Pipe, damit die blockierende
        Zeilen-Schleife in _run() sich beendet und neu startet."""
        while not self._stop:
            time.sleep(1.0)
            with self.lock:
                stale = (time.monotonic() - self._last_update_monotonic) > MACMON_STALE_SEC
            if stale:
                self._restart("kein Wert seit 5s")

    def _restart(self, reason, bypass_rate_limit=False):
        now = time.monotonic()
        with self.lock:
            if not bypass_rate_limit and (now - self._last_restart_monotonic) < MACMON_RESTART_MIN_GAP_SEC:
                return
            self._last_restart_monotonic = now
            self._last_update_monotonic = now  # verhindert sofortiges erneutes Ausloesen
            proc = self._proc
        log_event(f"macmon neu: {reason}")
        if proc is not None:
            try:
                proc.kill()
            except Exception:
                pass

    def restart(self):
        """Von aussen aufgerufen (z.B. direkt beim Aufwachen) - erzwingt
        sofort einen Neustart, unabhaengig von der 10s-Sperre zwischen
        zwei automatischen Watchdog-Neustarts."""
        self._restart("nach Aufwachen", bypass_rate_limit=True)

    def snapshot(self):
        with self.lock:
            return dict(self.latest)

    def stop(self):
        self._stop = True
        with self.lock:
            proc = self._proc
        if proc is not None:
            try:
                proc.kill()
            except Exception:
                pass


def safe_get(d, *keys, default=None):
    cur = d
    for k in keys:
        if not isinstance(cur, dict) or k not in cur:
            return default
        cur = cur[k]
    return cur


class SenderThread(threading.Thread):
    def __init__(self, macmon, usb_link, fan_boost_client, cyd_cmd_listener, is_awake_fn=None):
        super().__init__(daemon=True)
        self.macmon = macmon
        self.usb_link = usb_link
        self.fan_boost_client = fan_boost_client
        self.cyd_cmd_listener = cyd_cmd_listener
        # Auftrag "Ruhezustand-Fix", Schritt 2: liefert False waehrend
        # Systemschlaf UND DarkWake (siehe wake_watch.py) - in dem
        # Fenster verlaesst kein Paket mehr den Mac. Ohne Angabe (z.B.
        # falls PyObjC fehlt) gilt wie bisher "immer wach".
        self.is_awake_fn = is_awake_fn or (lambda: True)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.lock = threading.Lock()
        self.latest_payload = None
        self.paused = False
        self._stop = False
        # Einmalig hochgezaehlter Zaehler (Auftrag "Ruhemodus") - das CYD
        # erkennt am Hochzaehlen (nicht am reinen Vorhandensein), dass ein
        # SOEBEN gesendeter SLEEP-Befehl tatsaechlich angekommen ist, auch
        # wenn zwischendurch kein anderes Feld sich aendert.
        self.sleep_ack_counter = 0
        # Auftrag "Monitor-Umschaltung" - additiv, exakt gleiches Muster
        # wie sleep_ack_counter: das CYD erkennt am Hochzaehlen (nicht
        # am reinen Vorhandensein), dass ein soeben gesendeter MONITOR-
        # Befehl tatsaechlich angekommen ist.
        self.monitor_ack_counter = 0
        self._last_send_monotonic = None
        self._log_fh = None
        # Unicast-Fallback (Messung 29.10.: Broadcast kam im Netz nur zu
        # ~1% an, Unicast zu 100%) - das CYD antwortet auf jedes
        # empfangene UDP-Paket mit seiner eigenen IP; diese Antwort kommt
        # auf demselben Socket zurueck, weil wir ihn fuer alle Sendungen
        # wiederverwenden (kein erneutes bind() noetig).
        self._learned_cyd_ip = None

        if config.DEBUG_LOG:
            log_dir = os.path.join(APP_SUPPORT_DIR, "logs")
            os.makedirs(log_dir, exist_ok=True)
            path = os.path.join(log_dir, "send_log.csv")
            is_new = not os.path.exists(path)
            self._log_fh = open(path, "a", buffering=1)
            if is_new:
                self._log_fh.write("wall_time,gap_since_last_send_s,collect_duration_ms,cpu_pct\n")

    def stop(self):
        self._stop = True

    def set_paused(self, paused):
        self.paused = paused

    def get_latest(self):
        with self.lock:
            return self.latest_payload

    def bump_sleep_ack(self):
        """Von CydCommandListener.on_sleep (anderer Thread) aufgerufen,
        sobald ein gueltiger SLEEP-Befehl verarbeitet wurde."""
        with self.lock:
            self.sleep_ack_counter += 1

    def bump_monitor_ack(self):
        """Von CydCommandListener.on_monitor (anderer Thread) aufgerufen,
        sobald ein gueltiger MONITOR-Befehl verarbeitet wurde (Auftrag
        "Monitor-Umschaltung") - bestaetigt nur "angekommen", nicht
        "DDC sicher gelungen" (das laeuft asynchron weiter, siehe
        monitor_switch.py)."""
        with self.lock:
            self.monitor_ack_counter += 1

    def send_now(self):
        """Sendet SOFORT, ausserhalb des normalen 1Hz-Takts und
        unabhaengig von Pause-/Wach-Zustand (Auftrag "Ruhezustand-Fix",
        Schritt 2: die SLEEP-Bestaetigung MUSS das CYD noch VOR dem
        tatsaechlichen Einschlafen erreichen - "pmset sleepnow" kann
        sonst schneller wirken als der naechste regulaere Sendetick)."""
        payload = self._collect()
        with self.lock:
            self.latest_payload = payload
        try:
            self._send(payload)
        except Exception as exc:
            _log(f"Fehler beim Sofort-Senden (Sleep-Bestaetigung): {exc!r}")
        if self.usb_link.is_connected():
            try:
                self.usb_link.write_line(json.dumps(payload))
                mark_cyd_contact()
            except Exception as exc:
                _log(f"Fehler beim Sofort-Senden per USB: {exc!r}")

    def _collect(self):
        payload = {
            "cpu_pct": psutil.cpu_percent(interval=None),
            "ram_pct": psutil.virtual_memory().percent,
            "disk_pct": psutil.disk_usage("/").percent,
            "cpu_temp_c": None,
            "gpu_temp_c": None,
            "gpu_pct": None,
            "power_w": None,
            "fan_rpm": None,
            "fan_max_rpm": None,
            "fan_boost": 1 if self.fan_boost_client.is_active() else 0,
            "fan_left": self.fan_boost_client.remaining_seconds(),
            "fan_ok": 1 if fan_boost.is_available() else 0,
            # Auftrag "Ruhemodus" - neues, abwaertskompatibles Feld, siehe
            # bump_sleep_ack(). Bestehende Felder bleiben unveraendert.
            "sleep_ack": self.sleep_ack_counter,
            # Auftrag "Monitor-Umschaltung" - neues, abwaertskompatibles
            # Feld wie sleep_ack oben.
            "monitor_ack": self.monitor_ack_counter,
        }

        mm = self.macmon.snapshot()
        if mm:
            cpu_temp = safe_get(mm, "temp", "cpu_temp_avg")
            gpu_temp = safe_get(mm, "temp", "gpu_temp_avg")
            gpu_ratio = mm.get("gpu_scaled_ratio")
            power = mm.get("sys_power")
            fans = mm.get("fans") or []

            if cpu_temp is not None:
                payload["cpu_temp_c"] = round(cpu_temp, 1)
            if gpu_temp is not None:
                payload["gpu_temp_c"] = round(gpu_temp, 1)
            if gpu_ratio is not None:
                payload["gpu_pct"] = round(gpu_ratio * 100, 1)
            if power is not None:
                payload["power_w"] = round(power, 1)
            if fans:
                payload["fan_rpm"] = int(fans[0].get("rpm", 0))
                payload["fan_max_rpm"] = int(fans[0].get("max_rpm", 0))

        return payload

    def _send(self, payload):
        data = json.dumps(payload).encode("utf-8")
        target = config.TARGET_IP if config.TARGET_IP else "<broadcast>"
        self.sock.sendto(data, (target, config.UDP_PORT))

        # Zusaetzlich per Unicast an die vom CYD gelernte IP, sofern
        # keine feste Ziel-IP konfiguriert ist (die hat dann Vorrang).
        if not config.TARGET_IP and self._learned_cyd_ip:
            try:
                self.sock.sendto(data, (self._learned_cyd_ip, config.UDP_PORT))
            except Exception as exc:
                _log(f"Unicast-Sendefehler: {exc!r}")

    def _learn_cyd_ip(self):
        """Liest alle gerade verfuegbaren Antworten des CYD (es schickt
        auf jedes empfangene Paket seine eigene IP zurueck, siehe
        CLAUDE.md "Stolperfallen") und merkt sich die zuletzt gemeldete -
        kurzer Timeout, blockiert den 1s-Takt nur minimal."""
        if config.TARGET_IP:
            return
        self.sock.settimeout(0.15)
        try:
            while True:
                data, _addr = self.sock.recvfrom(128)
                ip = data.decode("utf-8", errors="ignore").strip()
                if ip and ip != self._learned_cyd_ip:
                    _log(f"CYD-IP gelernt: {ip}")
                if ip:
                    self._learned_cyd_ip = ip
                    mark_cyd_contact()
        except socket.timeout:
            pass
        except Exception:
            pass
        finally:
            self.sock.settimeout(None)

    def run(self):
        # Erster Aufruf "waermt" psutil.cpu_percent() auf, sonst liefert
        # der allererste Tick 0.0.
        psutil.cpu_percent(interval=None)

        next_tick = time.monotonic()
        while not self._stop:
            # Sendepause waehrend Systemschlaf/DarkWake (Auftrag
            # "Ruhezustand-Fix", Schritt 2) - es wird in diesem Fenster
            # weder gemessen noch gesendet. next_tick wird bei jeder
            # Pruefung neu auf "jetzt" gesetzt, damit nach dem Aufwachen
            # sofort (nicht erst beim naechsten regulaeren Tick) wieder
            # gesendet wird - deutlich unter den geforderten 2s.
            if not self.is_awake_fn():
                time.sleep(0.3)
                next_tick = time.monotonic()
                continue

            t0 = time.monotonic()
            try:
                payload = self._collect()
            except Exception as exc:
                _log(f"Fehler beim Erfassen der Werte: {exc!r}")
                payload = self.get_latest() or {}
            collect_ms = (time.monotonic() - t0) * 1000.0

            with self.lock:
                self.latest_payload = payload

            if not self.paused and payload:
                try:
                    self._send(payload)
                    now = time.monotonic()
                    gap = (now - self._last_send_monotonic) if self._last_send_monotonic else None
                    self._last_send_monotonic = now
                    if self._log_fh:
                        gap_str = f"{gap:.3f}" if gap is not None else ""
                        self._log_fh.write(
                            f"{time.time():.3f},{gap_str},{collect_ms:.1f},{payload.get('cpu_pct', '')}\n"
                        )
                except Exception as exc:
                    _log(f"Fehler beim Senden: {exc!r}")

                # Zusaetzlich zum WLAN-Broadcast dieselbe Zeile per USB,
                # falls ein Eiswolfs iBoard gerade am Kabel bestaetigt ist - im
                # selben Takt, kein eigener Thread (siehe Auftrag).
                if self.usb_link.is_connected():
                    self.usb_link.write_line(json.dumps(payload))
                    mark_cyd_contact()

                self._learn_cyd_ip()

            # Vom CYD per USB-Zeile gesendete Luefter-Boost-Befehle
            # ("FAN BOOST ..."/"FAN AUTO ..."), unabhaengig vom
            # Pause-Zustand entgegennehmen - siehe fan_boost.py.
            for line in self.usb_link.read_lines():
                mark_cyd_contact()
                self.cyd_cmd_listener.handle_line(line)

            next_tick += config.SEND_INTERVAL_SECONDS
            sleep_for = next_tick - time.monotonic()
            if sleep_for > 0:
                time.sleep(sleep_for)
            else:
                # Takt verpasst (z.B. durch einen Schlaf/Standby) - neu
                # synchronisieren statt in schneller Folge aufzuholen.
                next_tick = time.monotonic()


def _install_sleep_watcher(on_sleep):
    """Beendet einen laufenden Luefter-Boost sofort, wenn der Mac in
    den Ruhezustand geht (Auftrag) - ueber die normale macOS-
    Schlaf-Benachrichtigung (NSWorkspace). Gibt den Observer zurueck
    (muss am Leben gehalten werden), oder None, falls PyObjC aus
    irgendeinem Grund nicht verfuegbar ist - dann gilt die Regel
    einfach nicht, kein Absturz."""
    try:
        from AppKit import NSWorkspace, NSWorkspaceWillSleepNotification
        from Foundation import NSObject
    except Exception as exc:
        _log(f"Schlaf-Erkennung nicht verfuegbar: {exc!r}")
        return None

    class _SleepObserver(NSObject):
        def handleSleep_(self, _notification):
            try:
                on_sleep()
            except Exception as exc:
                _log(f"Fehler in der Schlaf-Behandlung: {exc!r}")

    observer = _SleepObserver.alloc().init()
    NSWorkspace.sharedWorkspace().notificationCenter().addObserver_selector_name_object_(
        observer, "handleSleep:", NSWorkspaceWillSleepNotification, None
    )
    return observer


class IBoardApp(rumps.App):
    def __init__(self):
        self.menubar_item = config.get_menubar_item()
        super().__init__("Eiswolfs iBoard", title=format_menubar_title(self.menubar_item, {}))
        self.pause_item = rumps.MenuItem(i18n.t("pause_start"), callback=self.toggle_pause)
        self.usb_item = rumps.MenuItem(i18n.t("usb_release"), callback=self.toggle_usb_release)
        self.link_item = rumps.MenuItem(i18n.t("link_prefix") + i18n.t("link_wifi"))
        self.cpu_item = rumps.MenuItem(f"{i18n.t('label_cpu')}: --")
        self.ram_item = rumps.MenuItem(f"{i18n.t('label_ram')}: --")
        self.disk_item = rumps.MenuItem(f"{i18n.t('label_disk')}: --")
        self.cpu_temp_item = rumps.MenuItem(f"{i18n.t('label_cpu_temp')}: --")
        self.gpu_temp_item = rumps.MenuItem(f"{i18n.t('label_gpu_temp')}: --")
        self.gpu_item = rumps.MenuItem(f"{i18n.t('label_gpu')}: --")
        self.power_item = rumps.MenuItem(f"{i18n.t('label_power')}: --")
        self.fan_item = rumps.MenuItem(f"{i18n.t('label_fan')}: --")
        # Nicht anklickbarer Fuss-Eintrag (callback=None -> rumps zeigt ihn
        # ausgegraut/inaktiv) - Auftrag "Name unten ergaenzen".
        self.about_item = rumps.MenuItem(f"Eiswolfs iBoard v{APP_VERSION}", callback=None)

        # Untermenue "In Menueleiste anzeigen" - waehlt, welcher Wert im
        # Icon oben steht (Haken an der aktiven Wahl). Der Schluessel
        # fuer select_menubar_item() haengt am MenuItem selbst, da rumps
        # ueber den Titel (den Label-Text) adressiert, nicht den Key.
        self.menubar_choice_menu = rumps.MenuItem(i18n.t("menubar_choice_title"))
        for key, label_key in MENUBAR_CHOICES:
            item = rumps.MenuItem(i18n.t(label_key), callback=self.select_menubar_item)
            item.menubar_key = key
            item.state = (key == self.menubar_item)
            self.menubar_choice_menu.add(item)

        # Luefter-Boost (Auftrag "Luefter-Boost") - sicherer Anfangs-
        # zustand: ein evtl. noch laufender Boost (z.B. vom CYD
        # gestartet, bevor die App neu gestartet wurde) wird sofort
        # beendet, bevor irgendetwas anderes passiert.
        fan_boost.reset_to_auto_on_startup()
        self.fan_boost = fan_boost.FanBoostClient(
            on_change=self._on_fan_boost_change,
            cyd_idle_seconds=cyd_idle_seconds,
        )
        self.fan_boost_sec = config.get_fan_boost_sec()
        self.fan_boost_rpm = config.get_fan_boost_rpm()
        # Auftrag "Monitor-Umschaltung" - eigener Switcher-Zustand (Rate-
        # Begrenzung ueber mehrere Befehle hinweg), siehe monitor_switch.py.
        self.monitor_switch = monitor_switch.MonitorSwitcher()
        self.cyd_cmd_listener = fan_boost.CydCommandListener(
            on_boost=self._on_cyd_fan_boost,
            on_auto=self._on_cyd_fan_auto,
            on_sleep=self._on_cyd_sleep,
            on_monitor=self._on_cyd_monitor,
        )

        self.fan_boost_menu = rumps.MenuItem(i18n.t("fan_boost_menu"))
        self.fan_boost_toggle_item = rumps.MenuItem(i18n.t("fan_boost_start"), callback=self.toggle_fan_boost)
        self.fan_boost_menu.add(self.fan_boost_toggle_item)

        self.fan_boost_duration_menu = rumps.MenuItem(i18n.t("fan_boost_duration_menu"))
        for sec in config.FAN_BOOST_DURATIONS:
            item = rumps.MenuItem(fan_boost.duration_label(sec), callback=self.select_fan_boost_duration)
            item.fan_boost_sec = sec
            item.state = (sec == self.fan_boost_sec)
            self.fan_boost_duration_menu.add(item)
        self.fan_boost_menu.add(self.fan_boost_duration_menu)

        self.fan_boost_rpm_menu = rumps.MenuItem(i18n.t("fan_boost_rpm_menu"))
        for level in config.FAN_BOOST_RPM_LEVELS:
            item = rumps.MenuItem(fan_boost.rpm_label(level), callback=self.select_fan_boost_rpm)
            item.fan_boost_rpm = level
            item.state = (level == self.fan_boost_rpm)
            self.fan_boost_rpm_menu.add(item)
        self.fan_boost_menu.add(self.fan_boost_rpm_menu)

        # Sprache (Auftrag "Sprache DE/EN") und Einheit (Auftrag
        # "Celsius/Fahrenheit") - je ein Untermenue mit zwei Optionen,
        # Haken an der aktiven Wahl, gleiches Muster wie oben.
        self.language_menu = rumps.MenuItem(i18n.t("language_menu"))
        self.language_de_item = rumps.MenuItem(i18n.t("language_de"), callback=self.select_language)
        self.language_de_item.lang_code = "de"
        self.language_en_item = rumps.MenuItem(i18n.t("language_en"), callback=self.select_language)
        self.language_en_item.lang_code = "en"
        self.language_menu.add(self.language_de_item)
        self.language_menu.add(self.language_en_item)

        self.units_menu = rumps.MenuItem(i18n.t("units_menu"))
        self.units_c_item = rumps.MenuItem(i18n.t("units_c"), callback=self.select_temp_unit)
        self.units_c_item.unit_code = "c"
        self.units_f_item = rumps.MenuItem(i18n.t("units_f"), callback=self.select_temp_unit)
        self.units_f_item.unit_code = "f"
        self.units_menu.add(self.units_c_item)
        self.units_menu.add(self.units_f_item)
        self._refresh_language_checkmarks()
        self._refresh_units_checkmarks()

        self.sleep_item = rumps.MenuItem(i18n.t("sleep_now"), callback=self.trigger_sleep_now)

        # Auftrag "Token nicht abtippen muessen" - einmalige, 60s
        # befristete lokale Freigabe der config.json fuers Windows-
        # Pendant (siehe token_share.py), statt den Token manuell
        # zwischen zwei Rechnern abzutippen.
        self.share_token_item = rumps.MenuItem(i18n.t("share_token_menu"), callback=self.share_token)

        # Schutz gegen Befehlswiederholung (Auftrag "Sicherheit und
        # Lizenzen", Teil 2) - eigenes Untermenue.
        self.security_menu = rumps.MenuItem(i18n.t("security_menu"))
        self.resync_item = rumps.MenuItem(i18n.t("resync_menu"), callback=self.allow_resync)
        self.security_menu.add(self.resync_item)
        self.legacy_item = rumps.MenuItem(i18n.t("legacy_menu"), callback=self.toggle_legacy)
        self.legacy_item.state = config.get_allow_legacy_commands()
        self.security_menu.add(self.legacy_item)

        # Auftrag "Monitor-Umschaltung": Ein/Aus, Bildschirmauswahl (per
        # UUID, siehe config.py/monitor_switch.py - Sicherheitsgrenze
        # "nur der ausgewaehlte Bildschirm"), die vier VCP-0x60-
        # Rohwerte, und ein Test-Knopf mit automatischem Zurueckschalten.
        self.monitor_enabled_item = rumps.MenuItem(i18n.t("monitor_enabled"), callback=self.toggle_monitor_switch)
        self.monitor_enabled_item.state = config.get_monitor_switch_enabled()

        self.monitor_screen_menu = rumps.MenuItem(i18n.t("monitor_screen_menu"))
        self._rebuild_monitor_screen_menu()

        self.monitor_values_menu = rumps.MenuItem(i18n.t("monitor_values_menu"))
        self.monitor_value_items = {}
        for logical, key in (
            (1, "monitor_value_hdmi1"), (2, "monitor_value_hdmi2"),
            (3, "monitor_value_dp1"), (4, "monitor_value_dp2"),
        ):
            item = rumps.MenuItem(
                i18n.t(key, value=config.get_monitor_vcp_value(logical)),
                callback=self.edit_monitor_value,
            )
            item.monitor_logical_input = logical
            item.monitor_label_key = key
            self.monitor_values_menu.add(item)
            self.monitor_value_items[logical] = item

        self.monitor_test_item = rumps.MenuItem(i18n.t("monitor_test_item"), callback=self.test_monitor_switch)

        self.monitor_menu = rumps.MenuItem(i18n.t("monitor_menu"))
        self.monitor_menu.add(self.monitor_enabled_item)
        self.monitor_menu.add(self.monitor_screen_menu)
        self.monitor_menu.add(self.monitor_values_menu)
        self.monitor_menu.add(self.monitor_test_item)

        menu_items = [
            self.link_item,
            self.cpu_item,
            self.ram_item,
            self.disk_item,
            self.cpu_temp_item,
            self.gpu_temp_item,
            self.gpu_item,
            self.power_item,
            self.fan_item,
            None,
            self.menubar_choice_menu,
            self.language_menu,
            self.units_menu,
            None,
            self.pause_item,
            self.usb_item,
            None,
            self.fan_boost_menu,
            self.sleep_item,
            self.monitor_menu,
            self.share_token_item,
            self.security_menu,
        ]

        # "Beim Login starten" nur sinnvoll/verfuegbar, wenn das hier
        # wirklich die gebaute .app ist (nicht das venv-Entwicklerprojekt
        # - dafuer gibt es install.sh/uninstall.sh).
        if login_item.is_available():
            self.login_item_menu = rumps.MenuItem(i18n.t("login_item"), callback=self.toggle_login_item)
            self.login_item_menu.state = login_item.is_enabled()
            menu_items += [None, self.login_item_menu]

        # Eingedeutschter/uebersetzter, selbst gebauter Beenden-Eintrag
        # statt des automatischen rumps-Quit-Buttons - so kann vor dem
        # tatsaechlichen Beenden noch ein laufender Luefter-Boost
        # beendet werden (Auftrag: "Boost endet sofort, wenn die
        # Mac-App beendet wird").
        self.quit_button = None
        self.quit_item = rumps.MenuItem(i18n.t("quit"), callback=self.quit_app)

        menu_items += [None, self.about_item, self.quit_item]
        self.menu = menu_items

        self.macmon = MacmonReader()
        self.usb_link = UsbLink()
        # Auftrag "Ruhezustand-Fix", Schritt 2+3: eigenstaendige Erkennung
        # von Systemschlaf/DarkWake (siehe wake_watch.py), UNABHAENGIG
        # vom bestehenden _install_sleep_watcher()/_on_system_sleep()
        # weiter unten (der kuemmert sich nur um den Luefter-Boost und
        # bleibt bewusst unveraendert). Beim Aufwachen wird macmon sofort
        # neu gestartet, auch wenn der letzte Watchdog-Neustart noch
        # keine 10s her ist.
        self._wake_watcher = wake_watch.WakeWatcher(on_wake=self.macmon.restart)
        self.sender = SenderThread(
            self.macmon, self.usb_link, self.fan_boost, self.cyd_cmd_listener,
            is_awake_fn=self._wake_watcher.is_awake,
        )
        self.sender.start()

        # Reine UI-Aktualisierung (Menueleiste) - liest nur den
        # zwischengespeicherten Payload des SenderThread, misst nichts
        # selbst und beeinflusst die Sendetaktung nicht.
        self.ui_timer = rumps.Timer(self.update_ui, config.SEND_INTERVAL_SECONDS)
        self.ui_timer.start()

        self._sleep_watcher = _install_sleep_watcher(self._on_system_sleep)

    def toggle_pause(self, sender):
        paused = not self.sender.paused
        self.sender.set_paused(paused)
        sender.title = i18n.t("pause_stop") if paused else i18n.t("pause_start")

    def toggle_login_item(self, sender):
        try:
            if login_item.is_enabled():
                login_item.disable()
                sender.state = False
            else:
                login_item.enable()
                sender.state = True
        except Exception as exc:
            _log(f"Autostart konnte nicht umgeschaltet werden: {exc!r}")
            rumps.alert(i18n.t("alert_title"), i18n.t("alert_autostart_failed"))

    def select_menubar_item(self, sender):
        self.menubar_item = sender.menubar_key
        config.set_menubar_item(sender.menubar_key)
        for item in self.menubar_choice_menu.values():
            if item is not None:
                item.state = (item is sender)
        self.update_ui(None)

    def _refresh_language_checkmarks(self):
        lang = config.get_language()
        self.language_de_item.state = (lang == "de")
        self.language_en_item.state = (lang == "en")

    def _refresh_units_checkmarks(self):
        unit = config.get_temp_unit()
        self.units_c_item.state = (unit == "c")
        self.units_f_item.state = (unit == "f")

    def select_language(self, sender):
        # i18n.t() liest config.get_language() bei jedem Aufruf neu - ein
        # Sprachwechsel wirkt dadurch sofort beim naechsten update_ui()
        # und beim naechsten Oeffnen eines Untermenues, OHNE Neustart.
        # Bereits vorhandene MenuItem-Titel (z.B. "Boost starten") werden
        # dafuer hier einmal direkt nachgezogen; neu angelegte Objekte
        # (z.B. das naechste Mal geoeffnete Untermenue) sind ohnehin
        # automatisch aktuell.
        config.set_language(sender.lang_code)
        self._refresh_language_checkmarks()
        self._retranslate_menu()

    def select_temp_unit(self, sender):
        config.set_temp_unit(sender.unit_code)
        self._refresh_units_checkmarks()
        self.update_ui(None)

    def _retranslate_menu(self):
        """Zieht alle fest angelegten (nicht dynamisch neu erzeugten)
        Menuetitel auf die gerade gewaehlte Sprache nach - noetig, weil
        rumps.MenuItem-Titel beim Erzeugen einmalig gesetzt werden, nicht
        bei jedem Zugriff neu uebersetzt werden."""
        self.pause_item.title = i18n.t("pause_stop") if self.sender.paused else i18n.t("pause_start")
        self.usb_item.title = i18n.t("usb_reconnect") if self.usb_link.is_released() else i18n.t("usb_release")
        self.menubar_choice_menu.title = i18n.t("menubar_choice_title")
        for key, label_key in MENUBAR_CHOICES:
            for item in self.menubar_choice_menu.values():
                if item is not None and getattr(item, "menubar_key", None) == key:
                    item.title = i18n.t(label_key)
        self.language_menu.title = i18n.t("language_menu")
        self.language_de_item.title = i18n.t("language_de")
        self.language_en_item.title = i18n.t("language_en")
        self.units_menu.title = i18n.t("units_menu")
        self.units_c_item.title = i18n.t("units_c")
        self.units_f_item.title = i18n.t("units_f")
        self.fan_boost_menu.title = i18n.t("fan_boost_menu")
        self.fan_boost_duration_menu.title = i18n.t("fan_boost_duration_menu")
        self.fan_boost_rpm_menu.title = i18n.t("fan_boost_rpm_menu")
        for item in self.fan_boost_duration_menu.values():
            if item is not None:
                item.title = fan_boost.duration_label(item.fan_boost_sec)
        for item in self.fan_boost_rpm_menu.values():
            if item is not None:
                item.title = fan_boost.rpm_label(item.fan_boost_rpm)
        if login_item.is_available():
            self.login_item_menu.title = i18n.t("login_item")
        self.sleep_item.title = i18n.t("sleep_now")
        self.share_token_item.title = i18n.t("share_token_menu")
        self.security_menu.title = i18n.t("security_menu")
        self.resync_item.title = i18n.t("resync_menu")
        self.legacy_item.title = i18n.t("legacy_menu")
        self.monitor_menu.title = i18n.t("monitor_menu")
        self.monitor_enabled_item.title = i18n.t("monitor_enabled")
        self.monitor_screen_menu.title = i18n.t("monitor_screen_menu")
        self.monitor_values_menu.title = i18n.t("monitor_values_menu")
        for logical, item in self.monitor_value_items.items():
            item.title = i18n.t(item.monitor_label_key, value=config.get_monitor_vcp_value(logical))
        self.monitor_test_item.title = i18n.t("monitor_test_item")
        self.quit_item.title = i18n.t("quit")
        self.update_ui(None)

    def _on_fan_boost_change(self):
        # Wird aus dem FanBoostClient-eigenen Thread aufgerufen -
        # update_ui() liest dessen Zustand ohnehin jede Sekunde neu,
        # hier nur ein sofortiger Anstoss fuer eine zuegige Anzeige.
        self.update_ui(None)

    def _on_cyd_fan_boost(self, seconds, rpm_level):
        if self.fan_boost.is_active():
            self.fan_boost.stop()
        self.fan_boost.start(rpm_level, seconds, started_by_cyd=True)

    def _on_cyd_fan_auto(self):
        self.fan_boost.stop()

    def _do_sleep(self):
        """Gemeinsame Umsetzung fuer CYD-Befehl UND Menuepunkt (Auftrag
        "Ruhemodus jetzt") - beendet zuerst einen evtl. laufenden
        Luefter-Boost (Luefter zurueck auf Automatik), loest dann echten
        System-Sleep aus (siehe mac_sleep.py). Fehler duerfen die App nie
        beenden."""
        try:
            if self.fan_boost.is_active():
                self.fan_boost.stop()
            mac_sleep.sleep_now()
        except Exception as exc:
            _log(f"Fehler beim Ausloesen des Ruhemodus: {exc!r}")

    def _on_cyd_sleep(self):
        # Wird aus dem CydCommandListener-eigenen Hintergrund-Thread
        # aufgerufen (UDP) oder synchron aus SenderThread.run() (USB-
        # Zeile) - sleep_ack wird IMMER hochgezaehlt, sobald der Befehl
        # hier ankam (bestaetigt dem CYD "angekommen", nicht "Sleep
        # garantiert sichtbar gelungen" - siehe mac_sleep.sleep_now()).
        # Reihenfolge seit Auftrag "Ruhezustand-Fix" bewusst: Zaehler
        # hochzaehlen UND sofort (ausserhalb des normalen Takts) senden,
        # BEVOR "pmset sleepnow" ausgeloest wird - sonst kann der Mac
        # schon schlafen, bevor der naechste regulaere Sendetick die
        # Bestaetigung ueberhaupt verschickt haette.
        self.sender.bump_sleep_ack()
        self.sender.send_now()
        self._do_sleep()

    def trigger_sleep_now(self, _sender):
        self._do_sleep()

    def _on_cyd_monitor(self, logical_input):
        # Auftrag "Monitor-Umschaltung" - gleiche Reihenfolge wie bei
        # _on_cyd_sleep: Quittung ZUERST (hochzaehlen + sofort senden),
        # damit das CYD nicht unnoetig auf den vollen 2s-Timeout wartet,
        # DANACH der eigentliche (asynchrone) DDC-Schreibvorgang.
        self.sender.bump_monitor_ack()
        self.sender.send_now()
        self.monitor_switch.request(logical_input)

    def toggle_monitor_switch(self, sender):
        enabled = not config.get_monitor_switch_enabled()
        config.set_monitor_switch_enabled(enabled)
        sender.state = enabled

    def _rebuild_monitor_screen_menu(self):
        """Baut die Bildschirmliste im Untermenue neu auf (beim Start
        UND ueber "Bildschirme neu suchen") - ruft m1ddc jedes Mal neu
        auf, damit neu angeschlossene/umbenannte Bildschirme auftauchen."""
        try:
            self.monitor_screen_menu.clear()
        except AttributeError:
            # rumps legt das interne NSMenu-Untermenue erst beim ERSTEN
            # .add() an (siehe rumps.py::MenuItem.__setitem__) - beim
            # allerersten Aufruf hier (noch nie etwas hinzugefuegt) ist
            # .clear() daher ein Fehlgriff, kein echter Fehlerzustand.
            pass
        rescan_item = rumps.MenuItem(i18n.t("monitor_screen_rescan"), callback=self.rescan_monitor_screens)
        self.monitor_screen_menu.add(rescan_item)
        self.monitor_screen_menu.add(None)

        screens = monitor_switch.list_screens()
        current_uuid = config.get_monitor_switch_screen_uuid()
        if not screens:
            none_item = rumps.MenuItem(i18n.t("monitor_screen_none"))
            none_item.set_callback(None)
            self.monitor_screen_menu.add(none_item)
            return
        for uuid, name in screens:
            item = rumps.MenuItem(name, callback=self.select_monitor_screen)
            item.monitor_screen_uuid = uuid
            item.monitor_screen_name = name
            item.state = (uuid == current_uuid)
            self.monitor_screen_menu.add(item)

    def rescan_monitor_screens(self, _sender):
        self._rebuild_monitor_screen_menu()

    def select_monitor_screen(self, sender):
        config.set_monitor_switch_screen(sender.monitor_screen_uuid, sender.monitor_screen_name)
        for item in self.monitor_screen_menu.values():
            if hasattr(item, "monitor_screen_uuid"):
                item.state = (item is sender)

    def edit_monitor_value(self, sender):
        current = config.get_monitor_vcp_value(sender.monitor_logical_input)
        response = rumps.Window(
            message=i18n.t("monitor_value_prompt_msg"),
            title=i18n.t("monitor_value_prompt_title"),
            default_text=str(current),
            ok=i18n.t("dialog_ok"),
            cancel=i18n.t("dialog_cancel"),
            dimensions=(220, 24),
        ).run()
        if not response.clicked:
            return
        try:
            value = int(response.text.strip())
        except ValueError:
            return
        if not (0 <= value <= 65535):
            return
        config.set_monitor_vcp_value(sender.monitor_logical_input, value)
        sender.title = i18n.t(sender.monitor_label_key, value=value)

    def test_monitor_switch(self, _sender):
        ok, err_key = self.monitor_switch.test_switch_to_hdmi2()
        if ok:
            rumps.alert(i18n.t("alert_title"), i18n.t("monitor_test_started"))
        else:
            rumps.alert(i18n.t("alert_title"), i18n.t(err_key))

    def share_token(self, _sender):
        """Startet die einmalige, 60s befristete lokale Freigabe der
        config.json fuers Windows-Pendant (Auftrag "Token nicht
        abtippen muessen") - zeigt die abzurufende Adresse in einem
        Alert, meldet per zweitem Alert, ob sie tatsaechlich abgerufen
        wurde oder die Zeit ohne Abruf ablief."""
        if not config.get_fan_boost_token():
            rumps.alert(i18n.t("alert_title"), i18n.t("share_token_no_token"))
            return

        def _on_done(served):
            rumps.alert(
                i18n.t("alert_title"),
                i18n.t("share_token_done") if served else i18n.t("share_token_timeout"),
            )

        url = token_share.start(on_done=_on_done)
        rumps.alert(i18n.t("alert_title"), i18n.t("share_token_body", url=url))

    def allow_resync(self, _sender):
        """Oeffnet ein 60s-Fenster, in dem das CYD seinen Befehlszaehler
        neu setzen darf (Auftrag "Sicherheit und Lizenzen", Teil 2b) -
        fuer den Fall, dass das Geraet seinen eigenen Zaehler verloren
        hat (z.B. Werksreset). Am Geraet: Menue -> Einrichtung ->
        "Zaehler neu abgleichen" ANTIPPEN, WAEHREND dieses Fenster
        offen ist."""
        self.cyd_cmd_listener.allow_resync()
        rumps.alert(i18n.t("alert_title"), i18n.t("resync_alert_body"))

    def toggle_legacy(self, sender):
        new_value = not config.get_allow_legacy_commands()
        config.set_allow_legacy_commands(new_value)
        sender.state = new_value
        if new_value:
            rumps.alert(i18n.t("alert_title"), i18n.t("legacy_warning"))

    def toggle_fan_boost(self, sender):
        if self.fan_boost.is_active():
            self.fan_boost.stop()
            return
        if not fan_boost.is_available():
            rumps.alert(i18n.t("alert_title"), i18n.t("alert_fan_boost_unavailable"))
            return
        ok, err = self.fan_boost.start(self.fan_boost_rpm, self.fan_boost_sec, started_by_cyd=False)
        if not ok:
            rumps.alert(i18n.t("alert_title"), i18n.t("alert_fan_boost_start_failed", err=err))

    def select_fan_boost_duration(self, sender):
        self.fan_boost_sec = sender.fan_boost_sec
        config.set_fan_boost_sec(sender.fan_boost_sec)
        for item in self.fan_boost_duration_menu.values():
            if item is not None:
                item.state = (item is sender)

    def select_fan_boost_rpm(self, sender):
        self.fan_boost_rpm = sender.fan_boost_rpm
        config.set_fan_boost_rpm(sender.fan_boost_rpm)
        for item in self.fan_boost_rpm_menu.values():
            if item is not None:
                item.state = (item is sender)

    def _on_system_sleep(self):
        # Auftrag: Boost endet sofort, wenn der Mac in den Ruhezustand geht.
        if self.fan_boost.is_active():
            self.fan_boost.stop()

    def quit_app(self, _sender):
        if self.fan_boost.is_active():
            self.fan_boost.stop()
            time.sleep(0.3)  # kurze Chance fuer den AUTO-Befehl, noch rauszugehen
        rumps.quit_application()

    def toggle_usb_release(self, sender):
        if self.usb_link.is_released():
            self.usb_link.resume()
            sender.title = i18n.t("usb_release")
        else:
            self.usb_link.release(60)
            sender.title = i18n.t("usb_reconnect")

    def update_ui(self, _timer):
        link_label = i18n.t("link_usb") if self.usb_link.is_connected() else i18n.t("link_wifi")
        self.link_item.title = i18n.t("link_prefix") + link_label

        p = self.sender.get_latest()
        if not p:
            return
        self.title = format_menubar_title(self.menubar_item, p)
        na = i18n.t("na")
        self.cpu_item.title = f"{i18n.t('label_cpu')}: {p['cpu_pct']:.0f}%"
        self.ram_item.title = f"{i18n.t('label_ram')}: {p['ram_pct']:.0f}%"
        self.disk_item.title = f"{i18n.t('label_disk')}: {p['disk_pct']:.0f}%"
        self.cpu_temp_item.title = (
            f"{i18n.t('label_cpu_temp')}: {i18n.format_temp(p['cpu_temp_c'])}"
            if p["cpu_temp_c"] is not None else f"{i18n.t('label_cpu_temp')}: {na}"
        )
        self.gpu_temp_item.title = (
            f"{i18n.t('label_gpu_temp')}: {i18n.format_temp(p['gpu_temp_c'])}"
            if p["gpu_temp_c"] is not None else f"{i18n.t('label_gpu_temp')}: {na}"
        )
        self.gpu_item.title = f"{i18n.t('label_gpu')}: {p['gpu_pct']}%" if p["gpu_pct"] is not None else f"{i18n.t('label_gpu')}: {na}"
        self.power_item.title = f"{i18n.t('label_power')}: {p['power_w']} W" if p["power_w"] is not None else f"{i18n.t('label_power')}: {na}"
        self.fan_item.title = f"{i18n.t('label_fan')}: {p['fan_rpm']} RPM" if p["fan_rpm"] is not None else f"{i18n.t('label_fan')}: {na}"

        if self.fan_boost.is_active():
            remaining = self.fan_boost.remaining_seconds()
            suffix = i18n.t("fan_boost_stop_until") if remaining < 0 else i18n.t("fan_boost_stop_remaining", s=remaining)
            self.fan_boost_toggle_item.title = i18n.t("fan_boost_stop", suffix=suffix)
        else:
            available = fan_boost.is_available()
            self.fan_boost_toggle_item.title = i18n.t("fan_boost_start") if available else i18n.t("fan_boost_unavailable")


LOCK_PATH = os.path.join(APP_SUPPORT_DIR, "sender.lock")
_lock_file_handle = None  # muss fuer die Laufzeit der App offen bleiben


def acquire_single_instance_lock():
    """Verhindert zwei gleichzeitig sendende Instanzen (Auftrag
    "Autostart") - ueber einen Datei-Lock statt einer PID-Datei, damit
    kein Wettlauf entsteht, wenn der LaunchAgent laedt, waehrend Alex
    die App gerade von Hand gestartet hat (oder umgekehrt): der
    Betriebssystem-Lock ist atomar, "wer zuerst da ist, bekommt ihn"."""
    global _lock_file_handle
    _lock_file_handle = open(LOCK_PATH, "w")
    try:
        fcntl.flock(_lock_file_handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        _log("Laeuft bereits in einer anderen Instanz - beende mich leise.")
        sys.exit(0)


def check_apple_silicon():
    """Auftrag: auf einem Intel-Mac eine verstaendliche Meldung zeigen
    und sich ordentlich beenden, statt mit unklaren Fehlern (fehlendes
    SMC-Layout, falsche macmon-Architektur) abzustuerzen."""
    import platform
    if platform.machine() != "arm64":
        _log("Kein Apple-Silicon-Mac erkannt - beende mich.")
        try:
            rumps.alert(
                "Eiswolfs iBoard",
                "Eiswolfs iBoard unterstützt nur Macs mit Apple Silicon "
                "(M1 oder neuer). Dieser Mac hat einen Intel-Prozessor.",
            )
        except Exception:
            pass
        sys.exit(1)


if __name__ == "__main__":
    check_apple_silicon()
    acquire_single_instance_lock()
    IBoardApp().run()
