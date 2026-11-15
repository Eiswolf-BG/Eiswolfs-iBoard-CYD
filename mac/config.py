# Einstellungen fuer Eiswolfs iBoard. Seit dem Umbau auf eine
# installierbare App (29.10.) liegen die vom Nutzer aenderbaren Werte
# NICHT mehr in dieser Datei (die steckt bei der installierten App ja
# nur lesbar im Bundle), sondern in
# "~/Library/Application Support/Eiswolfs iBoard/config.json" - wird
# beim allerersten Start mit Standardwerten angelegt, eine bestehende
# Datei wird nie ueberschrieben. Gilt identisch fuer die Entwickler-
# Fassung (dieses venv-Projekt) UND die gebaute .app, beide teilen
# sich denselben Application-Support-Ordner und damit dieselbe Datei.
import json
import os

APP_SUPPORT_DIR = os.path.expanduser("~/Library/Application Support/Eiswolfs iBoard")
CONFIG_PATH = os.path.join(APP_SUPPORT_DIR, "config.json")

# Werden nur verwendet, wenn das jeweilige Feld in config.json fehlt
# (z.B. bei einer sehr alten Datei) oder die Datei gar nicht existiert.
_DEFAULTS = {
    "target_ip": None,
    "debug_log": False,
    "menubar_item": "cpu_pct",
    "fan_boost_sec": 60,
    "fan_boost_rpm": "MAX",
    "language": "de",
    "temp_unit": "c",
    # Auftrag "Monitor-Umschaltung" - additiv, Standard AUS. Die vier
    # VCP-0x60-Rohwerte sind mit den beim Machbarkeitstest bestaetigten
    # "verdoppelten" Dell/Alienware-Werten vorbelegt (HDMI1=4369,
    # HDMI2=4626) - bei anderen Monitormarken i.d.R. die einfachen
    # MCCS-Standardwerte (z.B. 17/18) stattdessen eintragen, siehe
    # Menue "Monitor-Umschaltung...".
    "monitor_switch_enabled": False,
    "monitor_switch_screen_uuid": "",
    "monitor_switch_screen_name": "",
    "monitor_vcp_1": 4369,  # HDMI 1
    "monitor_vcp_2": 4626,  # HDMI 2
    "monitor_vcp_3": 3855,  # DisplayPort 1
    "monitor_vcp_4": 4112,  # DisplayPort 2 (generischer Platzhalter, ungetestet)
}

# Gueltige Werte fuer "menubar_item" (Auftrag "waehlbarer Wert in der
# Menueleiste") - muessen mit den Schluesseln im Sende-Payload
# uebereinstimmen (siehe sender.py::_collect()).
MENUBAR_ITEMS = (
    "cpu_pct", "ram_pct", "disk_pct", "cpu_temp_c",
    "gpu_temp_c", "gpu_pct", "power_w", "fan_rpm",
)

# Sprache (Auftrag "Sprache DE/EN") und Temperatur-Einheit (Auftrag
# "Celsius/Fahrenheit") - beide starten standardmaessig auf "de"/"c",
# manuell umstellbar im Menue (siehe i18n.py).
LANGUAGES = ("de", "en")
TEMP_UNITS = ("c", "f")

# Luefter-Boost (Auftrag "Luefter-Boost"): Dauer-Stufen in Sekunden,
# 0 = "Bis Stopp" (unbegrenzt, siehe fan_boost.py). Drehzahl-Stufen
# muessen exakt zu fand.py::ALLOWED_RPM_KEYWORDS passen.
FAN_BOOST_DURATIONS = (30, 60, 120, 300, 600, 0)
FAN_BOOST_RPM_LEVELS = ("2000", "3000", "4000", "MAX")


def _load():
    try:
        os.makedirs(APP_SUPPORT_DIR, exist_ok=True)
    except OSError:
        return dict(_DEFAULTS)

    if not os.path.exists(CONFIG_PATH):
        try:
            with open(CONFIG_PATH, "w") as f:
                json.dump(_DEFAULTS, f, indent=2)
        except OSError:
            pass
        return dict(_DEFAULTS)

    try:
        with open(CONFIG_PATH) as f:
            data = json.load(f)
        merged = dict(_DEFAULTS)
        merged.update(data)
        return merged
    except (OSError, json.JSONDecodeError):
        return dict(_DEFAULTS)


_cfg = _load()

# Optionale feste Ziel-IP fuer den Versand, falls UDP-Broadcast im
# jeweiligen Netz nicht funktioniert (manche Router/APs blocken ihn).
# None = Broadcast ins lokale Netz (Standardfall, keine feste IP nötig).
TARGET_IP = _cfg.get("target_ip")

# Diagnose-Log (Sendezeitpunkte, Abstand zum letzten Versand, Dauer der
# Werteerfassung) nach logs/send_log.csv - fuer die Fehlersuche bei der
# "springt auf Keine Verbindung"-Meldung. Normalbetrieb: False.
DEBUG_LOG = bool(_cfg.get("debug_log", False))

# Diese beiden sind bewusst keine Nutzereinstellung (Protokoll-
# Konstanten, muessen zum CYD passen) - fest im Code statt in der JSON.
UDP_PORT = 4210
SEND_INTERVAL_SECONDS = 1.0


