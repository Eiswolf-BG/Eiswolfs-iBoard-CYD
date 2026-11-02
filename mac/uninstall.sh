#!/bin/bash
# Entfernt den Eiswolfs-iBoard-Sender-LaunchAgent wieder.
set -euo pipefail

PLIST_LABEL="com.alexdemurtas.eiswolfsiboard"
PLIST_DEST="$HOME/Library/LaunchAgents/${PLIST_LABEL}.plist"

if [ -f "$PLIST_DEST" ]; then
    launchctl unload "$PLIST_DEST" 2>/dev/null || true
    rm -f "$PLIST_DEST"
    echo "Eiswolfs-iBoard-Sender-LaunchAgent entfernt."
else
    echo "Kein LaunchAgent gefunden ($PLIST_DEST) - nichts zu tun."
fi
