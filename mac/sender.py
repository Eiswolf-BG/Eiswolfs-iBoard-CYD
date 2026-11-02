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
from usb_link import UsbLink


APP_VERSION = "1.0"

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


class MacmonReader:
    """Liest macmon im Hintergrund aus (ein dauerhafter Subprozess statt
    einem Aufruf pro Sekunde) und haelt die letzten Werte bereit. Startet
    den Prozess bei Bedarf neu, falls er abstuerzt - ein Fehler hier darf
    die App nie zum Absturz bringen. self.latest bleibt beim Neustart
    erhalten (wird nie zurueckgesetzt), damit kurze Aussetzer nicht zu
    einem ploetzlichen Verlust aller Temperatur-/GPU-/Power-/Fan-Werte
    fuehren."""

    def __init__(self):
        self.lock = threading.Lock()
        self.latest = {}
        self._stop = False
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

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
                for line in proc.stdout:
                    if self._stop:
                        break
                    try:
                        data = json.loads(line)
                        with self.lock:
                            self.latest = data
                    except (json.JSONDecodeError, ValueError):
                        continue
                proc.wait(timeout=1)
            except FileNotFoundError:
                _log(f"macmon nicht gefunden unter {macmon_bin!r} - "
                     "sende Temperatur/GPU/Power/Fan als Platzhalter")
                return
            except Exception as exc:
                _log(f"macmon-Subprozess-Fehler: {exc!r} - Neustart in 3s")
                time.sleep(3)

    def snapshot(self):
        with self.lock:
            return dict(self.latest)

    def stop(self):
        self._stop = True


def safe_get(d, *keys, default=None):
    cur = d
    for k in keys:
        if not isinstance(cur, dict) or k not in cur:
            return default
        cur = cur[k]
    return cur


class SenderThread(threading.Thread):
    def __init__(self, macmon, usb_link, fan_boost_client, cyd_cmd_listener):
        super().__init__(daemon=True)
        self.macmon = macmon
        self.usb_link = usb_link
        self.fan_boost_client = fan_boost_client
        self.cyd_cmd_listener = cyd_cmd_listener
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
        self.cyd_cmd_listener = fan_boost.CydCommandListener(
            on_boost=self._on_cyd_fan_boost,
            on_auto=self._on_cyd_fan_auto,
            on_sleep=self._on_cyd_sleep,
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
        self.sender = SenderThread(self.macmon, self.usb_link, self.fan_boost, self.cyd_cmd_listener)
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
        self._do_sleep()
        self.sender.bump_sleep_ack()

    def trigger_sleep_now(self, _sender):
        self._do_sleep()

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
