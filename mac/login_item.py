"""Autostart ("Beim Login starten") fuer die INSTALLIERTE Eiswolfs-
iBoard.app - ein eigener LaunchAgent, bewusst mit einem ANDEREN Label
als der Entwickler-LaunchAgent (com.alexdemurtas.eiswolfsiboard), damit
beide nie verwechselt werden. Kein Skript, kein Terminal noetig - alles
aus der App selbst heraus. Zeigt auf die .app im Programme-Ordner (den
tatsaechlichen Pfad, unter dem die laufende App gerade ausgefuehrt
wird - funktioniert damit unabhaengig davon, ob sie wirklich in
/Applications liegt oder woanders hin gezogen wurde).

Bewusst ein LaunchAgent-Plist statt SMAppService: braucht kein
zusaetzliches PyObjC-Framework (ServiceManagement ist kein
Standardbestandteil der ueblichen rumps/pyobjc-Installation) und ist
genauso zuverlaessig - "RunAtLoad" ohne "KeepAlive" startet die App
einmal beim Login, startet sie aber NIE automatisch neu, wenn der
Nutzer sie ueber "Beenden" schliesst."""
import os
import plistlib
import subprocess
import sys

LABEL = "com.alexdemurtas.eiswolfsiboard.app"
PLIST_PATH = os.path.expanduser(f"~/Library/LaunchAgents/{LABEL}.plist")


def _app_bundle_executable():
    """Pfad zur ausfuehrbaren Datei innerhalb der .app, so wie sie
    GERADE laeuft (sys.executable zeigt bei einer gebauten .app auf
    Contents/MacOS/<Name>)."""
    return sys.executable


def is_available():
    """True nur im gebuendelten .app-Betrieb - im venv/Entwickler-
    Betrieb gibt es keine "App" zum Autostarten (dafuer existieren
    install.sh/uninstall.sh)."""
    return getattr(sys, "frozen", False)


def is_enabled():
    return os.path.exists(PLIST_PATH)


def enable():
    exe = _app_bundle_executable()
    plist = {
        "Label": LABEL,
        "ProgramArguments": [exe],
        "RunAtLoad": True,
        # Bewusst KEIN KeepAlive - siehe Moduldoc.
    }
    os.makedirs(os.path.dirname(PLIST_PATH), exist_ok=True)
    with open(PLIST_PATH, "wb") as f:
        plistlib.dump(plist, f)
    try:
        subprocess.run(["launchctl", "unload", PLIST_PATH], capture_output=True)
        subprocess.run(["launchctl", "load", PLIST_PATH], capture_output=True, check=True)
    except Exception:
        # Plist steht trotzdem - wirkt spaetestens beim naechsten Login.
        pass


def disable():
    if not os.path.exists(PLIST_PATH):
        return
    try:
        subprocess.run(["launchctl", "unload", PLIST_PATH], capture_output=True)
    except Exception:
        pass
    try:
        os.remove(PLIST_PATH)
    except OSError:
        pass
