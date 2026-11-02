#!/bin/bash
# Richtet die Eiswolfs-iBoard-Sender-App als LaunchAgent ein (Autostart
# bei Login). Bitte erst von Hand testen (python3 sender.py), bevor du
# das hier laufen laesst.
set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLIST_LABEL="com.alexdemurtas.eiswolfsiboard"
PLIST_DEST="$HOME/Library/LaunchAgents/${PLIST_LABEL}.plist"
LOG_DIR="$DIR/logs"

mkdir -p "$LOG_DIR"

if [ ! -x "$DIR/venv/bin/python3" ]; then
    echo "Fehler: venv nicht gefunden unter $DIR/venv - bitte zuerst:"
    echo "  cd \"$DIR\" && python3 -m venv venv && ./venv/bin/pip install -r requirements.txt"
    exit 1
fi

sed \
    -e "s#__PYTHON_PATH__#$DIR/venv/bin/python3#g" \
    -e "s#__SENDER_PATH__#$DIR/sender.py#g" \
    -e "s#__LOG_DIR__#$LOG_DIR#g" \
    "$DIR/${PLIST_LABEL}.plist" > "$PLIST_DEST"

launchctl unload "$PLIST_DEST" 2>/dev/null || true
launchctl load "$PLIST_DEST"

echo "Eiswolfs-iBoard-Sender als LaunchAgent eingerichtet und gestartet."
echo "Logs: $LOG_DIR/eiswolfsiboard.out.log / eiswolfsiboard.err.log"
echo "Status pruefen: launchctl list | grep eiswolfsiboard"
