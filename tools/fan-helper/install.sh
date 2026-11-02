#!/bin/bash
# Installiert den Eiswolfs-iBoard-Luefter-Hilfsdienst (fand.py) als
# LaunchDaemon (Root) - Alex fuehrt dieses Skript SELBST mit sudo aus,
# siehe CLAUDE.md Abschnitt "Luefter-Boost". Es fragt nirgends nach
# einem Passwort (das macht sudo selbst) und speichert keines.
set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
    echo "Bitte mit sudo ausfuehren:"
    echo "  sudo bash \"$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/install.sh\""
    exit 1
fi

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LABEL="com.alexdemurtas.eiswolfsiboard.fanhelper"
PLIST_DEST="/Library/LaunchDaemons/${LABEL}.plist"
TARGET_USER="${SUDO_USER:-}"

if [ -z "$TARGET_USER" ]; then
    echo "Fehler: konnte den aufrufenden Benutzer nicht ermitteln (SUDO_USER leer)."
    echo "Bitte ueber 'sudo' ausfuehren, nicht als direkt eingeloggter root."
    exit 1
fi

mkdir -p /Library/Logs

sed \
    -e "s#__FAND_PATH__#$DIR/fand.py#g" \
    -e "s#__USER__#$TARGET_USER#g" \
    -e "s#__LOG_OUT__#/Library/Logs/eiswolfsiboard-fanhelper.log#g" \
    -e "s#__LOG_ERR__#/Library/Logs/eiswolfsiboard-fanhelper.err.log#g" \
    "$DIR/${LABEL}.plist" > "$PLIST_DEST"

chown root:wheel "$PLIST_DEST"
chmod 644 "$PLIST_DEST"

launchctl bootout system "$PLIST_DEST" 2>/dev/null || true
launchctl bootstrap system "$PLIST_DEST"
launchctl enable "system/${LABEL}"

echo "Luefter-Hilfsdienst installiert und gestartet (Benutzer: $TARGET_USER)."
echo "Logs: /Library/Logs/eiswolfsiboard-fanhelper.log / .err.log"
echo "Status pruefen: sudo launchctl print system/${LABEL}"