def _save():
    """Schreibt den kompletten, zusammengefuehrten Einstellungsstand
    zurueck - erhaelt dabei auch Felder, die _DEFAULTS (noch) nicht
    kennt, da _cfg beim Laden mit dict.update() aus der Datei befuellt
    wurde (siehe _load())."""
    try:
        with open(CONFIG_PATH, "w") as f:
            json.dump(_cfg, f, indent=2)
        # Datei enthaelt den Luefter-Boost-Token (siehe secrets.h) - nur
        # Alex' Benutzer soll sie lesen koennen.
        os.chmod(CONFIG_PATH, 0o600)
    except OSError:
        pass


def get_menubar_item():
    value = _cfg.get("menubar_item", "cpu_pct")
    return value if value in MENUBAR_ITEMS else "cpu_pct"


def set_menubar_item(value):
    if value not in MENUBAR_ITEMS:
        return
    _cfg["menubar_item"] = value
    _save()


def get_fan_boost_sec():
    value = _cfg.get("fan_boost_sec", 60)
    return value if value in FAN_BOOST_DURATIONS else 60


def set_fan_boost_sec(value):
    if value not in FAN_BOOST_DURATIONS:
        return
    _cfg["fan_boost_sec"] = value
    _save()


def get_fan_boost_rpm():
    value = _cfg.get("fan_boost_rpm", "MAX")
    return value if value in FAN_BOOST_RPM_LEVELS else "MAX"


def set_fan_boost_rpm(value):
    if value not in FAN_BOOST_RPM_LEVELS:
        return
    _cfg["fan_boost_rpm"] = value
    _save()


def get_fan_boost_token():
    """None, wenn (noch) kein gemeinsamer Token erzeugt wurde - der
    CYD-Befehlsempfang lehnt dann sicherheitshalber JEDEN Befehl ab
    (siehe fan_boost.py). Wird nie geloggt/ausgegeben."""
    return _cfg.get("fan_boost_token")


def get_last_cmd_counter():
    """Schutz gegen Befehlswiederholung (Auftrag "Sicherheit und
    Lizenzen", Teil 2) - zuletzt akzeptierter CYD-Befehlszaehler,
    0 = noch nie ein signierter Befehl angenommen."""
    return int(_cfg.get("last_cmd_counter", 0))


def set_last_cmd_counter(value):
    _cfg["last_cmd_counter"] = int(value)
    _save()


def get_allow_legacy_commands():
    """Standard AUS (Auftrag) - nur fuer die kurze Uebergangszeit, falls
    CYD-Firmware und Mac-App aus irgendeinem Grund NICHT gleichzeitig
    aktualisiert wurden. Bei EIN akzeptiert fan_boost.py auch das alte,
    unsignierte "<Befehl> <Token>"-Format OHNE Wiederholungsschutz."""
    return bool(_cfg.get("allow_legacy_commands", False))


def set_allow_legacy_commands(value):
    _cfg["allow_legacy_commands"] = bool(value)
    _save()


def get_language():
    value = _cfg.get("language", "de")
    return value if value in LANGUAGES else "de"


def set_language(value):
    if value not in LANGUAGES:
        return
    _cfg["language"] = value
    _save()


def get_temp_unit():
    value = _cfg.get("temp_unit", "c")
    return value if value in TEMP_UNITS else "c"


def set_temp_unit(value):
    if value not in TEMP_UNITS:
        return
    _cfg["temp_unit"] = value
    _save()


# Auftrag "Monitor-Umschaltung" - siehe monitor_switch.py fuer die
# eigentliche DDC-Logik, hier nur reine Einstellungs-Verwaltung.

def get_monitor_switch_enabled():
    return bool(_cfg.get("monitor_switch_enabled", False))


def set_monitor_switch_enabled(value):
    _cfg["monitor_switch_enabled"] = bool(value)
    _save()


def get_monitor_switch_screen_uuid():
    """Leer = kein Bildschirm ausgewaehlt - monitor_switch.py tut dann
    bewusst NICHTS (kein Fallback auf "irgendeinen"/"alle Bildschirme",
    siehe Auftrag/Sicherheitsgrenze)."""
    return _cfg.get("monitor_switch_screen_uuid", "") or ""


def get_monitor_switch_screen_name():
    """Nur fuer die Menue-Anzeige - die UUID oben ist die eigentliche,
    verbindliche Auswahl."""
    return _cfg.get("monitor_switch_screen_name", "") or ""


def set_monitor_switch_screen(uuid, name):
    _cfg["monitor_switch_screen_uuid"] = uuid or ""
    _cfg["monitor_switch_screen_name"] = name or ""
    _save()


def get_monitor_vcp_value(logical_input):
    """logical_input: 1=HDMI1, 2=HDMI2, 3=DisplayPort1, 4=DisplayPort2.
    None bei ungueltigem Wert - Aufrufer MUSS das pruefen (kein Default-
    Rateversuch, siehe Sicherheitsgrenze "nur die 4 erlaubten Werte")."""
    if logical_input not in (1, 2, 3, 4):
        return None
    return int(_cfg.get(f"monitor_vcp_{logical_input}", _DEFAULTS[f"monitor_vcp_{logical_input}"]))


def set_monitor_vcp_value(logical_input, value):
    if logical_input not in (1, 2, 3, 4):
        return
    try:
        value = int(value)
    except (TypeError, ValueError):
        return
    if not (0 <= value <= 65535):
        return
    _cfg[f"monitor_vcp_{logical_input}"] = value
    _save()
