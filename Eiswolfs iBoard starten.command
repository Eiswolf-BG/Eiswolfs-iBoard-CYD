#!/bin/bash
# Startet die Eiswolfs-iBoard-Menueleisten-App per Doppelklick im
# Finder. Das Terminal-Fenster schliesst sich danach von selbst
# (osascript). Laeuft die App schon (von Hand oder per Autostart),
# beendet sie sich selbst leise - siehe sender.py::acquire_single_instance_lock().
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MAC_DIR="$DIR/mac"

if [ ! -x "$MAC_DIR/venv/bin/python3" ]; then
    osascript -e 'display dialog "Eiswolfs iBoard: Die virtuelle Umgebung fehlt.\n\nBitte einmalig im Terminal ausfuehren:\ncd \"'"$MAC_DIR"'\" && python3 -m venv venv && ./venv/bin/pip install -r requirements.txt" buttons {"OK"} default button "OK"'
    exit 1
fi

nohup "$MAC_DIR/venv/bin/python3" "$MAC_DIR/sender.py" > /dev/null 2>&1 &
disown

sleep 1
osascript -e 'display notification "Eiswolfs iBoard laeuft jetzt in der Menueleiste." with title "Eiswolfs iBoard"' 2>/dev/null

# Schliesst das vom Doppelklick geoeffnete Terminal-Fenster wieder.
osascript -e 'tell application "Terminal" to close (every window whose name contains "Eiswolfs iBoard starten")' 2>/dev/null
