# Development / Entwicklung

*English below / Deutsch weiter unten.*

## English

This is the public build/contribution reference. It doesn't cover
day-to-day project history or internal notes - just what you need to
build and run each part from source.

### Layout

- `cyd/` - ESP32 firmware (PlatformIO, C++). Targets the classic CYD
  (ESP32-2432S028, resistive touch). Supports up to 4 Wake-on-LAN
  devices (`Storage::WolDevice[]`), managed on-device under Setup →
  "WoL devices"; each gets a tile/button via the shared
  `handleDeviceWidgets()` on Controls, the "Devices" page and the "No
  connection to Mac" screen.
- `mac/` - macOS menu-bar app (Python, `rumps` + PyInstaller). Pauses
  sending entirely during system sleep and display sleep/DarkWake
  (`wake_watch.py`), which is what lets the CYD turn its display off
  while the Mac sleeps and back on once it's genuinely awake again.
- `windows/` - optional Windows background listener (Go), lets the CYD
  shut a Windows laptop down. Fully modular (no personal data in code
  or installer) - the NSIS setup wizard collects device name/port/
  token/autostart and writes them to `%APPDATA%\EiswolfsIBoard\
  config.json`. See [windows/README.md](../windows/README.md) and
  [docs/WOL_SETUP.md](WOL_SETUP.md) for adding a new device.
- `tools/fan-helper/` - optional root helper for fan-speed control. See
  [tools/fan-helper/README.md](../tools/fan-helper/README.md).
- `docs/` - this file, [TOKEN.md](TOKEN.md) (the shared-secret system),
  and the photo gallery.

### Building

```
# CYD firmware
cd cyd && pio run                    # build
pio run --target upload              # flash

# Mac app
cd mac
python3 -m venv venv && ./venv/bin/pip install -r requirements.txt
./venv/bin/pyinstaller eiswolfs_iboard.spec --noconfirm
codesign --force --deep --sign - "dist/Eiswolfs iBoard.app"   # ad-hoc

# Windows listener (cross-built from macOS/Linux, no Windows needed to build)
cd windows/listener
GOOS=windows GOARCH=amd64 go build -trimpath -ldflags "-H=windowsgui -s -w" \
    -o ../installer/EiswolfsIBoardListener.exe .
cd ../installer && makensis installer.nsi
```

`-trimpath` (Go) strips local build-machine file paths from the binary
- always use it when building a binary you intend to share.

### The shared token

Fan boost, sleep, and the Windows shutdown listener need a shared
secret token, identical across the three components. It's never
checked into the repo. See [TOKEN.md](TOKEN.md) for what it is and how
to set your own.

### Contributing

Pull requests welcome. Please don't commit anything under
`cyd/src/secrets.h`, `*/config.json`, `local/`, or anything else that
would end up in `.gitignore` for a reason - check the comments there
if you're unsure why a path is excluded.

---

## Deutsch

Das ist die öffentliche Bau-/Mitwirkungs-Referenz. Keine laufende
Projekt-Historie oder interne Notizen - nur das, was zum Bauen und
Ausführen der einzelnen Teile aus dem Quellcode nötig ist.

### Aufbau

- `cyd/` - ESP32-Firmware (PlatformIO, C++). Für das klassische CYD
  (ESP32-2432S028, resistiver Touch). Unterstützt bis zu 4 Wake-on-LAN-
  Geräte (`Storage::WolDevice[]`), verwaltet direkt am Gerät unter
  Einrichtung → "WoL-Geräte"; jedes bekommt eine Kachel/einen Button
  über das gemeinsame `handleDeviceWidgets()` auf Controls, der Seite
  "Geräte" und "Keine Verbindung zum Mac".
- `mac/` - macOS-Menüleisten-App (Python, `rumps` + PyInstaller). Pausiert
  den Versand komplett bei Systemschlaf und bei Display-Schlaf/DarkWake
  (`wake_watch.py`) - genau das lässt das CYD-Display ausgehen, während
  der Mac schläft, und erst wieder angehen, wenn er wirklich wach ist.
- `windows/` - optionaler Windows-Hintergrund-Listener (Go), lässt das
  CYD einen Windows-Laptop herunterfahren. Vollständig modular (keine
  persönlichen Werte im Code/Installer) - der NSIS-Einrichtungsdialog
  fragt Gerätename/Port/Token/Autostart ab und schreibt sie nach
  `%APPDATA%\EiswolfsIBoard\config.json`. Siehe
  [windows/README.md](../windows/README.md) und
  [docs/WOL_SETUP.md](WOL_SETUP.md) zum Einrichten eines neuen Geräts.
- `tools/fan-helper/` - optionaler Root-Hilfsdienst für die
  Lüftersteuerung. Siehe
  [tools/fan-helper/README.md](../tools/fan-helper/README.md).
- `docs/` - diese Datei, [TOKEN.md](TOKEN.md) (das Token-System) und
  die Foto-Galerie.

### Bauen

```
# CYD-Firmware
cd cyd && pio run                    # bauen
pio run --target upload              # flashen

# Mac-App
cd mac
python3 -m venv venv && ./venv/bin/pip install -r requirements.txt
./venv/bin/pyinstaller eiswolfs_iboard.spec --noconfirm
codesign --force --deep --sign - "dist/Eiswolfs iBoard.app"   # ad-hoc

# Windows-Listener (von macOS/Linux aus cross-kompiliert, kein Windows zum Bauen nötig)
cd windows/listener
GOOS=windows GOARCH=amd64 go build -trimpath -ldflags "-H=windowsgui -s -w" \
    -o ../installer/EiswolfsIBoardListener.exe .
cd ../installer && makensis installer.nsi
```

`-trimpath` (Go) entfernt lokale Build-Rechner-Pfade aus der
Binärdatei - immer verwenden, wenn die gebaute Datei weitergegeben
werden soll.

### Der gemeinsame Token

Lüfter-Boost, Ruhemodus und der Windows-Shutdown-Listener brauchen
einen gemeinsamen, geheimen Token - identisch in allen drei
Komponenten. Steht nie im Repo. Siehe [TOKEN.md](TOKEN.md), was das
ist und wie man seinen eigenen setzt.

### Mitwirken

Pull Requests willkommen. Bitte nichts unter `cyd/src/secrets.h`,
`*/config.json`, `local/` oder sonstigen Pfaden committen, die aus
gutem Grund in `.gitignore` stehen - bei Unklarheit die Kommentare
dort nachlesen.
