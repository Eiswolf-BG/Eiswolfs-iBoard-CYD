#!/bin/bash
# Entfernt den Eiswolfs-iBoard-Luefter-Hilfsdienst rueckstandslos und
# stellt den Luefter auf Automatik - Alex fuehrt dieses Skript SELBST
# mit sudo aus.
set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
    echo "Bitte mit sudo ausfuehren:"
    echo "  sudo bash \"$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/uninstall.sh\""
    exit 1
fi

LABEL="com.alexdemurtas.eiswolfsiboard.fanhelper"
PLIST_DEST="/Library/LaunchDaemons/${LABEL}.plist"

# launchctl bootout sendet SIGTERM an fand.py - dessen eigener
# Signal-Handler/atexit stellt dabei bereits auf Automatik zurueck.
launchctl bootout system "$PLIST_DEST" 2>/dev/null || true

rm -f "$PLIST_DEST"
rm -f /var/run/eiswolfs-iboard-fan.sock

echo "Luefter-Hilfsdienst entfernt, Luefter sollte auf Automatik stehen."
echo "Pruefen: python3 \"$(cd "$(dirname "${BASH_SOURCE[0]}")/../fan-test" && pwd)/fan_control.py\" status"
