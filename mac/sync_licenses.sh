#!/bin/bash
# Lizenzdateien fuers DMG aus der EINZIGEN Quelle im Hauptordner uebernehmen
# (LICENSE, THIRD-PARTY-LICENSES.md, licenses/) - kein doppelter Pflegeaufwand.
# Vor jedem DMG-Bau ausfuehren, danach mac/dmg_staging/ nach mac/dmg_build/ kopieren.
set -euo pipefail
cd "$(dirname "$0")"
cp ../LICENSE dmg_staging/LICENSE
cp ../THIRD-PARTY-LICENSES.md dmg_staging/THIRD-PARTY-LICENSES.md
rm -rf dmg_staging/licenses
cp -R ../licenses dmg_staging/licenses
echo "Lizenzdateien nach mac/dmg_staging/ synchronisiert."
