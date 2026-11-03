"""Zentrale, zweisprachige Textliste fuer die Mac-App (Auftrag "Sprache
DE/EN"). t(key, **kwargs) liest die aktuell in config.json gespeicherte
Sprache bei JEDEM Aufruf neu (kein Caching) - ein Sprachwechsel im Menue
wirkt dadurch sofort beim naechsten update_ui()-Durchlauf, ohne
Neustart. Templates mit Platzhaltern nutzen Python-str.format()."""
import config

_STRINGS = {
    "de": {
        "pause_start": "Senden pausieren",
        "pause_stop": "Senden fortsetzen",
        "usb_release": "USB freigeben",
        "usb_reconnect": "USB wieder verbinden",
        "link_prefix": "Weg: ",
        "link_usb": "USB",
        "link_wifi": "WLAN",
        "na": "n/a",
        "label_cpu": "CPU",
        "label_ram": "RAM",
        "label_disk": "Disk",
        "label_cpu_temp": "CPU-Temp",
        "label_gpu_temp": "GPU-Temp",
        "label_gpu": "GPU-Last",
        "label_power": "Leistung",
        "label_fan": "Luefter",
        "menubar_choice_title": "In Menüleiste anzeigen",
        "fan_boost_menu": "Lüfter-Boost",
        "fan_boost_start": "Boost starten",
        "fan_boost_unavailable": "Boost starten (nicht verfügbar)",
        "fan_boost_stop": "Boost stoppen ({suffix})",
        "fan_boost_stop_until": "bis Stopp",
        "fan_boost_stop_remaining": "noch {s}s",
        "fan_boost_duration_menu": "Dauer",
        "fan_boost_rpm_menu": "Drehzahl",
        "duration_until_stop": "Bis Stopp",
        "duration_seconds": "{s} Sekunden",
        "duration_minute": "{m} Minute",
        "duration_minutes": "{m} Minuten",
        "rpm_max": "Vollgas",
        "rpm_level": "{level} RPM",
        "sleep_now": "Ruhemodus jetzt",
        "login_item": "Beim Login starten",
        "quit": "Beenden",
        "language_menu": "Sprache",
        "language_de": "Deutsch",
        "language_en": "English",
        "units_menu": "Einheit",
        "units_c": "Celsius (°C)",
        "units_f": "Fahrenheit (°F)",
        "alert_title": "Eiswolfs iBoard",
        "alert_autostart_failed": "Der Autostart konnte nicht geändert werden. Bitte später erneut versuchen.",
        "alert_fan_boost_unavailable": "Lüfter-Boost nicht verfügbar - der Hilfsdienst ist nicht installiert (siehe tools/fan-helper).",
        "alert_fan_boost_start_failed": "Lüfter-Boost konnte nicht gestartet werden: {err}",
        "share_token_menu": "Token an Windows senden…",
        "share_token_no_token": "Noch kein eigener Token gesetzt - siehe docs/TOKEN.md, um erst einen für Mac-App und CYD anzulegen.",
        "share_token_body": "Öffne diese Adresse innerhalb von 60 Sekunden im Browser auf deinem Windows-Laptop:\n\n{url}\n\nDie heruntergeladene Datei als config.json nach %APPDATA%\\EiswolfsIBoard\\ verschieben (vorhandene Datei ersetzen).",
        "share_token_done": "Übertragen! Die Datei wurde einmal heruntergeladen und die Freigabe ist jetzt schon wieder geschlossen.",
        "share_token_timeout": "Zeit abgelaufen (60s) ohne Abruf - die Freigabe ist jetzt geschlossen. Einfach nochmal auf \"Token an Windows senden…\" klicken.",
    },
    "en": {
        "pause_start": "Pause sending",
        "pause_stop": "Resume sending",
        "usb_release": "Release USB",
        "usb_reconnect": "Reconnect USB",
        "link_prefix": "Path: ",
        "link_usb": "USB",
        "link_wifi": "WiFi",
        "na": "n/a",
        "label_cpu": "CPU",
        "label_ram": "RAM",
        "label_disk": "Disk",
        "label_cpu_temp": "CPU Temp",
        "label_gpu_temp": "GPU Temp",
        "label_gpu": "GPU Load",
        "label_power": "Power",
        "label_fan": "Fan",
        "menubar_choice_title": "Show in menu bar",
        "fan_boost_menu": "Fan Boost",
        "fan_boost_start": "Start boost",
        "fan_boost_unavailable": "Start boost (not available)",
        "fan_boost_stop": "Stop boost ({suffix})",
        "fan_boost_stop_until": "until stopped",
        "fan_boost_stop_remaining": "{s}s left",
        "fan_boost_duration_menu": "Duration",
        "fan_boost_rpm_menu": "Speed",
        "duration_until_stop": "Until stopped",
        "duration_seconds": "{s} seconds",
        "duration_minute": "{m} minute",
        "duration_minutes": "{m} minutes",
        "rpm_max": "Max speed",
        "rpm_level": "{level} RPM",
        "sleep_now": "Sleep now",
        "login_item": "Start at login",
        "quit": "Quit",
        "language_menu": "Language",
        "language_de": "Deutsch",
        "language_en": "English",
        "units_menu": "Units",
        "units_c": "Celsius (°C)",
        "units_f": "Fahrenheit (°F)",
        "alert_title": "Eiswolfs iBoard",
        "alert_autostart_failed": "Could not change autostart. Please try again later.",
        "alert_fan_boost_unavailable": "Fan boost not available - the helper service is not installed (see tools/fan-helper).",
        "alert_fan_boost_start_failed": "Could not start fan boost: {err}",
        "share_token_menu": "Send token to Windows…",
        "share_token_no_token": "No token set yet - see docs/TOKEN.md to create one for the Mac app and CYD first.",
        "share_token_body": "Open this address within 60 seconds in a browser on your Windows laptop:\n\n{url}\n\nMove the downloaded file to %APPDATA%\\EiswolfsIBoard\\ as config.json (replace the existing file).",
        "share_token_done": "Sent! The file was downloaded once and the share is already closed again.",
        "share_token_timeout": "Timed out (60s) without being picked up - the share is now closed. Just click \"Send token to Windows…\" again.",
    },
}


def t(key, **kwargs):
    lang = config.get_language()
    table = _STRINGS.get(lang, _STRINGS["de"])
    template = table.get(key, _STRINGS["de"].get(key, key))
    return template.format(**kwargs) if kwargs else template


def format_temp(celsius):
    """Formatiert einen Temperaturwert gemaess der gewaehlten Einheit
    (config.get_temp_unit(), 'c'/'f') - rundet auf ganze Grad, ohne
    Einheitenbuchstaben bei Celsius (bisheriger, knapper Stil), mit "F"
    bei Fahrenheit (Unterscheidung noetig, da sonst nicht erkennbar)."""
    if config.get_temp_unit() == "f":
        return f"{celsius * 9.0 / 5.0 + 32.0:.0f}°F"
    return f"{celsius:.0f}°C"
