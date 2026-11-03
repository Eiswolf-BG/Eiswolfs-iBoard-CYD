# Eiswolfs iBoard - Projektkontext für Claude Code

Lies diese Datei ZUERST, bevor du irgendwas an diesem Projekt änderst.

## Auftrag DMG v2: Fortschritt (02.11., laufend)

Bei Unterbrechung: hier zuerst weiterlesen, nichts bereits Abgehaktes
wiederholen. Ziel: installierbare App v1.1 (Menüleisten-Auswahl +
Lüfter-Boost-Client, OHNE Hilfsdienst/Token), neue DMG, Alex' Mac
aktualisieren, optional Firmware für den Bekannten (ohne Alex' Token).

- [x] Schritt 1: Code korrekt, Textsuche nach dem Build (Bundle UND
      DMG) auf Token/Pfade/config.json: 0 Treffer
- [x] Schritt 2: App v1.1 gebaut, ad-hoc signiert (valid on disk),
      in bereinigter Umgebung (env -i, leerer PATH) gestartet, lief
      stabil, config.json unverändert (alle Felder inkl. Token
      erhalten)
- [x] Schritt 3: Neue DMG gebaut (`dist/Eiswolfs-iBoard.dmg`), alte
      v1.0-DMG als `Eiswolfs-iBoard-v1.0.dmg` archiviert,
      INSTALLATION.md um Menüleisten-Auswahl, Lüfter-Boost-Hinweis
      und Update-Anleitung ergänzt
- [x] Schritt 4: Alex' Mac aktualisiert (alte App ersetzt), Tests a-h
      alle bestätigt (inkl. eines echten, ungeplanten Hilfsdienst-
      Ausfalls während des Tests, der Test f) organisch mitgeliefert
      hat)
- [x] Schritt 5 (optional): Firmware für den Bekannten mit Beispiel-
      Token gebaut (`dist/Eiswolfs-iBoard-CYD-merged.bin`), NICHT
      geflasht, Alex' echtes Token nirgends in der Datei, Alex' echte
      secrets.h sofort wiederhergestellt und neu gebaut
- [x] CLAUDE.md-Abschnitt zu v1.1 ergänzt

**Auftrag abgeschlossen (02.11.).**

## Auftrag Lüfter-Boost: Fortschritt (02.11., laufend)

Bei Unterbrechung: hier zuerst weiterlesen, nichts bereits Abgehaktes
wiederholen. Reihenfolge laut Auftrag: Teil A (CYD-Labels) -> Teil B
(Menüleisten-Wert) -> Teil C0 (Hochdrehzahl-Test) -> Teil C1
(Hilfsdienst) -> Teil C2 (Mac-App Boost) -> Teil C3 (CYD-Firmware
Boost) -> Teil D (Bauen/Flashen/Testen).

- [x] Teil A: CYD-Labels ("TEMP", "FAN / RPM" im Streifen, Balken auf
      CPU/GPU/RAM-Höhe) - fertig, `pio run` erfolgreich
- [x] Teil B: Mac-App, wählbarer Menüleisten-Wert - fertig, Untermenü
      "In Menüleiste anzeigen", gespeichert als "menubar_item" in
      config.json, geprüft (Datei verliert keine anderen Felder)
- [x] Teil C0: Test Phase 2 (~3000 RPM) + Phase 3 (Maximaldrehzahl) -
      ERFOLGREICH (Alex hat es gehört, Temperaturen blieben niedrig,
      Automatik kam danach zuverlaessig zurueck) -> Teil C wird gebaut.
      ERWEITERUNG (Alex' Wunsch danach): Boost braucht eine waehlbare
      Zieldrehzahl, nicht nur "Vollgas". Vier feste Stufen: 2000, 3000,
      4000 RPM und Vollgas (Maximaldrehzahl laut SMC) - unabhaengig
      sowohl am CYD-Menue als auch im Mac-App-Menue waehlbar (wie bei
      der Boost-Dauer).
- [x] Teil C1: Hilfsdienst (`tools/fan-helper/fand.py` + `smc_core.py`,
      LaunchDaemon, Unix-Socket `/var/run/eiswolfs-iboard-fan.sock`
      nur fuer Alex' Benutzer, Wachhund 10s, 4h-Obergrenze) - Logik
      mit gefaketem SMC getestet (Befehle + Wachhund-Timeout),
      install.sh/uninstall.sh geschrieben, ECHTE Installation steht
      noch aus (Alex fuehrt sudo install.sh selbst aus, siehe Teil D)
- [x] Teil C2: Mac-App Boost-Modul (`mac/fan_boost.py`: Socket-Client
      FanBoostClient, CydCommandListener auf UDP 4211 + USB-Zeile,
      Token-Pruefung, Menü "Lüfter-Boost" mit Dauer- UND Drehzahl-
      Untermenue, Schlaf-Erkennung via NSWorkspace, eigener
      "Beenden"-Handler) - Logik gegen gefakten Hilfsdienst getestet
      (Start/Ablauf/Stop/Token/Rate-Limit)
- [x] Teil C3: CYD-Firmware Boost (langes Antippen FAN-Kachel 0,5s,
      orange "BOOST"+Restzeit auf dem Streifen, "n/v" bei fehlender
      Bestaetigung, neue Menü-Seite "Lüfter" mit Dauer+Drehzahl,
      "Lüfter"/"Einrichtung" teilen sich jetzt eine Reihe) - `pio run`
      erfolgreich
- [x] Teil D: Gebaut, geflasht, auf Alex' Mac getestet (Labels,
      Menüleisten-Wert, Hilfsdienst installiert, Boost per Mac-Menü,
      Boost per langem Antippen inkl. Stop/"Bis Tippen"/kurzes
      Antippen tut nichts) - EIN Bug gefunden und behoben (siehe
      "Stolperfallen" unten: Touch-Entprellung), danach alles
      bestätigt funktionierend. Am Ende: Entwicklerkopie beendet,
      installierte App (/Programme, alter Stand ohne Boost) läuft
      wieder, Lüfter auf Automatik.
- [x] CLAUDE.md-Abschnitt zum fertigen Boost-Aufbau ergänzt (siehe
      "Lüfter-Boost" weiter unten)

**Auftrag abgeschlossen (02.11.).**

## Auftrag installierbare App: Fortschritt (29.10., laufend)

Bei Unterbrechung: hier zuerst weiterlesen, nichts bereits Abgehaktes
wiederholen. Reihenfolge laut Auftrag: Schritt 1 (Bundle bauen) ->
Schritt 2 (Einstellungen/Autostart/Robustheit) -> Schritt 3 (Signieren/
DMG/Anleitung) -> Schritt 4 (Testen auf Alex' Mac).

- [x] Schritt 1: App-Bundle bauen (PyInstaller, arm64, LSUIElement,
      macmon eingebettet, NSLocalNetworkUsageDescription DE) - fertig,
      `mac/eiswolfs_iboard.spec`
- [x] Schritt 2: Einstellungen in Application Support, Menüpunkt "Beim
      Login starten" (eigener LaunchAgent statt SMAppService, siehe
      unten), Robustheit (Intel-Mac-Erkennung, fehlende Sensoren schon
      vorher robust) - fertig
- [x] Schritt 3: Ad-hoc-Signatur (verifiziert), DMG mit
      INSTALLATION.md + THIRD-PARTY-LICENSES, liegt in
      `dist/Eiswolfs-iBoard.dmg` - fertig
- [x] Schritt 4: Getestet auf Alex' Mac (Entwickler-LaunchAgent
      entladen, aus der DMG nach /Applications installiert, bereinigte
      Umgebung, Autostart-Umschaltung, Single-Instance-Schutz bei
      LaunchAgent-Race) - fertig, Autostart der installierten App ist
      AKTIV, Entwickler-Autostart ENTFERNT
- [x] CLAUDE.md-Abschnitt zum fertigen Bau-/Signier-/Autostart-Ablauf
      ergänzt (siehe "Installierbare App (DMG)" weiter unten)

**Auftrag abgeschlossen (02.11.).**

## Installierbare App (DMG)

Neben der Entwicklungsumgebung (`mac/venv`, gestartet über
`Eiswolfs iBoard starten.command`) existiert eine fertig gebaute,
eigenstaendig installierbare Version fuer andere Macs (z.B. bei einem
Bekannten). Beide Varianten teilen sich bewusst denselben
Application-Support-Ordner (`config.json`, Logs, Single-Instance-Lock),
damit sie sich nie gegenseitig ueberschreiben und nie gleichzeitig
senden koennen.

### Bauen

```
cd "/Users/alexdemurtas/Eiswolfs-iBoard/mac"
./venv/bin/pyinstaller eiswolfs_iboard.spec --noconfirm
```

Ergebnis liegt in `mac/dist/Eiswolfs iBoard.app` (arm64-only,
LSUIElement, kein Dock-Symbol). Danach ad-hoc signieren und pruefen:

```
codesign --force --deep --sign - "mac/dist/Eiswolfs iBoard.app"
codesign --verify --deep --strict --verbose=2 "mac/dist/Eiswolfs iBoard.app"
```

DMG bauen: `.app` + `Programme`-Symlink (`ln -s /Applications Programme`)
+ `INSTALLATION.md` + `THIRD-PARTY-LICENSES.md` + `macmon_LICENSE.txt`
in einen Staging-Ordner (`mac/dmg_build/`) legen, dann:

```
hdiutil create -volname "Eiswolfs iBoard" -srcfolder mac/dmg_build \
  -ov -format UDZO "dist/Eiswolfs-iBoard.dmg"
```

Fertige Datei: `dist/Eiswolfs-iBoard.dmg` (~11 MB). Die
Installationsanleitung und die Lizenzliste liegen als Quelle in
`mac/dmg_staging/` und werden beim DMG-Bau nach `mac/dmg_build/`
kopiert.

### Eingebettetes macmon

`mac/bundle_resources/macmon` ist eine Kopie des arm64-Binaries aus
`/opt/homebrew/Cellar/macmon/<version>/bin/macmon` (MIT-Lizenz, Text in
`mac/bundle_resources/macmon_LICENSE.txt` bzw. im DMG als
`THIRD-PARTY-LICENSES.md`/`macmon_LICENSE.txt`). Die App ruft NIE ein
systemweit installiertes macmon ueber PATH auf, sondern immer die
eingebettete Kopie via `resource_path("macmon")` in `mac/sender.py`.

**Wichtige PyInstaller-Falle:** `binaries=[("bundle_resources/macmon", ".")]`
im Spec legt die Datei NICHT neben die ausfuehrbare Datei in
`Contents/MacOS/`, sondern physisch in `Contents/Frameworks/`, mit
einem Symlink `Contents/Resources/macmon -> ../Frameworks/macmon`. Der
richtige Pfad im gebuendelten Betrieb ist deshalb
`Contents/Resources/<name>` (siehe `resource_path()` in `sender.py`),
NICHT `os.path.dirname(sys.executable)`.

### Einstellungen

`~/Library/Application Support/Eiswolfs iBoard/config.json` -wird beim
ersten Start mit Standardwerten angelegt, danach NIE automatisch
ueberschrieben (`mac/config.py`). Gleicher Ordner enthaelt `logs/` und
den Single-Instance-Lock `sender.lock` (`fcntl.flock`).

### Autostart ("Beim Login starten")

Eigenes Modul `mac/login_item.py`, nur aktiv wenn die App gebuendelt
laeuft (`sys.frozen`). Schreibt einen eigenen LaunchAgent-Plist nach
`~/Library/LaunchAgents/com.alexdemurtas.eiswolfsiboard.app.plist` -
bewusst ein ANDERES Label als der Entwickler-LaunchAgent
(`com.alexdemurtas.eiswolfsiboard`, siehe `mac/install.sh`/
`mac/uninstall.sh`), damit beide nie verwechselt werden. `RunAtLoad`
ohne `KeepAlive`, damit "Beenden" im Menue nie automatisch neu
gestartet wird. `ProgramArguments` zeigt auf `sys.executable`, folgt
also automatisch dem tatsaechlichen Installationsort. Kein
SMAppService (brauchte ein zusaetzliches PyObjC-Framework,
hoeheres Kompatibilitaetsrisiko).

### Gatekeeper / Signatur

Die App ist nur ad-hoc signiert (kein zahlungspflichtiges Apple-
Entwicklerkonto vorhanden) - `codesign --verify` ist erfolgreich
("valid on disk"), aber `spctl --assess --type execute` meldet
erwartungsgemaess "rejected", da nicht notarisiert. Das ist normal,
keine Fehlfunktion. Auf einem fremden Mac erscheint deshalb beim
ERSTEN Start eine Gatekeeper-Warnung - Loesung: Rechtsklick auf die App
-> "Oeffnen", oder Systemeinstellungen -> Datenschutz & Sicherheit ->
"Trotzdem oeffnen" (Details in `INSTALLATION.md` im DMG).

### Was NICHT enthalten ist

Die installierbare App enthaelt KEINE Luefter-STEUERUNG (nur reines
Auslesen der Luefterdrehzahl zur Anzeige) - das fruehere
Luefter-Test-Feature in `tools/fan-test/` ist bewusst NICHT Teil
dieses Bundles.

### Fallstricke

- Niemals `mac/venv` oder einen Homebrew-Pfad im Bundle voraussetzen -
  alles muss aus `Contents/` der `.app` kommen (mit sanitisierter
  Umgebung, z.B. `env -i`, testbar).
- Bei jeder Spec-/Code-Aenderung, die Binaries/Daten betrifft, nach dem
  Bauen IMMER pruefen, wo PyInstaller die Datei tatsaechlich ablegt
  (`find "Eiswolfs iBoard.app" -name <name>`), nicht nur vom erwarteten
  Pfad ausgehen.
- Der Entwickler-LaunchAgent und der App-LaunchAgent duerfen nie
  gleichzeitig aktiv sein (sonst senden zwei Instanzen parallel) - der
  Single-Instance-Lock faengt das zwar ab, aber im Normalbetrieb sollte
  nur eine der beiden Varianten laufen.

## Umbenennung (29.10.): "iBoard" -> "Eiswolfs iBoard"

Projektordner heißt jetzt `/Users/alexdemurtas/Eiswolfs-iBoard` (vorher
`/Users/alexdemurtas/iBoard`) - Bindestrich statt Leerzeichen, damit
Pfade in Skripten/PlatformIO/LaunchAgent sicher funktionieren. Anzeige-
name überall "Eiswolfs iBoard" (mit Leerzeichen). Beim Umbenennen eines
Ordners IMMER daran denken:
- Die Python-`venv` im Ordner `mac/` funktioniert nach einem Umzug
  NICHT mehr (enthält absolute Pfade) - neu anlegen
  (`python3 -m venv venv && ./venv/bin/pip install -r requirements.txt`).
- Der PlatformIO-Build-Ordner `cyd/.pio/` sollte komplett gelöscht und
  neu gebaut werden.
- Jeden Treffer für den alten Namen/Pfad im ganzen Projekt durchgehen
  (`grep -rln "iBoard"`), nicht blind ersetzen - manche Treffer sind
  bewusst unveränderte technische Kennungen (siehe unten).

**Bewusst NICHT umbenannt** (technische Kennungen, nicht der
Anzeigename):
- Der Ordner `/iBoard` auf der SD-Karte mit `config.json` - bleibt so,
  damit bestehende Geräte ohne neue Einrichtung weiterlaufen.
- Die Protokollzeilen `"IBOARD?"` / `"IBOARD_OK"` im USB-Handshake,
  Port-Nummer (4210) und das JSON-Paketformat.
- Das Dateiformat der `config.json` selbst.

Mac-App: `rumps.App`-Name jetzt `"Eiswolfs iBoard"` (steuert u.a. den
Application-Support-Ordner - der alte, leere `~/Library/Application
Support/iBoard`-Ordner wurde gelöscht, rumps legt den neuen
`.../Eiswolfs iBoard` automatisch beim ersten Start an). LaunchAgent-
Label/Plist-Datei jetzt `com.alexdemurtas.eiswolfsiboard`
(`mac/com.alexdemurtas.eiswolfsiboard.plist`, vorher `...iboard.plist`).
War zum Zeitpunkt der Umbenennung kein LaunchAgent aktiv - falls doch,
immer zuerst mit `uninstall.sh` entfernen, dann umbenennen, dann mit
`install.sh` neu einrichten.

CYD: WLAN-Hostname jetzt `"Eiswolfs-iBoard"` (`WiFi.setHostname(...)`,
gesetzt in `connectWifi()` und `setup_flow.cpp::testConnection()`, VOR
`WiFi.begin()`). Anzeigename "Eiswolfs iBoard" erscheint: Kopfzeile
links (zweizeilig, siehe "Kopfzeilen-Layout" unten), Hauptmenü-Titel
("Eiswolfs iBoard Menue"), beim WLAN-Verbindungsaufbau, auf dem "Keine
Verbindung zum Mac"-Bildschirm, und in der Ersteinrichtung
(Kalibrierungs- und WLAN-Auswahl-Bildschirm).

Spätere installierbare Mac-App heißt `Eiswolfs iBoard.app`, die DMG
`Eiswolfs-iBoard.dmg`.

## Projektziel

Ein Mac mini (M4, Apple Silicon) sendet seine Systemwerte (CPU/GPU-Last,
Temperaturen, RAM, Leistungsaufnahme, Lüfterdrehzahl) per WLAN an ein
klassisches CYD (ESP32-2432S028), das als kleines Standgerät am
Tischrand die Werte live anzeigt. Das CYD braucht nur Strom (USB) zum
Betrieb, zum Flashen hängt es per USB am Mac.

## Architektur

- **Mac-App** (`mac/`): Python-Menüleisten-App (rumps), liest einmal pro
  Sekunde CPU/RAM/Disk (psutil), CPU-/GPU-Temperatur + GPU-Last +
  Leistungsaufnahme + Lüfterdrehzahl (alles über das Kommandozeilen-Tool
  `macmon`, JSON-Ausgabe, kein sudo nötig - liefert auch die Lüfter-
  Drehzahl mit, eine separate SMC-Abfrage war nicht nötig). Alle Werte
  werden als eine kompakte JSON-Zeile **gleichzeitig auf zwei Wegen**
  verschickt: per UDP-Broadcast ins lokale Netz auf Port 4210 (keine
  feste Ziel-IP nötig) UND, falls ein iBoard per USB-Kabel gefunden
  wurde, zusätzlich über die serielle USB-Verbindung (siehe
  `usb_link.py`). Welcher Weg am Gerät tatsächlich verwendet wird,
  entscheidet das CYD selbst (siehe "Verbindungsmodi" unten) - die
  Mac-App schickt immer beides, sofern verfügbar.
- **CYD-Firmware** (`cyd/`): PlatformIO/Arduino-Projekt für das
  klassische CYD. Verbindet sich ins WLAN, lauscht per UDP auf Port
  4210 UND auf dem USB-Serial-Eingang, zeigt die empfangenen Werte in
  Echtzeit an (flackerfreies Teil-Redraw, nur geänderte Bereiche).
  Zeigt "Keine Verbindung zum Mac", wenn länger als 6s kein gültiges
  Paket auf dem jeweils aktiven Weg ankommt, und springt automatisch
  zurück, sobald wieder Daten kommen. Verbindet sich bei WLAN-Abbruch
  selbst neu.

## Ordnerstruktur

```
iBoard/
  CLAUDE.md          - diese Datei
  .gitignore         - schliesst Logs und Build-Artefakte aus
  mac/               - Python-Menüleisten-App
    venv/            - virtuelle Umgebung (nicht in Git)
    sender.py        - Haupt-App, SenderThread (fester 1s-Takt, UDP+USB)
    usb_link.py       - USB-Serial-Erkennung/-Verbindung (Hallo-Protokoll)
    config.py         - optionale feste Ziel-IP, Debug-Log-Schalter
    install.sh       - richtet LaunchAgent ein (Autostart bei Login)
    uninstall.sh     - entfernt den LaunchAgent wieder
  cyd/               - PlatformIO-Projekt fürs CYD
    platformio.ini
    fonts/
      LICENSES.md              - Lizenzen der beiden Anzeige-Schriften
      freesansbold_bsd_license.txt
    src/
      main.cpp                 - Anzeige, Menü, Netzwerk/USB, Modi
      touch_calib.h/.cpp       - Touch-Rohdaten + affine Kalibrierung
      storage.h/.cpp           - config.json (SD) / NVS-Fallback
      setup_flow.h/.cpp        - Ersteinrichtungs-Assistent
      config.h                 - Pins, Timeouts, Debug-Flag
      fonts/
        PixelFont20.h           - "Pixel"-Schrift (Press Start 2P, OFL)
        PressStart2P_OFL.txt
```

## WICHTIGSTE SCHUTZREGEL

Die Projekte **Flightradar** (`/Users/alexdemurtas/CYD Flightradar`),
**Toolbox/Satellitenradar** (`/Users/alexdemurtas/eiswolfs-toolbox`) und
**Solar Guardian** (`/Users/alexdemurtas/solar-guardian-cyd`) dürfen auf
keinen Fall verändert werden - in diesen Ordnern darf höchstens
GELESEN werden (z.B. um Display-/Touch-Konfiguration abzuschauen),
niemals geschrieben, gelöscht oder etwas hineinkopiert. Alles Neue für
dieses Projekt gehört ausschließlich in den Ordner `iBoard/`.

## Arbeitsregel: nur bauen und flashen, nie von selbst committen

Dieses Projekt hat (Stand jetzt) noch kein Git-Repository. Falls später
eines angelegt wird: niemals von selbst committen, taggen oder pushen -
nur wenn Alex ausdrücklich danach fragt. Normales Arbeiten (Code ändern,
`pio run`, flashen) passiert ohne Rückfrage, Versionskontroll-Aktionen
nur auf expliziten Wunsch.

## Kopfzeilen-Layout (Auftrag "Watt-Kachel", 29.10.)

Umschaltbar über `Config::USE_OLD_HEADER` in `config.h` (`false` =
Normalbetrieb/neues Layout, `true` = altes 20px-Layout) - bei
Nichtgefallen einfach auf `true` setzen und neu bauen/flashen, beide
Code-Pfade bleiben vollständig im Code erhalten
(`drawHeaderRightOld()` vs. `drawHeaderSourceNew()`/
`drawHeaderPwrValueNew()`, Dispatcher: `drawHeaderDynamic()`).

**Neues Layout** (Standard, Stand 29.10. nach "Kopfzeile überarbeiten"):
Kopfzeile 34px hoch (`HEADER_H`), drei Bereiche:
- **Links** (ein Drittel, `HEADER_LEFT_W`≈106px, Tippfläche bis 40px
  hoch reichend fürs Hauptmenü): "Eiswolfs" / "iBoard" zweizeilig,
  GROSS und in der **klaren Schrift** (`pickClearFont()`, ~16px,
  eisblau `HEADER_NAME_COLOR`=`0x7EBF`≈#7FD6FF) - IMMER klar, auch
  wenn im Menü "Pixel" gewählt ist, statisch gezeichnet in
  `drawStaticLayout()`. Breite wird vor dem Zeichnen geprüft, damit
  nichts den PWR-Streifen berührt.
- **Rechts oben in der Ecke** (`headerSourceRect`, 76x13px): das
  Quellenkürzel ("USB"/"WLAN", mit "*" bei fest gewählter Quelle) -
  seit diesem Umbau NICHT mehr unter dem Namen links, sondern
  rechtsbündig am oberen Rand, ebenfalls klare Schrift
  (`drawHeaderSourceNew()`), einziger dynamischer Teil dort, nur bei
  Änderung neu gezeichnet.
- **Rechts, Watt-Kachel** (`pwrTileRect`): unverändert im selben Stil
  wie die sechs Hauptkacheln - gelber Streifen links (`PWR_STRIPE_W`=
  58px, Label "PWR" schwarz, einmalig statisch gezeichnet) und
  Wertbereich rechts daneben (Wattzahl gelb, z.B. "18.9 W", fehlt der
  Wert: "--"). Eigene, einmalig berechnete Schriftgröße `pwrFont`
  (siehe `computeRowFonts()`, Kandidatenwerte "100.0 W"/"99.9 W"/"--"),
  unabhängig von `row1Font`/`row2Font` der Hauptkacheln. **Bewusst
  unverändert gelassen** (Höhe/Fonthöhen-Berechnung), damit die
  Wattzahl beim Verschieben der Quellenanzeige nicht kleiner wird -
  das Quellenkürzel oben rechts überlappt die Watt-Kachel nur
  räumlich (gleiche x-Spalte), nicht die Schriftgrößen-Berechnung.
  **Noch nicht am Gerät visuell bestätigt**: ob "WLAN*"/"USB*" oben
  rechts und eine grosse Wattzahl wie "99.9 W" sich bei sehr breiten
  Werten berühren - bitte am Gerät gegenprüfen (siehe Auftrag,
  Testpunkt j).

Die Kopfzeilen-Vergrößerung (20px -> 34px) geht von der Höhe der
Hauptkacheln ab (`TILE_H`≈106px -> 99px) - Labels/Wertzahlen blieben
dabei in der Praxis gleich groß (gleiche diskrete Schriftstufe wie
vorher), nur der Balken-/Leerraum wurde etwas knapper.

## Anzeige-Layout (6 Kacheln mit Farbstreifen, siehe `main.cpp`)

Darunter 2 Reihen x 3 Kacheln (`tileRect()`,
`TILE_W`≈103px, `TILE_H`≈99px, 3px Lücke, 2px Außenrand): Reihe 1 =
CPU | GPU | RAM (Last), Reihe 2 = TMP | TMP | FAN (CPU-Temp, GPU-Temp,
Lüfter). Jede Kachel hat oben einen **farbigen Streifen** (29px, Label
in schwarzer Schrift) statt eines dünnen Rahmens - die Kacheln sind
allein durch Streifenfarbe + kleinen Abstand erkennbar. Streifenfarben:
CPU/erste TMP-Kachel = Cyan, GPU/zweite TMP-Kachel = Lila, RAM =
Orange, FAN = Grün (die beiden TMP-Kacheln übernehmen bewusst die Farbe
der Last-Kachel direkt darüber, damit klar ist, welche Temperatur zu
welcher Last gehört). Die Zahl selbst bleibt bei TMP in ihrer eigenen
Warnfarbe (grün/gelb/rot, siehe `colorForTemp()`). Darunter der
Wertbereich (`drawTileValue()`) - bei jeder Änderung wird NUR dieser
Bereich gelöscht und neu gezeichnet, nie die ganze Kachel/der Streifen.

**Gegen Textüberlauf UND gegen das "Aufblitzen" (behoben, siehe unten)**:
Die Schriftgröße wird NICHT mehr pro Wert neu gemessen, sondern einmalig
pro Zeile berechnet (`computeRowFonts()`, bei jedem Layoutaufbau und bei
jedem Schriftwechsel neu aufgerufen) - anhand der längsten jemals
möglichen Werte ("100%"/"--" für Reihe 1, "110C"/"4900"/"AUS"/"--" für
Reihe 2). `drawTileValue()` wendet nur noch diese fixe `row1Font`/
`row2Font`-Größe an. Einheiten stehen direkt an der Zahl ("20%", "61C"),
beim Lüfter steht die Zahl allein gross, "RPM" klein darunter (nur wenn
Platz ist), bei 0 RPM steht "AUS". Balken (CPU/GPU/RAM-Last, Lüfter)
liegen in einem eigenen, festen Streifen am unteren Rand der Kachel.

## Schriften "Pixel" und "Klar" (siehe `cyd/fonts/LICENSES.md`)

Zwei umschaltbare Anzeige-Schriften, beide frei nutzbar:
- **Pixel** (Standard): Press Start 2P (OFL-Lizenz), per eigenem
  Python-Skript (nicht Teil des Firmware-Builds) aus der TTF-Datei ins
  TFT_eSPI-Format (GFXFF) umgewandelt, Basisgröße 20px
  (`cyd/src/fonts/PixelFont20.h`), per `tft.setTextSize()` hoch-
  skaliert.
- **Klar**: FreeSansBold, bereits Teil der TFT_eSPI-Bibliothek (BSD-
  Lizenz, lib_deps holt sie automatisch), Größen 9pt/18pt/24pt.
- Beide Schriften können **kein Grad-Zeichen** darstellen (beide nur
  ASCII 0x20-0x7E) - Temperaturen zeigen deshalb immer "61C" statt
  "61°", auch bei "Klar".
- Umschalten über das Hauptmenü (siehe unten), Feld `font` in
  `config.json` ("pixel"/"clear"). **Standard seit 29.10.: "clear"**
  für neue Geräte (kein `config.json`/NVS vorhanden - Default in
  `Storage::DeviceConfig` in `storage.h`). Eine BESTEHENDE
  Konfiguration mit gespeichertem `font`-Feld wird beim Laden immer
  von dort überschrieben (siehe `storage.cpp`) und bleibt dadurch
  unverändert, auch wenn sie "pixel" sagt - der neue Default betrifft
  nur echte Erstinbetriebnahmen.
- Bootscreen-Texte (Name, WLAN-Status, Version) und seit "Kopfzeile
  überarbeiten" auch der Kopfzeilen-Name/die Quellenanzeige sind
  IMMER in der klaren Schrift, unabhängig von der im Menü gewählten
  Familie (`pickClearFont()` in `main.cpp`, erzwingt `CLEAR_CANDIDATES`
  statt `currentFamily`).

## Bootscreen (Auftrag "Bootscreen mit Apfel", 29.10.)

Erscheint 7s (`Config::BOOT_SCREEN_MS`) lang NUR beim echten Boot/Reset
mit gültiger `config.json` (`runBootScreen()` in `main.cpp`) - NICHT
bei WLAN-Wechseln, Verbindungsabbrüchen oder beim Schließen des Menüs.
Fehlt noch eine gültige Konfiguration (WLAN+Kalibrierung), startet
stattdessen direkt die Ersteinrichtung, ganz ohne Bootscreen.

**Dashboard-Stil (Auftrag "Bootscreen im Dashboard-Stil", 02.11.)**:
der Bootscreen wirkte vorher deutlich anders als das Dashboard (Apfel
mintgrün/verwaschen durch Verlauf+Glanzfleck+Dithering, Titel/Balken in
einem eigenen Eisblau `#7FD6FF`, das im Dashboard gar nicht vorkommt).
Jetzt verwendet er AUSSCHLIESSLICH Dashboard-Farben, keine eigenen
Werte mehr:

| Farbe | Konstante | Verwendung im Dashboard |
|---|---|---|
| Grün | `TFT_GREEN` | FAN-Streifen (das EINE Grün) |
| Gelb | `TFT_YELLOW` | PWR-Streifen |
| Lila | `TFT_PURPLE` | GPU-Streifen |
| Orange | `TFT_ORANGE` | RAM-Streifen |
| Hellblau | `TFT_CYAN` | CPU-Streifen |
| Weiß | `TFT_WHITE`/`0xFFFF` | Puls-Linie |

**Ursache des grauen Hintergrunds/blassen Apfels gefunden**: `Boot::BG`
war bereits `0x0000` (identisch zum Dashboard-Hintergrund, der
ebenfalls reines `TFT_BLACK` ist, nicht dunkelblau - alle
`fillScreen()`-Aufrufe im Projekt nutzen `TFT_BLACK`). Der graue/blasse
Eindruck kam NICHT vom Hintergrund selbst, sondern vom alten
Apfel-Bild: ein Blau-Grün-Verlauf, ein halbtransparenter weißer
Glanzfleck und Floyd-Steinberg-Dithering (zur Vermeidung von
Farbbändern im Verlauf) erzeugten viele Mischfarben nahe Mint-/
Blaugrün-Tönen rund um den Apfel, die auf dem Foto wie ein grauer
Schleier wirkten. Mit dem neuen, rein zweifarbigen Apfel (nur
`TFT_GREEN`/Schwarz, kein Verlauf, kein Glanz, kein Dithering, siehe
unten) tritt der Effekt nicht mehr auf.

**Aufbau** (Querformat 320x240, alle Farben/Positionen als Konstanten
im `Boot`-Namespace in `main.cpp`):
- Hintergrund reines Schwarz `#000000` (`Boot::BG`), unverändert -
  bei Änderungen am Apfel-Design IMMER `BG` in `generate_apple.py` und
  `Boot::BG` in `main.cpp` im Gleichschritt halten.
- Titel "Eiswolfs iBoard" oben mittig, klare/fette Schrift (`AppFont`-
  Äquivalent, `pickClearFont()`), Grundlinie y=32 (`BC_DATUM`/`BL_DATUM`)
  - **mehrfarbig**: "Eiswolfs " in `TFT_GREEN`, danach "iBoard"
    buchstabenweise: i=Gelb, B=Lila, o=Orange, a=Hellblau, r=Grün,
    d=Lila. Jede Buchstaben-X-Position wird aus der GEMESSENEN Breite
    der Teil-Zeichenkette davor berechnet (`tft.textWidth(before)`),
    NICHT aus der Summe von Einzelbreiten, damit der Abstand exakt
    stimmt - die Schriftgröße wird einmalig anhand der GESAMTEN Zeile
    bestimmt und bleibt für alle Teilstrings gleich. Gesamtbreite wird
    vorher gemessen, die Zeile mittig gesetzt.
- Apfel (siehe unten) zentriert, 116x125px, Stiel-Oberkante bei y≈51
  (`Boot::APPLE_Y`) - Position/Größe unverändert.
- Weiße Puls-Linie (`drawPulseSegment()`), NICHT Teil des Apfel-Bilds,
  feste Punktliste `Boot::PULSE_POINTS` relativ zu `PULSE_BASE_Y`=122,
  x=70..250, ca. 3,5px breit (`tft.drawWideLine(...,3.5f,TFT_WHITE,BG)`)
  - war schon vorher korrekt (siehe Bugfix-Kommentar im Code: die
    Hintergrundfarbe fürs Antialiasing an den Linienenden war versehentlich
    `TFT_WHITE` statt `Boot::BG`, das wurde bereits in einer früheren
    Session behoben), unverändert gelassen. Wird über die ersten
    `Config::BOOT_PULSE_MS`=1500ms von links nach rechts aufgebaut (nur
    das neue Teilstück je Tick) und bleibt danach stehen.
- Ladebalken x=30..290, y=204, **nur noch 4px hoch** (`Boot::BAR_H`,
  vorher 8px, "filigraner"), Spur `#262626` (`Boot::BAR_TRACK`),
  Füllung `TFT_CYAN` (CPU-Farbe), runde Enden (`fillRoundRect(...,
  BAR_H/2,...)`), wächst gleichmäßig über die vollen 7s (nach Zeit,
  nicht nach WLAN-Status), pro Tick nur das neue Stück.
- WLAN-Status jetzt **horizontal zentriert** (vorher linksbündig),
  Grundlinie y=228 (`Boot::STATUS_Y`), kleine/schmale **Bibliotheks-
  schrift** (`useBuiltinFont()`, bewusst NICHT die große "klare"
  Titelschrift) in `#C8C8C8` (`Boot::STATUS_GREY`): "WLAN: verbinde..."
  -> "WLAN: <IP>" oder "WLAN: nicht verbunden" - eigener kleiner
  Zeichenbereich, überschreibt nie den Balken.
- Versionsnummer unten rechts, einzige Konstante `Config::APP_VERSION`
  in `config.h`, **bleibt in Dunkelgrau** (`Boot::VERSION_GREY`,
  unverändert) und in derselben kleinen Bibliotheksschrift wie die
  Statuszeile. Weicht bei einer langen IP-Adresse automatisch eine
  Zeile nach oben aus (`Boot::VERSION_Y_HIGH`, über den Balken), sobald
  der Abstand zur jetzt zentrierten Statuszeile unter
  `Boot::VERSION_GAP_MIN`=8px fallen würde - wird bei jeder
  Statustext-Änderung neu geprüft und bei Bedarf umgezeichnet
  (alte Position vorher löschen).

**Hintergrund-Verbindungsaufbau**: `runBootScreen()` ruft `WiFi.begin()`
selbst NICHT-BLOCKIEREND auf (eigene Uhr statt der alten, bis zu 15s
blockierenden `while`-Schleife aus dem früheren `connectWifi()`, das es
jetzt nicht mehr gibt) - WLAN verbindet sich im Hintergrund, während die
7s-Animation läuft, sodass beim Ende des Bootscreens meist schon Daten
da sind.

**Überspringen**: ein kurzes Antippen irgendwo auf dem Bildschirm
beendet den Bootscreen sofort (`TouchCalib::wasTapped()`, feuert beim
Loslassen - der Finger ist zu diesem Zeitpunkt also schon oben).
Danach läuft `Config::BOOT_TAP_LOCKOUT_MS`=300ms lang eine Tipp-Sperre
(alle Berührungen werden verworfen), damit derselbe Tap nicht
gleichzeitig einen Button der nächsten Anzeige auslöst. Der 3s-Halte-
Notausgang für die Einrichtung läuft komplett VOR `runBootScreen()` im
`setup()`-Ablauf und ist davon unberührt.

**"Keine SD-Karte"-Hinweis**: erscheint bewusst ERST NACH dem
Bootscreen (bzw. nach der Ersteinrichtung), nicht mehr davor wie
früher - der Bootscreen selbst läuft unabhängig vom SD-Status immer
durch.

### Der Apfel (`cyd/tools/generate_apple.py`)

**Komplett neu (Auftrag "Bootscreen im Dashboard-Stil", 02.11.)**: NUR
noch eine grüne KONTUR (ca. 3px breiter Ring, `Boot::OUTLINE_FINAL_PX`)
auf schwarzem Hintergrund - kein flächig gefüllter Körper mehr, kein
Verlauf, kein Glanzfleck, kein Dithering, kein Blatt (Alex' ausdrücklicher
Wunsch nach einer Zwischenfassung: "wie Apple, nur ohne das
Abgebissene" - also die klassische Zwei-Wülste-Apfel-Silhouette ohne
Biss, als reine Linie). Nur ein kleiner grüner Stiel oben bleibt übrig.
Farbe exakt `TFT_GREEN`/0x07E0 (Dashboard-Konstante, FAN-Streifen) -
das Skript prüft das nach der Erzeugung selbst per `assert` (Bildmitte
muss exakt Schwarz sein, mindestens ein Pixel muss exakt 0x07E0 sein).

Erzeugung: Die Silhouette ist EIN glatter, per Catmull-Rom-Spline
interpolierter Umriss durch wenige Kontrollpunkte (rechte Kontur
gespiegelt für links) - das war nach mehreren Fehlversuchen die einzige
robuste Methode: mehrere Ellipsen/Kreise zu vereinigen (`ImageChops.
lighter`) oder eine Scanline-Breitenfunktion mit konstanter Mittelbreite
erzeugten sichtbare Beulen bzw. eckige/kastige Kanten (siehe Git-
Historie des Skripts/Session-Verlauf, falls die Form je wieder
"komisch" aussieht - **nicht** zu einer Ellipsen-Vereinigung
zurückwechseln) - **bleibt unverändert als Kontrollpunkt-Basis**, nur
die Füllung wurde geändert. Aus der SCHARFEN (noch ungeblurrten)
Silhouette wird per Erosion (`ImageFilter.MinFilter`) eine nach innen
verkleinerte Kopie erzeugt, die Differenz (`ImageChops.subtract`)
ergibt einen gleichmäßig breiten Ring - erst DANACH leicht
weichgezeichnet (nur für glatte Kanten, nicht zur Ringverbreiterung).
Stiel: kleine, gefüllte grüne Form (eine Kontur wäre dort zu dünn, um
erkennbar zu sein), durch einen schmalen schwarzen "Hof"
(`Boot::GAP_FINAL_PX`≈2px) von der Apfel-Kontur optisch getrennt -
erst der schwarze Hof zeichnen, dann die kleinere grüne Form obendrauf,
das trennt ihn auch dort, wo er die Kontur überlappt. Gezeichnet in
4-facher Auflösung, dann mit LANCZOS auf 116x125px verkleinert (weiche
Kanten statt Pixeloptik) - Mischfarben entstehen dadurch NUR an den
Kanten zwischen Schwarz und Grün. Die RGB565-Umwandlung rundet jeden
Pixel direkt (kein Floyd-Steinberg-Fehlerstreuen mehr nötig, da es
keinen Verlauf mehr gibt, der in Stufen zerfallen könnte).

Aufruf: `cd cyd/tools && python3 generate_apple.py` (braucht Pillow).
Erzeugt `apple_preview.png`/`apple_preview_3x.png` (zur Kontrolle, IMMER
vor dem Flashen ansehen) und schreibt `cyd/src/apple_rgb565.h`
(RGB565-Array, PROGMEM, 116x125 = 29000 Bytes ≈ 28,3 KB) - bei
Änderungen am Apfel-Design dieses Skript anpassen und neu laufen
lassen, NIE `apple_rgb565.h` von Hand editieren. Eingebunden in
`main.cpp` per `tft.pushImage(x, y, APPLE_W, APPLE_H, APPLE_RGB565)` -
funktioniert auf dem ESP32 direkt aus dem Flash (PROGMEM), kein
`pgm_read_*` nötig (anders als bei AVR).

### Stolperfallen (Auftrag "Bootscreen im Dashboard-Stil", 02.11.)

- **Erosion IMMER auf der scharfen, ungeblurrten Maske rechnen, nie auf
  einer bereits weichgezeichneten** - sonst erodiert die Kontur an
  Kanten unterschiedlich stark (je nachdem wie "weich" die Kante an
  dieser Stelle schon war) und der Ring wird ungleichmäßig breit.
  Reihenfolge immer: 1) scharfe Maske bauen, 2) erodieren
  (`MinFilter`), 3) Differenz bilden, 4) ERST DANACH einen leichten
  Blur für glatte Kanten drauflegen.
- **`MinFilter`-Kernelgröße muss ungerade sein** (`ksize = erode_r*2+1`),
  sonst wirft Pillow einen Fehler.
- **`Boot::BG` in `main.cpp` und `BG` in `generate_apple.py` müssen
  identisch bleiben** (aktuell beide `0x0000`/`(0,0,0)`) - sonst zeigt
  das PROGMEM-Bild einen sichtbaren Rahmen/Kasten um den Apfel, weil der
  Bildhintergrund nicht mit dem Bootscreen-Hintergrund verschmilzt.
- **Gleichfarbige, sich überlappende Formen (Stiel auf Kontur) brauchen
  den schwarzen "Hof"-Trick** (zuerst größere schwarze Form zeichnen,
  dann kleinere grüne obendrauf) - ohne das verschmilzt der Stiel optisch
  unsichtbar mit der Kontur, sobald beide exakt dieselbe Grünfarbe haben.
- **Bei mehrfarbigem Text (Titel) die Breite des bisherigen TEXTES
  messen, nicht die einzelnen Buchstabenbreiten aufsummieren**
  (`tft.textWidth(bisherigerTeilstring)` statt
  `summe(breite(buchstabe))` für jeden Buchstaben einzeln) - Kerning/
  Fontmetrik kann von der reinen Summe abweichen, das hätte sichtbar
  ungleiche Lücken zwischen Buchstaben erzeugt.
- **Beim Verkleinern eines zuvor flächig gefüllten Elements (Ladebalken
  8px→4px, Statuszeile) immer auch Track-/Hintergrundfarbe und
  Redraw-Bereich neu prüfen** - ein zu klein geratener Löschbereich
  lässt alte Pixel der vorherigen (breiteren) Fassung stehen.
- **Freund-Firmware (`dist/*.bin`) nach JEDER Bootscreen-Änderung neu
  bauen** - sie wird nicht automatisch aktualisiert und zeigt sonst noch
  den alten Bootscreen; dabei immer das Backup-/Wiederherstellen-Muster
  für `secrets.h` einhalten (echtes Token sichern, Fake-Token einsetzen,
  bauen, Original zurückspielen, erneut bauen), NIE das echte Token im
  gemergten `.bin` landen lassen (per `grep -a -c -F` auf das Token aus
  `config.json` prüfen, danach die Temp-Datei mit dem Token löschen).

## Hauptmenü (Antippen von "iBoard" in der Kopfzeile, siehe `runMainMenu()`)

Tippfläche mindestens 80x40px (reicht bewusst etwas in die erste
Kachelreihe hinein, die Streifen dort reagieren nicht auf Touch).
Vollbild-Menü mit großen Tasten (≥40px hoch):
- **Verbindung**: drei Tasten "Auto"/"WLAN"/"USB", aktive Auswahl
  farbig gefüllt. Siehe "Verbindungsmodi" unten.
- **Schrift**: zwei Tasten "Pixel"/"Klar", aktive Auswahl farbig
  gefüllt.
- **Einrichtung**: öffnet das bestehende Untermenü "Nur WLAN ändern"/
  "Touch neu kalibrieren" (`runSetupMenu()`).
- **Zurück**: schließt das Menü, die Live-Anzeige wird komplett neu
  aufgebaut (`drawStaticLayout()` + `lastDrawnValid=false`).

Empfang und der 6s-Verbindungs-Timeout laufen im Hintergrund normal
weiter, während das Menü offen ist. Auf dem "Keine Verbindung zum
Mac"-Bildschirm heißt der Button "Menü" (statt früher "Einrichtung")
und öffnet dasselbe Hauptmenü - so lässt sich der Übertragungsweg auch
ohne Verbindung wechseln.

## Verbindungsmodi (Feld `source` in `config.json`, Standard "auto")

- **Auto**: wie bisher - kam in den letzten 3s ein gültiges USB-Paket,
  ist USB aktiv, sonst WLAN (`usbActiveNow()`).
- **WLAN**: nur UDP-Pakete werden ausgewertet, USB-Datenzeilen werden
  komplett ignoriert (der Handshake "IBOARD?"/"IBOARD_OK" bleibt davon
  unberührt, damit die Mac-App das Gerät weiter erkennt, siehe
  `pollUsbSerial()`). Der 6s-Timeout zählt nur noch WLAN-Pakete.
- **USB**: nur USB-Zeilen werden ausgewertet, UDP wird ignoriert. Das
  WLAN darf dabei verbunden bleiben, die Anzeige hängt aber nicht davon
  ab. Der 6s-Timeout zählt nur noch USB-Pakete.
- Ist ein Modus fest gewählt (nicht "Auto"), zeigt die Kopfzeile ein
  "*" hinter "USB"/"WLAN".

**Wichtig zur Stromversorgung**: Das CYD bekommt seinen Strom über das
USB-Kabel vom Mac. Zieht man das Kabel, ist das Gerät AUS - es gibt
und kann deshalb keinen automatischen Wechsel "USB→WLAN bei gezogenem
Kabel" geben. Für den späteren Betrieb am anderen Ende des Tisches
(eigenes Netzteil, kein Mac-Kabel) ist der Modus "WLAN" vorgesehen,
manuell im Menü gewählt.

## USB-Protokoll (siehe `mac/usb_link.py` und `cyd/src/main.cpp::pollUsbSerial()`)

- Mac sucht alle 5s passende serielle Ports (Namen enthalten
  "usbserial"/"wchusbserial"/"usbmodem"), öffnet sie mit `dtr=False`/
  `rts=False` **vor** dem Öffnen (sonst bootet das CYD neu), sendet
  `IBOARD?\n` und erwartet binnen 1s `IBOARD_OK\n` - nur dann gilt der
  Port als bestätigtes iBoard und bleibt offen. Andere Geräte (z.B.
  das Flightradar-CYD) antworten nicht und werden übersprungen.
- Ist ein Port bestätigt, schreibt der `SenderThread` (Mac) dieselbe
  JSON-Zeile, die auch per UDP geht, zusätzlich auf diesen Port -
  im selben festen 1s-Takt, kein eigener Thread dafür.
- CYD liest zeilenweise vom USB-Serial: `IBOARD?` → Antwort
  `IBOARD_OK`; eine Zeile, die mit `{` beginnt → wie ein UDP-Paket
  ausgewertet (`parsePacket(..., isUsb=true)`); alles andere (Boot-
  Ausgaben etc.) wird ignoriert.
- Menüpunkt (Mac-Menüleiste) "USB freigeben": schließt den Port und
  pausiert die Suche für 60s (oder bis erneutem Klick) - für
  PlatformIO-Flashes, während die App läuft.
- Ist kein CYD per USB angeschlossen, läuft die Suche im Hintergrund
  weiter, ohne die App zu blockieren - WLAN funktioniert unabhängig
  davon normal weiter.

## Technische Details / Stolperfallen

- **CYD-Display-Pinbelegung**: 1:1 vom Flightradar-Projekt übernommen
  (nur gelesen, nicht verändert) - ILI9341_2-Treiber, physische
  Panelgröße 240x320, hier aber **Querformat** (`tft.setRotation(1)`,
  effektiv 320x240). SPI-Pins TFT_MISO=12/TFT_MOSI=13/TFT_SCLK=14/
  TFT_CS=15/TFT_DC=2/TFT_RST=-1/TFT_BL=21, SPI_FREQUENCY=55000000.
  **`tft.invertDisplay(true)` ist Pflicht** direkt nach `tft.init()` -
  ohne das zeigt dieser Displaytreiber falsche/unlesbare Farben (von
  Alex live bestätigt, 29.10.).
- **Touch-Pins**: ebenfalls 1:1 vom Flightradar-Projekt übernommen -
  XPT2046 an TOUCH_CLK=25/TOUCH_CS=33/TOUCH_MOSI=32/TOUCH_MISO=39/
  TOUCH_IRQ=36, eigener SPI-Bus (VSPI), `touch.setRotation(0)` (roh,
  unrotiert - die Umrechnung auf Bildschirmkoordinaten passiert
  ausschließlich über die eigene affine Kalibrierung in
  `touch_calib.cpp`, nicht über die Touch-Chip-Rotation).
- **SD-Karten-Pins**: ebenfalls vom Flightradar-Projekt übernommen -
  eigener SPI-Bus (HSPI): SD_CS=5/SD_MOSI=23/SD_MISO=19/SD_CLK=18.
  Überschneidet sich nicht mit Display (VSPI) oder Touch (VSPI, aber
  anderer CS).
- **UDP-Port**: 4210, für Broadcast-Versand UND CYD-Empfang.
- **Datenpaket-Format**: eine JSON-Zeile pro Sekunde, siehe
  `mac/sender.py` für die genauen Feldnamen - bei Änderungen am Format
  auf beiden Seiten (Mac-Sender UND CYD-Parser) gleichzeitig anpassen.
- **macmon**: externes Homebrew-Tool, liefert GPU-Last/-Leistung und
  Temperaturen als JSON auf stdout, braucht kein sudo. Liefert auch
  die Lüfterdrehzahl (Feld `fans[0].rpm`/`fans[0].max_rpm`) - eine
  separate SMC-Abfrage war dafür nicht nötig.
- **Keine festen WLAN-Zugangsdaten mehr im Code** (Auftrag 29.10.): das
  frühere `cyd/src/wifi_credentials.h` wurde entfernt. WLAN und
  Touch-Kalibrierung liegen jetzt ausschließlich in `config.json`, die
  am Gerät selbst über die Ersteinrichtung erzeugt wird.

### Konfigurationsspeicherung (SD-Karte, Fallback NVS)

- Datei: `/iBoard/config.json` im Root der SD-Karte (siehe
  `storage.h/.cpp`). Aufbau (ohne echte Werte):
  ```json
  {
    "wifi_ssid": "...",
    "wifi_password": "...",
    "mac_ip": "",
    "font": "pixel",
    "source": "auto",
    "touch_cal": { "a": 0.0, "b": 0.0, "c": 0.0, "d": 0.0, "e": 0.0, "f": 0.0 }
  }
  ```
  `font`/`source` wurden additiv ergänzt (29.10.) - fehlen sie in einer
  älteren `config.json`, gelten die Standardwerte "pixel"/"auto", die
  Datei bleibt dadurch abwärtskompatibel. `mac_ip` leer = UDP-Broadcast
  (Normalfall). `touch_cal` ist eine
  affine Abbildung roh->Bildschirm (`screenX = a*rawX + b*rawY + c`,
  `screenY = d*rawX + e*rawY + f`), per 4-Punkt-Kleinste-Quadrate-Fit
  aus der Kalibrierung berechnet (siehe `touch_calib.cpp::fit()`) -
  bewusst KEIN einfaches Min/Max-pro-Achse wie im Flightradar-Projekt,
  weil das bei vertauschten/gespiegelten Rohachsen (z.B. durch
  Querformat) versagen würde. Der Fit korrigiert das automatisch, ohne
  die Achsenlage der Hardware vorher wissen zu müssen.
- Lade-Reihenfolge: SD zuerst, dann NVS (`Preferences`, Namespace
  `iboard`) als Fallback. Speichern: auf die SD, wenn gemountet - sonst
  in NVS als Notlösung. Fehlt die SD-Karte beim Start, zeigt das
  Display kurz "Keine SD-Karte gefunden" und fährt mit NVS fort.
- Ist weder auf der SD-Karte noch in NVS eine gültige Konfiguration
  vorhanden (kein `wifi_ssid` oder keine gültige `touch_cal`), startet
  automatisch die Ersteinrichtung.

### Ersteinrichtung (siehe `setup_flow.h/.cpp`)

1. **Touch-Kalibrierung**: vier Zielkreuze in den Ecken (Reihenfolge
   oben-links, oben-rechts, unten-rechts, unten-links), danach ein
   Testpunkt in der Mitte. Liegt der Testpunkt-Tap mehr als 40px vom
   Zentrum entfernt, gilt die Kalibrierung als fehlgeschlagen und die
   komplette Ecken-Sequenz startet automatisch neu - kein Abbruch nötig.
2. **WLAN-Suche**: `WiFi.scanNetworks()` (liefert ohnehin nur
   2,4-GHz-Netze, der ESP32 kann kein 5-GHz) mit SSID+RSSI-Liste
   (max. 6 gleichzeitig sichtbar), "Neu suchen" und "Manuell eingeben"
   (öffnet die Tastatur für versteckte Netze).
3. **Bildschirmtastatur** (`setup_flow.cpp::runKeyboard()`): Klein-/
   Großbuchstaben (Shift-Taste), Ziffern/Symbole (umschaltbar),
   Löschen, Leertaste, bei Passwortfeldern zusätzlich
   "Anzeigen"/"Verbergen". "Zurück" springt bei der Passworteingabe
   zur Netzwerkliste zurück.
4. **Verbindungstest**: `WiFi.begin()` mit Timeout 15s. Bei Erfolg
   wird `config.json` erst JETZT geschrieben (nie vorher) und die
   eigentliche Anzeige startet. Bei Fehlschlag zurück zur
   Passworteingabe (gleiche SSID).
5. Danach normaler Betrieb (UDP-Empfang + Dashboard).

### Einrichtung erneut aufrufen

- Im Hauptmenü (siehe oben) unter "Einrichtung": **getrennte** Optionen
  "Nur WLAN ändern" und "Touch neu kalibrieren" (`runSetupMenu()`) -
  bewusst nie beides gleichzeitig, um die jeweils andere, noch gültige
  Einstellung nicht aus Versehen zu überschreiben.
- Notausgang: Touchscreen beim Einschalten ca. 3 Sekunden gehalten
  (`Config::BOOT_HOLD_SETUP_MS`) erzwingt die komplette Ersteinrichtung
  (Kalibrierung + WLAN), auch wenn eine - ggf. schlechte - Kalibrierung
  bereits vorhanden ist. Gedacht für den Fall, dass die Kalibrierung so
  schlecht ist, dass der Menü-Button selbst nicht trifft.
- Der "Keine Verbindung zum Mac"-Bildschirm zeigt zusätzlich das aktuell
  verbundene (oder zuletzt konfigurierte) WLAN und die IP-Adresse an -
  damit sofort sichtbar ist, ob das Gerät im falschen Netz hängt.

### Behoben: staendiges Wechseln auf "Keine Verbindung" (29.10.)

Ursache: ESP32-WLAN-Modem-Sleep (Standard-Energiesparmodus) laesst
Broadcast-/Multicast-Pakete (unsere UDP-Sendung ist Broadcast) nur zum
naechsten DTIM-Beacon-Intervall durch den Access Point - bei schlafendem
Funkmodul kann das zu wiederkehrenden, mehrsekuendigen Verzoegerungen
fuehren, obwohl Mac und Netzwerk voellig ruhig/stabil laufen. Kombiniert
mit dem alten 3s-Timeout reichte das fuer ein falsches "Keine
Verbindung". Fix: `WiFi.setSleep(false)` nach jedem `WiFi.begin()`
(Erstverbindung UND Reconnect-Pfad), Timeout auf 6s angehoben, und die
UDP-Empfangsschleife leert jetzt ALLE wartenden Pakete pro Durchlauf
(`while (udp.parsePacket() > 0)`) statt nur eines - damit sich bei kurz
langsamerem Zeichnen kein Rueckstand aufbaut. Verifiziert per
Serial-Diagnose-Log (`Config::DEBUG_LOG`, temporaer auf `true`) ueber
~10 Minuten Laufzeit: 0 Wechsel auf "Keine Verbindung". Mac-seitig
zusaetzlich robuster gemacht (auch wenn nicht die Hauptursache): Versand
laeuft seit diesem Fix in einem eigenen `SenderThread` mit fest
getakteter absoluter Uhr (`sender.py`), komplett unabhaengig vom rumps/
Cocoa-Runloop und von macmon - eine einzelne langsame Messung verschiebt
das Sendeintervall dadurch nicht mehr nach hinten.

### Behoben: Zahl blitzt bei Aenderung kurz gross/klein auf (29.10.)

Ursache: Die Schriftgroesse einer Kachel wurde bei JEDER Werteaenderung
neu anhand der exakten aktuellen Zeichenkette gemessen (`pickFont()`,
Kandidaten gross->klein, erster Treffer der passt). Kurze Werte wie
"5%" passen bei der groessten Kandidatengroesse leicht, laengere wie
"17%" manchmal nicht (abhaengig von den tatsaechlichen Zeichenbreiten
der jeweiligen Schrift) - das fuehrte zu UNTERSCHIEDLICHEN Groessen
zwischen Kacheln UND zwischen aufeinanderfolgenden Werten derselben
Kachel (sichtbar als kurzes Aufblitzen). Fix: Die Schriftgroesse wird
jetzt einmalig pro Zeile beim Layoutaufbau berechnet (siehe
"Anzeige-Layout" oben, `computeRowFonts()`) und bei jedem einzelnen
Wert nur noch unveraendert angewendet (`applyFont(row1Font/row2Font)`
in `drawTileValue()`) - keine erneute Messung mehr pro Update. Getestet
mit schnell wechselnden Werten (0/5/17/100%, Extremwerte) per direktem
USB-Test ohne Mac-App: keine Groessenspruenge mehr, kein Absturz.

### Behoben: Abbrueche im Modus "WLAN" - UDP-Broadcast wird im Netz fast komplett verschluckt (29.10.)

Im erzwungenen Modus "WLAN" (kein USB zur Abdeckung mehr) fiel auf, dass
die Anzeige staendig auf "Keine Verbindung" sprang, obwohl der Mac
weitersendete. Messung (direkter Soak-Test, 90 Broadcast-Pakete in 90s):
**nur 1 von 90 Broadcast-Paketen kam beim CYD an - derselbe Test mit
Unicast an die bekannte CYD-IP kam zu 100% an.** Ursache liegt im
lokalen Netz (vermutlich IGMP-Snooping/Broadcast-Daempfung zwischen dem
per Kabel haengenden Mac und dem WLAN-Client CYD an der FritzBox), nicht
im Code. Im Modus "Auto" war das bisher nie aufgefallen, weil USB (immer
zuverlaessig per Kabel) die Anzeige am Leben hielt, waehrend WLAN
im Hintergrund fast komplett ins Leere lief.

Fix (genau der im urspruenglichen Auftrag vorgesehene Unicast-Fallback,
jetzt durch die Messung tatsaechlich noetig): Das CYD antwortet auf
JEDES empfangene UDP-Paket (unabhaengig von Parse-Erfolg oder Modus)
mit einem kleinen Paket, das nur die eigene IP als Text enthaelt, an
den Absender zurueck (`main.cpp`, UDP-Empfangsschleife). Der
`SenderThread` (Mac, `sender.py::_learn_cyd_ip()`) liest diese Antwort
auf demselben Socket, der auch sendet (kein erneutes `bind()` nötig,
funktioniert im LAN ohne NAT problemlos), merkt sich die gelernte IP
und schickt ab dann JEDE Zeile zusaetzlich zum Broadcast per Unicast an
diese IP (`_send()`). Greift nur, wenn `config.TARGET_IP` nicht manuell
gesetzt ist (die hat weiterhin Vorrang). Verifiziert: 60 Sende-Takte,
75 empfangene Pakete beim CYD (teils Broadcast UND Unicast fuer denselben
Tick angekommen, unschaedlich - `current` wird dadurch hoechstens doppelt
mit demselben Inhalt aktualisiert).

### Behoben: Cyanfarbener Kasten hinter "Schrift:" im Hauptmenue (29.10.)

Ursache: `drawButton()` setzt bei einer GEFUELLTEN (aktiven) Taste die
Textfarbe inkl. HINTERGRUND auf die jeweilige Akzentfarbe
(`tft.setTextColor(TFT_BLACK, color)`), bei einer leeren Taste dagegen
Vordergrund=Farbe/Hintergrund=Schwarz. Dieser Zustand bleibt in
TFT_eSPI bis zum naechsten `setTextColor()`-Aufruf aktiv. War die
zuletzt vor "Schrift:" gezeichnete Verbindungs-Taste ("USB") gerade die
AKTIVE (gefuellte, z.B. im Modus "USB"), blieb der cyanfarbene
Hintergrund fuer den direkt folgenden `tft.print("Schrift:")`-Aufruf
stehen - deshalb nur manchmal sichtbar (abhaengig vom gewaehlten
Verbindungsmodus beim Foto) und bei "Verbindung:" nie (das steht vor
jeder Taste, erbt noch die Ausgangsfarbe Weiss/Schwarz). Fix: Vor
sowohl "Verbindung:" als auch "Schrift:" wird die Textfarbe jetzt
explizit auf Weiss/Schwarz zurueckgesetzt, statt den Zustand der
vorherigen `drawButton()`-Aufrufe zu erben (`runMainMenu()` in
`main.cpp`).

### Geprueft: Weisse statt cyanfarbene Streifen auf Fotos (29.10.)

Die CPU- und die linke TMP-Kachel verwenden exakt dieselbe
`TFT_CYAN`-Konstante wie alle anderen Farbwerte im Code (keine
Sonderbehandlung, keine Byte-Tausch-/RGB565-Transformation dazwischen,
siehe `drawStripe()`/`TILES[]` in `main.cpp`) - Code-seitig also kein
Unterschied zu den korrekt dargestellten Orange-/Lila-/Gruen-Streifen.
Da das Display per USB nicht direkt einsehbar war, konnte dies nur per
Codepruefung, nicht visuell am Geraet verifiziert werden. Hellstes
Cyan ist auf Handy-/Kamerafotos erfahrungsgemaess die mit Abstand
anfaelligste Farbe fuer Ueberbelichtung/Weissabgleich-Clipping bei
hoher Hintergrundbeleuchtung - das ist die wahrscheinlichste Erklaerung.
**Bitte am Geraet selbst (nicht per Foto) gegenpruefen** - falls die
Streifen live tatsaechlich weiss statt cyan wirken, bitte erneut
melden, dann wird weiter codeseitig gesucht (z.B. Panel-spezifischer
Farbfehler nur bei dieser einen RGB565-Kombination).

## Machbarkeitstest "Lüfter steuern" (29.10., `tools/fan-test/`)

Ziel war herauszufinden, ob sich die Lüfterdrehzahl auf Alex' Mac mini
M4 per Software erhöhen lässt. Details/Recherche in
`tools/fan-test/ERGEBNISSE.md`, eigenständiges Testskript in
`tools/fan-test/fan_control.py` (SMC-Zugriff per IOKit/ctypes, Struct-
Layout und Schlüssel gegen den offenen Referenzcode von MacFanControl
geprüft, nicht neu erraten - das Referenz-Repo liegt unangetastet in
`tools/fan-test/MacFanControl/` zum Nachschlagen).

**Ergebnis: Geht, mit Einschränkungen.** Auf M3/M4 verlangt die
Firmware vor jedem manuellen Lüfterbefehl erst eine Freigabe
(`Ftst`-Schlüssel, danach automatisch per `fan_control.py` gehandhabt).
Phase 1 (~2000 RPM, 30s) wurde live bestätigt: Lüfter stieg
nachweislich von ~1000 auf ~2160 RPM, Automatikmodus ließ sich danach
zuverlässig wiederherstellen (per `sudo python3 fan_control.py reset`
oder automatisch über das Skript selbst). Phase 2/3 (~2500/~3900 RPM)
wurden in dieser Session nicht mehr durchlaufen (Test nach Phase 1 für
ausreichend erklärt) - vor einem produktiven Einbau (Teil C) sollten
sie bei Gelegenheit noch nachgeholt werden.

**Teil C (Lüfter-Boost-Funktion am CYD) wurde in dieser Session NICHT
gebaut** - bewusst zurückgestellt (Zeit-/Ressourcengrund), nicht weil
der Test fehlgeschlagen wäre. Für eine künftige Session: Die
Grundlage (geprüfte SMC-Schlüssel/Ablauf) steht bereit, siehe oben -
der Auftrag selbst (Sicherheitsregeln, Token, Watchdog, Mac-App-
Menüpunkt, CYD-Menüseite "Lüfter", neue `config.json`-Felder
`fan_boost_pct`/`fan_boost_min`) ist im Originalauftrag vom 29.10.
vollständig beschrieben und kann direkt umgesetzt werden.

**Wichtig**: `fan_control.py` ist ein reines Testwerkzeug für diese
eine Maschine - kein Dienst, keine Installation, hinterlässt nichts
Dauerhaftes. Die spätere installierbare Mac-App für andere Macs
bekommt laut Auftrag ausdrücklich KEINE Lüftersteuerung und keinen
Hilfsdienst.

## Mac-App starten, beenden, Autostart (29.10.)

- **Ort**: `/Users/alexdemurtas/Eiswolfs-iBoard/mac/sender.py` (läuft
  aus der venv in `mac/venv/`).
- **Von Hand starten**: Doppelklick auf
  `/Users/alexdemurtas/Eiswolfs-iBoard/Eiswolfs iBoard starten.command`
  im Finder - startet die App im Hintergrund, zeigt kurz eine
  Benachrichtigung und schließt das dabei kurz aufgehende Terminal-
  Fenster von selbst wieder. Läuft die App schon, beendet sich der
  neue Versuch sofort leise (siehe Single-Instance-Schutz unten).
- **Beenden**: Menüleisten-Symbol antippen -> "Beenden" (eingedeutscht,
  `self.quit_button = "Beenden"` in `sender.py`, ersetzt rumps'
  Standard-"Quit"). Startet NICHT von selbst neu (siehe Autostart).
- **Autostart (LaunchAgent)**: `cd mac && ./install.sh` richtet ihn
  ein, `./uninstall.sh` entfernt ihn wieder vollständig
  (`launchctl unload` + Plist löschen). Das Plist
  (`com.alexdemurtas.eiswolfsiboard.plist`) hat bewusst **kein
  `KeepAlive`** - startet nur einmal beim Login, läuft NICHT wieder an,
  wenn Alex die App über "Beenden" schließt. Status prüfen:
  `launchctl list | grep eiswolfsiboard`.
- **Single-Instance-Schutz**: `acquire_single_instance_lock()` in
  `sender.py` - ein `fcntl.flock()` auf `mac/.sender.lock` (nicht in
  Git), atomar und damit race-frei, falls der LaunchAgent lädt,
  während Alex die App gerade von Hand gestartet hat (oder
  umgekehrt): wer die Datei zuerst sperrt, läuft - der zweite Versuch
  erkennt das sofort und beendet sich leise, ohne doppelt zu senden.
- Stand nach der letzten Einrichtung in dieser Session: Autostart ist
  **aktiv** (mit `./install.sh` eingerichtet und getestet).

## Lüfter-Boost (Auftrag "Luefter-Boost", 02.11.)

Zusatzfunktion: der Lüfter lässt sich kurzzeitig auf eine feste Stufe
hochdrehen, um einen heissen Mac zusätzlich zu kühlen - startbar sowohl
am Mac (Menü) als auch am CYD (langes Antippen der FAN-Kachel).
**Enthält KEINE Luefter-STEUERUNG in der installierbaren DMG-App** - der
Hilfsdienst/die Rechte dafür bleiben bewusst ausserhalb des Bundles
(siehe "Nächster Auftrag" unten).

### Drei Komponenten

1. **`tools/fan-helper/`** (Root, LaunchDaemon) - `fand.py` +
   `smc_core.py` (eigene Kopie der SMC-Zugriffsschicht aus
   `tools/fan-test/fan_control.py`, damit der Dienst unabhängig vom
   Test-Ordner bleibt). Lauscht auf der Unix-Socket-Datei
   `/var/run/eiswolfs-iboard-fan.sock` (nach dem Start auf den Besitzer
   `alexdemurtas`, Modus 0600 beschränkt - NIE auf dem Netz erreichbar).
   Befehle: `PING`, `STATUS`, `BOOST <2000|3000|4000|MAX>`, `AUTO` -
   jeweils eine Zeile pro Verbindung, Antwort als JSON. Begrenzt sich
   selbst (nie unter Automatik-Minimum, nie über SMC-Maximum), hat
   einen eigenen Wachhund (kein `PING`/`BOOST` seit 10s -> Automatik)
   und eine absolute Obergrenze von 4 Stunden pro Boost. Setzt beim
   Start, bei SIGTERM und beim Beenden immer auf Automatik zurück.
   `KeepAlive: true` im Plist (bewusst ANDERS als beim App-LaunchAgent
   unten) - ein abgestürzter Hilfsdienst muss sofort neu starten, sonst
   bleibt ein laufender Boost unbeobachtet.
   - Installieren: `cd tools/fan-helper && sudo bash install.sh`
     (ermittelt den aufrufenden Benutzer über `$SUDO_USER`, schreibt
     `/Library/LaunchDaemons/com.alexdemurtas.eiswolfsiboard.fanhelper.plist`).
   - Deinstallieren: `sudo bash uninstall.sh` (sendet SIGTERM, entfernt
     Plist + Socket-Datei, Lüfter landet garantiert auf Automatik).
   - Logs: `/Library/Logs/eiswolfsiboard-fanhelper.log`/`.err.log`.
2. **`mac/fan_boost.py`** (Mac-App-Seite, braucht kein Root) -
   `FanBoostClient` spricht mit dem Hilfsdienst über die Socket-Datei,
   verwaltet die Dauer SELBST (pingt alle 2s, beendet nach Ablauf) und
   `CydCommandListener` nimmt Befehle vom CYD entgegen (UDP Port 4211,
   NICHT der Datenport 4210, plus USB-Zeilen über
   `usb_link.py::read_lines()`). Fehlt der Hilfsdienst/die Socket-
   Datei, meldet jede Funktion nur "nicht verfügbar", nichts stürzt ab.
   Ein am CYD gestarteter Boost endet automatisch, wenn die
   CYD-Verbindung >60s weg ist (`cyd_idle_seconds()` in `sender.py`) -
   ein am Mac-Menü gestarteter Boost davon bewusst NICHT betroffen.
   Zusätzliche Stopp-Wege: App-Beenden (eigener `quit_app()`-Handler
   statt des rumps-Standard-Quit-Buttons) und Schlaf-Erkennung über
   `NSWorkspaceWillSleepNotification` (PyObjC, `_install_sleep_watcher()`).
   Menü: "Lüfter-Boost" mit "Boost starten/stoppen" (zeigt Restzeit),
   Untermenüs "Dauer" (`config.FAN_BOOST_DURATIONS`) und "Drehzahl"
   (`config.FAN_BOOST_RPM_LEVELS`), beide in `config.json` gespeichert
   (`fan_boost_sec`/`fan_boost_rpm`) - gilt NUR für am Mac gestartete
   Boosts, unabhängig von der CYD-eigenen Einstellung (siehe unten).
3. **CYD-Firmware** (`cyd/src/main.cpp`) - langes Antippen (~0,5s,
   `Config::FAN_LONGPRESS_MS`) der FAN-Kachel startet/beendet den
   Boost, erkannt über den rohen, kontinuierlichen Touch-Zustand
   (`TouchCalib::rawPoint()`), NICHT über `wasTapped()` (das feuert nur
   beim Loslassen). Sendet `"FAN BOOST <sek> <stufe> <token>"`
   (0 Sekunden = "Bis Tippen") bzw. `"FAN AUTO <token>"` auf dem
   aktiven Weg (USB-Zeile oder UDP-Unicast an die gelernte Mac-IP,
   `Config::FAN_CMD_UDP_PORT` = 4211). Wartet danach bis
   `Config::FAN_CMD_CONFIRM_TIMEOUT_MS` (3s) auf Bestätigung im
   Datenstrom (Feld `fan_boost`) - keine Bestätigung -> kurz "n/v" auf
   dem Streifen, dann zurück zur normalen Anzeige. Während eines
   bestätigten Boosts wird der FAN-Streifen orange mit "BOOST" +
   Restzeit (`updateFanStripe()`), sonst normal grün "FAN / RPM" - nur
   bei tatsächlicher Änderung neu gezeichnet. Neue Menüseite "Lüfter"
   (`runFanSettingsMenu()`, erreichbar über Hauptmenü -> "Lüfter", teilt
   sich jetzt eine Reihe mit "Einrichtung") für Dauer (30s/1/2/5/10min/
   Bis Tippen) und Drehzahl (2000/3000/4000/Vollgas) - gespeichert in
   `deviceCfg.fanBoostSec`/`fanBoostRpm` (additiv in `storage.cpp`,
   Standard 60s/"MAX", wie der Mac-App-Default).

### Protokoll/Felder

- JSON-Datenzeile CYD<-Mac bekam 3 neue, abwärtskompatible Felder:
  `fan_boost` (0/1), `fan_left` (Sekunden, -1 = bis Stopp), `fan_ok`
  (1 = Hilfsdienst verfügbar). Zeigt immer den WAHREN Zustand, nicht
  nur das zuletzt vom CYD Befohlene.
- Token: ein langer Zufallswert, identisch in `cyd/src/secrets.h`
  (`FAN_BOOST_TOKEN`, NICHT in Git - siehe `.gitignore`) und in
  `config.json` (`fan_boost_token`, Datei ist `chmod 600`). Wird NIE in
  CLAUDE.md/Chat/Logs ausgegeben. Ungültiger/fehlender Token sowie mehr
  als 1 Befehl/Sekunde werden kommentarlos ignoriert.

### Stolperfallen

- **Touch-Entprellung beim langen Antippen (Bug, gefunden+behoben
  02.11.)**: Ein einzelner kurzer Aussetzer des resistiven Touch-
  Sensors MITTEN in einer langen Berührung wurde ursprünglich sofort
  als "losgelassen" gewertet - ein einziges Halten konnte dadurch ZWEI
  Lang-Antippen-Ereignisse auslösen (Boost startete und endete sofort
  wieder, beobachtet beim "Bis Tippen"-Test). Fix: `touched=false` muss
  jetzt 150ms am Stück anhalten, bevor die Berührung als beendet gilt
  (siehe die `fanReleasePending`-Logik in `loop()`).
- PyInstaller-Binaries landen in `Contents/Frameworks/`, erreichbar nur
  über den Symlink `Contents/Resources/<name>` - gilt weiterhin, siehe
  Abschnitt "Installierbare App" oben.
- Der Hilfsdienst-Socket-Pfad `/var/run/...` wird bei jedem Neustart
  neu angelegt (`os.remove` falls vorhanden) - zwei gleichzeitig
  laufende Hilfsdienst-Instanzen sind dadurch nicht vorgesehen/getestet.

## Installierbare App v1.1 (Auftrag "DMG v2", 02.11.)

Zweiter DMG-Bau - enthält jetzt zusätzlich zu den Grundfunktionen die
Menüleisten-Auswahl und den Lüfter-Boost-CLIENT (`fan_boost.py`), wie
im vorherigen Abschnitt angekündigt. `APP_VERSION` auf `"1.1"`
angehoben (in `mac/sender.py` UND `mac/eiswolfs_iboard.spec` - beide
Stellen pflegen, nicht nur eine).

- **Bauen**: identischer Ablauf wie bei v1.0 (siehe "Installierbare
  App (DMG)" oben) - `pyinstaller eiswolfs_iboard.spec`, danach
  `codesign --force --deep --sign -`. Bundle enthält jetzt auch
  `fan_boost.py`/`login_item.py`/`config.py`/`usb_link.py` automatisch
  (liegen im selben `mac/`-Ordner, PyInstaller nimmt sie über die
  Import-Kette mit).
- **DMG**: `dist/Eiswolfs-iBoard.dmg` (~11 MB) - die alte v1.0-DMG
  liegt daneben als `dist/Eiswolfs-iBoard-v1.0.dmg` (Archiv, bewusst
  nicht gelöscht).
- **Token bleibt zwingend ausserhalb des Bundles**: Die App LIEST das
  Lüfter-Boost-Token nur zur Laufzeit aus
  `~/Library/Application Support/Eiswolfs iBoard/config.json`
  (`config.get_fan_boost_token()`) - erzeugt selbst NIE eines, wenn
  keines da ist (`CydCommandListener.handle_line()` lehnt dann jeden
  CYD-Befehl ab, siehe `if not token: return`). Nach JEDEM Bau per
  Textsuche im fertigen `.app`-Bundle UND in der gemounteten DMG
  prüfen: weder das echte Token noch `/Users/<benutzer>` noch eine
  `config.json` dürfen irgendwo auftauchen (bei v1.1 bestätigt: 0
  Treffer). `bundle_resources/` enthält bewusst nur `macmon` + Lizenz,
  nichts Nutzerspezifisches.
- **Update auf Alex' Mac**: laufende App beenden, altes
  `/Applications/Eiswolfs iBoard.app` löschen, neues hinkopieren -
  der bestehende Autostart-LaunchAgent
  (`~/Library/LaunchAgents/com.alexdemurtas.eiswolfsiboard.app.plist`)
  zeigt auf denselben Pfad und muss beim Ersetzen NICHT neu
  eingerichtet werden, solange der App-Name/-Ort gleich bleibt.
  `~/Library/Application Support/Eiswolfs iBoard/config.json` bleibt
  beim Update unangetastet (alle Felder inkl. Token, Menüleisten-Wahl,
  Boost-Dauer/-Drehzahl bestätigt erhalten).
- **Getestet (v1.1)**: Bundle+DMG-Textsuche sauber, Start in
  bereinigter Umgebung (`env -i`, leerer PATH) funktioniert, Boost per
  Mac-Menü (30s) und per langem Antippen der FAN-Kachel (Start+Stopp)
  funktionieren, falsches Token wird ignoriert, Menü zeigt "nicht
  verfügbar" bei gestopptem Hilfsdienst ohne zu hängen, Autostart
  weiterhin aktiv, Lüfter am Ende auf Automatik.
- **Firmware für fremde CYDs ohne Alex' Token** (`tools/fan-helper`
  bleibt dabei komplett aussen vor, betrifft nur `cyd/src/secrets.h`):
  `cyd/src/secrets.h` kurzzeitig durch ein Beispiel-Token ersetzen,
  `pio run` bauen, mit `esptool.py merge_bin` (Offsets `0x1000`
  Bootloader/`0x8000` Partitionstabelle/`0x10000` App, `--flash_mode
  dio --flash_freq 40m --flash_size 4MB` - Standard-esp32dev-Werte,
  identisch zum normalen Build) zu EINER Datei zusammenführen, DANACH
  SOFORT Alex' echte `secrets.h` wiederherstellen und neu bauen, damit
  der Projektzustand wieder dem echten Gerät entspricht. Ergebnis:
  `dist/Eiswolfs-iBoard-CYD-merged.bin` + `dist/FLASHEN.md`. Per
  Textsuche bestätigt: Alex' echtes Token kommt darin nicht vor (0
  Treffer) - lediglich generische PlatformIO-Framework-Pfade
  (`/Users/alexdemurtas/.platformio/...`, durch Compiler-Debug-Infos
  eingebettet, in JEDER auf diesem Mac gebauten Firmware enthalten,
  keine Nutzer-/Kontodaten) tauchen auf. NIE auf Alex' eigenes Gerät
  geflasht, am echten Zielgerät nicht gegengetestet (siehe Hinweis in
  `FLASHEN.md`).

## Auftrag Ruhemodus: Fortschritt

Ziel: Antippen der PWR-Kachel am CYD schickt Alex' Mac mini in
denselben Zustand wie das Drücken der Fingerabdrucktaste (ohne
Fingererkennung). Bauen+flashen ja, NICHT committen/taggen/pushen.
Schutzregel: nur im Ordner Eiswolfs-iBoard arbeiten, `tools/fan-helper`
unberührt lassen, kein Root, kein neuer Dienst. Bei "weiter" hier
zuerst nachlesen, dann an der ersten unerledigten Stelle fortsetzen.

- [x] Schritt 1: Ausgangszustand festgehalten, Alex hat die Taste
      gedrückt, Systemprotokolle ausgewertet. **Befund:** Die
      Fingerabdrucktaste ohne erkannten Finger löst WEDER Display-
      noch System-Sleep aus, sondern eine **sofortige
      Bildschirmsperre** (`log show`: `loginwindow ->
      -[LWTouchIDLockScreen ...] | No assertions, calling to lock
      screen immediate` -> `-[LWScreenLock
      lockScreenImmediateFromTouchIDPress]` -> `startScreenLock:
      entered kLWLockFromDirectLock (8)`). Direkt danach steht im Log
      `isDisplayAwake: true` - das Display ist in diesem Moment
      technisch noch wach, das "Monitore gehen aus" ist der schwarze
      Sperrbildschirm-Shield (`mode: unlocked -> blackshield`), keine
      echte Display-Abschaltung. `pmset -g log` zeigt dabei KEINE
      Sleep/Wake-Transition - der Mac mini selbst bleibt die ganze
      Zeit voll wach, nur die Sitzung ist gesperrt. Die iBoard-App
      (inkl. `macmon`-Subprozess) läuft während der Sperre
      unveraendert weiter und sendet normal ans CYD - das CYD zeigt
      also weiterhin Werte, keine "Keine Verbindung"-Meldung.
      **Gewaehlter Befehl** (kein Root, kein neuer Dienst):
      `osascript -e 'tell application "System Events" to keystroke
      "q" using {control down, command down}'` (simuliert Cmd+Ctrl+Q,
      die Standard-Sperrbildschirm-Tastenkombination). Live getestet:
      landet im `log show` exakt im selben Codepfad wie der
      Tastendruck (`startScreenLock: entered kLWLockFromDirectLock
      (8)`), einziger Unterschied ein interner Timing-Parameter
      (`useShortTimeout:0` statt `1`), ohne erkennbare Auswirkung.
      Von Alex live bestätigt: fühlt sich identisch zum Tastendruck
      an.
      **NACHTRAG (noch selbiger Auftrag, nach Rückfrage von Alex):**
      Die ursprüngliche Entscheidung "kein `pmset sleepnow`" wurde
      bewusst REVIDIERT. Alex wusste nicht, dass die Taste nur
      sperrt statt schläft, und wollte stattdessen den
      stromsparenderen echten System-Sleep - mit der offenen Frage,
      ob sich ihr Mac mini (Bluetooth-Tastatur, siehe unten) daraus
      auch per Fingerabdruck wieder aufwecken lässt. Manuell
      getestet (`pmset sleepnow`, ohne unseren Code): Mac mini ging
      in echten Sleep, Aufwecken UND Entsperren per
      Fingerabdrucktaste hat Alex live bestätigt - funktioniert
      trotz Bluetooth-Verbindung der Tastatur (laut
      `system_profiler SPBluetoothDataType`: "Magic Keyboard von
      Alex" per Bluetooth, NICHT per Kabel - bei Mac mini/Studio
      eigentlich eine bekannte Apple-Einschränkung für Touch-ID-
      Entsperrung, hier aber offenbar kein Problem). **Finale
      Entscheidung: `mac_sleep.py` nutzt jetzt `pmset sleepnow`
      (echter System-Sleep) statt der Cmd+Ctrl+Q-Bildschirmsperre**
      - die Funktion heißt jetzt `sleep_now()` (umbenannt von
      `lock_screen()`, Aufrufstellen in `sender.py` mitgezogen). Die
      physische Taste selbst bleibt unverändert bei "nur Sperren"
      (dafür bietet macOS keine Einstellung) - nur UNSER eigener
      Ruhemodus-Befehl (CYD-Kachel + Menüpunkt) löst jetzt echten
      Sleep aus, nicht mehr dasselbe wie die Taste.
- [x] Schritt 4 (Wake-on-LAN) **entfällt**: Voraussetzung war laut
      Auftrag "nur falls Schritt 1 System Sleep ergibt" - Schritt 1
      ergab aber Bildschirmsperre, kein System Sleep. Zusätzlich hat
      Alex ausdrücklich gesagt, dass sie selbst per Fingerabdruck
      aufweckt und der Taster/die PWR-Kachel nur zum Ausschalten/
      Sperren da sein soll - kein Aufweck-Button, keine MAC-Adresse-
      Erfassung, kein Magic Packet gebaut.
- [x] Schritt 2: Mac-App fertig.
      - `mac/mac_sleep.py` (neu, Funktion seit dem Nachtrag oben
        `sleep_now()`): ruft `pmset sleepnow` (echter System-Sleep,
        kein Root nötig), fängt jeden Fehler ab (gibt `False`
        zurück, wirft nie).
      - `mac/fan_boost.py`: `CydCommandListener` bedient jetzt
        zusätzlich `SLEEP <token>` (eigener `_SLEEP_RE`, eigene,
        von FAN-Befehlen unabhängige 1s-Ratenbegrenzung
        `_last_sleep_cmd_monotonic`/`CYD_SLEEP_CMD_MIN_INTERVAL_S`)
        auf DEMSELBEN Port 4211/derselben USB-Zeilen-Weiterleitung -
        bewusst EIN gemeinsamer Listener statt eines zweiten (Port
        lässt sich nur einmal binden). Nutzt dasselbe Token wie der
        Lüfter-Boost (`config.get_fan_boost_token()`), keine zweite
        Geheimnis-Datei. Token steht wie gehabt nirgends im Log.
      - `mac/sender.py`: `SenderThread.sleep_ack_counter` (+
        `bump_sleep_ack()`, thread-sicher über den bestehenden Lock) -
        neues, rein additives JSON-Feld `"sleep_ack"` in `_collect()`,
        alle bisherigen Felder unverändert. `IBoardApp._do_sleep()`
        beendet zuerst einen laufenden Lüfter-Boost (zurück auf
        Automatik), ruft dann `mac_sleep.sleep_now()` - gemeinsam
        genutzt von `_on_cyd_sleep()` (CYD-Befehl, zählt danach
        `sleep_ack` hoch) UND dem neuen Menüpunkt "Ruhemodus jetzt"
        (`trigger_sleep_now()`). Beides in try/except, kann die App
        nie beenden.
- [x] Schritt 3: CYD-Firmware fertig (`cyd/src/main.cpp`).
      - `sendSleepCommand()`: "SLEEP <token>" auf demselben Weg/Port
        wie `sendFanCommand()` (USB-Zeile oder UDP an die gelernte
        Mac-IP, `Config::FAN_CMD_UDP_PORT`=4211).
      - Tippfläche: bereits vorhandenes, bisher ungenutztes
        `pwrTileRect` ({106,0,214,34} bei 320px Breite) - deckt
        gelben Streifen + Wattzahl zusammen ab, überlappt
        `headerLeftRect` (Menü) nicht, da es exakt bei
        `HEADER_LEFT_W` beginnt.
      - Einfaches Antippen (`TouchCalib::wasTapped()`, kein langes
        Halten) + eigene 3s-Sperre (`pwrTapLockedUntilMs`,
        unabhängig von der Touch-eigenen Entprellung) gegen
        prellende Doppel-Befehle.
      - Neuer `PwrFeedback`-Zustand (`None`/`Ruhe`/`NotAvailable`):
        "RUHE ..." sofort beim Antippen für 2s
        (`PWR_FEEDBACK_RUHE_MS`), `sleepCmdPending`/
        `sleepCmdBaselineAck` warten unabhängig davon bis zu
        `Config::FAN_CMD_CONFIRM_TIMEOUT_MS` (3s, bestehende
        Konstante wiederverwendet) auf eine Änderung von
        `current.sleepAck` - bleibt sie aus, kurz "n/v"
        (`PWR_FEEDBACK_NA_MS`=1,5s), danach immer zurück zur
        normalen Wattzahl. `drawPwrFeedbackText()` zeichnet nur in
        `pwrValueRect` (dieselbe Fläche wie der Wattwert), in
        TFT_YELLOW (Streifenfarbe); `drawHeaderPwrValueNew()` lässt
        diese Fläche während eines aktiven Feedback-Zustands
        bewusst in Ruhe (kein Überschreiben/Flackern).
- [x] Schritt 5: installierte App beendet, Entwicklungskopie
      gestartet (lernt CYD-IP, findet USB), `cyd`-Projekt gebaut und
      auf das angeschlossene CYD geflasht (`/dev/cu.wchusbserial31410`).
      Tests a-g:
      - [x] c) Falsches Token wird ignoriert - per UDP selbst getestet,
            keine Reaktion, kein Lock ausgelöst.
      - [x] f) Laufender Lüfter-Boost wird beim Ruhebefehl beendet -
            selbst getestet (noch GEGEN die erste Fassung mit
            Bildschirmsperre, vor dem System-Sleep-Nachtrag): Boost
            gestartet (`boost_active: true`), SLEEP mit echtem Token
            gesendet, danach `boost_active: false, mode: AUTO` UND
            Sperre im Log bestätigt. Fan-Stop-Reihenfolge in
            `_do_sleep()` seitdem unverändert (läuft vor dem
            `mac_sleep`-Aufruf, unabhängig davon was dieser genau
            macht) - nicht erneut mit echtem Sleep nachgetestet, um
            keinen weiteren unnötigen Sleep-Zyklus auszulösen.
      - [x] a) Menüpunkt "Ruhemodus jetzt" - von Alex live getestet,
            funktioniert (echter Sleep, danach Fingerabdruck-Aufwecken
            bestätigt).
      - [x] b) Antippen der PWR-Kachel löst den Ruhemodus aus - von
            Alex live getestet, funktioniert.
      - [~] d) Zweimaliges schnelles Antippen löst nur einen Befehl
            aus - NICHT einzeln gegengetestet (Alex' Rückmeldung "alles
            funktioniert" war allgemein, kein gezielter Doppel-Tipp-
            Test). 3s-Sperre im Code per Review bestätigt (Schritt 3),
            gilt als wahrscheinlich i.O., aber nicht explizit bestätigt.
      - [ ] e) Ohne laufende Mac-App zeigt die Kachel "n/v" - noch nicht
            getestet.
      - [~] g) Menü links oben öffnet weiter wie bisher - NICHT gezielt
            gegengetestet (nur das Mac-Menü wurde für "Ruhemodus jetzt"
            benutzt, nicht das CYD-eigene Menü oben links).
      Installierte App (v1.1, OHNE Ruhemodus) wieder gestartet,
      Entwicklungskopie sauber beendet. Schutzregel-Check: Flightradar/
      Solarguardian `git status` leer, Toolbox nur bekannte Altlasten
      (15 Zeilen, unverändert zur Session davor), `tools/fan-helper`
      (fand.py, MD5 geprüft) unangetastet, lief die ganze Zeit
      durchgehend als eigener Root-Prozess weiter. Nichts committet,
      getaggt oder gepusht (kein Git-Repo in diesem Projektordner
      ohnehin). Die Firmware für den Bekannten (`dist/Eiswolfs-iBoard-
      CYD-merged.bin`) wurde NICHT neu gebaut - bekommt die
      Antipp-Funktion erst beim nächsten Neubau automatisch mit,
      bleibt dort aber ohne (Alex' echtes) Token wirkungslos.

Auftrag abgeschlossen. Offene Kleinigkeiten für später: Test d)
(Doppel-Tipp-Entprellung) und g) (CYD-Menü direkt neben der PWR-Kachel)
wurden nicht gezielt einzeln gegengetestet, nur die Kernfunktion (a/b).
Test e) ("n/v" ohne laufende Mac-App) noch offen.

## Auftrag Sprache DE/EN, Celsius/Fahrenheit, Helligkeitsregler: Fortschritt

Hintergrund: Alex will das Projekt als öffentliches Repo
"Eiswolfs-iBoard-CYD" releasen (siehe README/LICENSE weiter unten) -
ein rein deutsches Menü/Screenshots lockt laut Alex niemanden an, daher
Englisch als zweite, umschaltbare Sprache. Zusätzlich gewünscht: Celsius/
Fahrenheit umschaltbar, und ein Helligkeitsregler fürs Display (kam als
spontaner Nachtrag "Sorry!" während derselben Aufgabe dazu). Build+Flash
ja, NICHT committen/taggen/pushen (gilt weiterhin wie beim Ruhemodus-
Auftrag), Schutzregel unverändert (nur dieser Ordner, `tools/fan-helper`
unberührt).

- [x] **CYD-Firmware, Sprache DE/EN**: komplett neue i18n-Infrastruktur
      (gab es vorher gar nicht in diesem Projekt) - `cyd/src/i18n.h`
      (`enum class StringId` + `I18n`-Namespace), `cyd/src/i18n.cpp`
      (zwei parallele `const char*`-Arrays DE/EN in EXAKT derselben
      Reihenfolge wie das Enum, `static_assert` gegen `StringId::COUNT`
      - identisches Muster wie bei Flightradar/Solar Guardian, dort
      schon bewährt). Sprache additiv in `Storage::DeviceConfig::
      language` (Default "de", SD+NVS). Erste Ersteinrichtungsseite
      fragt jetzt "Sprache / Language" (bewusst NICHT übersetzt, da die
      Sprache zu diesem Zeitpunkt noch nicht feststeht) ab, bevor
      Kalibrierung/WLAN laufen. Hauptmenü hat einen Umschalt-Button
      ("Sprache: DE"/"Language: EN", `runMainMenu()`), toggelt+speichert+
      rendert sofort neu. ALLE bisher deutschen UI-Strings (Kalibrierung,
      Bildschirmtastatur, WLAN-Scan/Verbindung, "Keine Verbindung"-
      Schirm, Hauptmenü, Einrichtung-Untermenü, Lüfter-Boost-Menü)
      durch `I18n::t(StringId::...)` ersetzt - nur echte Eigennamen
      ("Eiswolfs iBoard", "PWR") und reine Zahlen (RPM-Werte) blieben
      bewusst als Literal.
- [x] **CYD-Firmware, Helligkeitsregler** (spontaner Nachtrag): TFT_eSPI
      schaltet den Backlight-Pin (`TFT_BL`=21, siehe platformio.ini) nur
      digital HIGH/LOW, kein PWM - `applyBrightness()` in `main.cpp`
      übernimmt das selbst per `analogWrite(TFT_BL, ...)` (ESP32-
      Arduino-Core-Kompatibilitäts-Shim über ledc, funktioniert auf
      alten UND neuen Core-Versionen ohne eigenes ledcSetup/ledcAttach).
      Neues additives Feld `Storage::DeviceConfig::brightness` (10-100,
      Default 100 = unverändertes Verhalten für bestehende Geräte).
      Neue Reglerseite `runBrightnessMenu()` (+/- Stepper in 10%-
      Schritten statt Drag-Slider - dieses Projekt hatte noch keinen
      Dragging-Widget-Code, +/- ist robust mit dem bestehenden Tap-
      Muster), erreichbar über einen neuen dritten Button im
      Einrichtung-Untermenü (`runSetupMenu()`, Zeilenhöhen dafür leicht
      gestaucht, passt noch auf den 240px hohen Bildschirm). Neue
      StringId `SETUP_BRIGHTNESS` ("Helligkeit"/"Brightness").
      `pio run` + Flash auf `/dev/cu.wchusbserial31410` erfolgreich
      (RAM 14.7%, Flash 71.2%).
- [x] **Mac-App, Sprache DE/EN + Celsius/Fahrenheit**: neue Datei
      `mac/i18n.py` (Dict-basiert, `t(key, **kwargs)` liest
      `config.get_language()` bei JEDEM Aufruf neu - kein Caching, ein
      Sprachwechsel wirkt sofort beim nächsten `update_ui()`-Takt (läuft
      ohnehin jede Sekunde) UND beim nächsten Öffnen eines Untermenüs;
      zusätzlich `format_temp(celsius)` für die Einheit). `mac/
      config.py`: additive `get_language()`/`set_language()` (Default
      "de") und `get_temp_unit()`/`set_temp_unit()` (Default "c") nach
      exakt demselben Muster wie `get_menubar_item()`. **Wichtig:**
      Alex hat die zunächst erwogene Geolocation-/Systemregion-basierte
      automatische Standard-Einheit ausdrücklich WIEDER VERWORFEN ("das
      mit der Geolocation wieder vergessen") - der Standard ist jetzt
      schlicht fest "c" (Celsius), umstellbar im Menü, keine
      Locale-Erkennung im Code. Alle bisher hardcodierten deutschen
      Strings in `mac/sender.py` (Menüleisten-Titel, alle
      `rumps.MenuItem`-Labels, `update_ui()`, die drei `rumps.alert()`-
      Texte, `toggle_pause`/`toggle_usb_release`) sowie in `mac/
      fan_boost.py` (`duration_label()`/`rpm_label()`) durch
      `i18n.t(...)` ersetzt. Neue Untermenüs "Sprache"/"Language" und
      "Einheit"/"Units" (gleiches Muster wie das bestehende
      "In Menüleiste anzeigen"-Untermenü, Haken an der aktiven Wahl).
      Da rumps-Menütitel beim Erzeugen einmalig gesetzt werden (nicht
      live übersetzt), zieht eine neue `_retranslate_menu()`-Methode
      ALLE fest angelegten Menütitel sofort nach, wenn die Sprache im
      Menü gewechselt wird (inkl. Lüfter-Boost-Untermenüs, Autostart-
      Eintrag, "Ruhemodus jetzt", "Beenden"). `check_apple_silicon()`
      (Start-Fehlermeldung für Intel-Macs) blieb bewusst unübersetzt -
      betrifft weder Alex noch ihren Bekannten (beide Apple Silicon),
      kein normaler Nutzungspfad.
      **Getestet:** `py_compile` fehlerfrei, Entwicklungskopie kurz
      live gestartet (installierte App dafür sauber per `osascript
      ... quit` beendet, danach wieder per `open` gestartet) - Start
      ohne Fehler, CYD-IP sofort per WLAN gelernt. Menü selbst (Sprach-/
      Einheiten-Umschaltung in der laufenden App) noch NICHT einzeln
      von Alex am echten Menü durchgeklickt.
- [x] **DMG neu gebaut** (Alex hat das englische Menü live am Gerät
      bestätigt, danach grünes Licht für v1.0 gegeben): Mac-App-interne
      `APP_VERSION` in `mac/sender.py`/`mac/eiswolfs_iboard.spec` von
      "1.1" auf "1.0" gesetzt (erster öffentlicher Release, eigene
      Zählung unabhängig von der fortlaufenden Entwickler-Build-Nummer
      davor). `pyinstaller` gebaut, ad-hoc signiert
      (`codesign --verify`: "valid on disk"), `dist/Eiswolfs-iBoard.dmg`
      neu erstellt (~11 MB). Token-Leak-Check (echten Token aus
      `~/Library/Application Support/.../config.json` gelesen, NIE
      angezeigt, nur per `grep -c` auf 0 Treffer geprüft) in App-Bundle
      UND DMG: beide 0 Treffer.
- [x] **Öffentliches GitHub-Repo `Eiswolfs-iBoard-CYD` erstellt und
      released (02./03.11.)**: Alex hat explizit "ja alles sieht gut
      aus... dann können wir v1.0 pushen" gesagt (nach Live-Test des
      englischen Menüs am Gerät) sowie "Kann man beides CYD und Macapp
      über webflasher anbieten?" gefragt.
      - Drei Galerie-Fotos von Alex nach `docs/images/` umbenannt
        (`menu.jpg`=Englisches Menü, `fan-boost.jpg`=Boost aktiv,
        `device.jpg`=Gerät auf dem Tisch), `docs/gallery.md` mit allen
        vier Bildern (inkl. vorhandenem `main.jpg`) befüllt. **Wichtig:**
        ein zusätzliches, zwischendurch gesendetes Solar-Guardian-Foto
        war ein Versehen ("sorry das foto gehört raus") - NICHT in
        dieses Projekt übernommen.
      - **Eigener Web-Flasher** nach dem Vorbild von Flightradar/Solar
        Guardian gebaut (ESP Web Tools `esp-web-install-button`):
        `index.html` (Cyan/Eis-Theme, zwei Abschnitte - CYD-Flash-Button
        UND Mac-App-Download-Button) + `manifest.json` +
        `bootloader.bin`/`partitions.bin`/`firmware.bin` (UNGEMERGT,
        Standard-Offsets 0x1000/0x8000/0x10000) liegen im Repo-Root,
        über GitHub Pages ausgeliefert unter
        `https://eiswolf-bg.github.io/Eiswolfs-iBoard-CYD/` (Pages per
        `gh api .../pages` aktiviert, `source.branch=main`,
        `source.path=/`). Live geprüft: Seite lädt (200), `manifest.json`
        korrekt erreichbar, DMG-Download-Link
        (`.../releases/latest/download/Eiswolfs-iBoard.dmg`) löst
        korrekt zum v1.0-Release-Asset auf.
      - **Für DIESEN Web-Flasher-Build (die drei .bin-Dateien im
        Repo-Root UND `dist/Eiswolfs-iBoard-CYD-merged.bin` fürs
        Release) wurde erneut - wie beim früheren Freund-Build -
        `cyd/src/secrets.h` kurz durch einen Wegwerf-Token ersetzt**
        (Inhalt NIE angezeigt, nur strukturell per
        `sed -E 's/"[^"]{6,}"/"***MASKED***"/g'` verglichen, um
        sicherzugehen dass die neue Datei dieselbe Form hat), gebaut,
        per `grep -c` auf 0 Treffer des echten Tokens in allen vier
        Binärdateien geprüft (0/0/0/0), DANACH sofort die echte
        `secrets.h` wiederhergestellt und das Projekt erneut gebaut
        UND auf das angeschlossene Testgerät geflasht (damit der lokale
        Stand wieder exakt dem echten Gerät entspricht, wie bei jedem
        Freund-Firmware-Build zuvor).
      - `README.md`: neue Stichpunkte für Sprache/Einheiten/Helligkeit,
        neuer Abschnitt "Web installer" mit Link, Installation-Abschnitt
        verweist jetzt zuerst auf den eigenen Web-Flasher.
        `dist/FLASHEN.md`: eigener Web-Flasher jetzt als empfohlene
        Methode, der bisherige generische esptool-js-Weg bleibt als
        Alternative erhalten.
      - `.gitignore` ergänzt um `mac/.sender.lock` (leere Lock-Datei,
        kein Zugewinn im Repo). Vor dem ersten Commit vollständiger
        Secrets-Scan: `cyd/src/secrets.h` korrekt ausgeschlossen (via
        `.gitignore`), `git add -A -n`-Trockenlauf durchgesehen, alle
        `.plist`-Dateien auf Token/Secrets geprüft (keine gefunden, nur
        unkritische lokale Pfade mit Alex' macOS-Benutzernamen).
      - Repo erstellt (`gh repo create Eiswolf-BG/Eiswolfs-iBoard-CYD
        --public`), initialer Commit (57 Dateien), gepusht, Tag `v1.0`
        gesetzt und gepusht, GitHub Release `v1.0` erstellt mit drei
        Anhängen (`Eiswolfs-iBoard.dmg`,
        `Eiswolfs-iBoard-CYD-merged.bin`, `FLASHEN.md`).
      - Schutzregel-Check danach: Flightradar/Solar Guardian `git
        status` weiterhin leer, Toolbox unverändert (15 Zeilen
        Altlasten), `tools/fan-helper`-Root-Prozess lief die ganze Zeit
        durchgehend weiter.
- [x] **Solar-Guardian-Nebenfrage geklärt** (nicht Teil dieses Projekts,
      nur zur Dokumentation): Alex' Beobachtung "Leistung 0W, kein
      Schlafmodus" und später "wo ist die Mondsichel?" stellten sich
      als zwei harmlose, bereits vorhandene Verhaltensweisen heraus
      (Solar-Ertragsabfrage pausiert nachts bereits wie gewollt,
      Batterie/Wetter laufen bewusst mit separatem, reduziertem Takt
      weiter; die fehlende Mondsichel lag daran, dass Solar Guardian
      genau waehrend des 22:30-22:55-Abschlussfensters per Silent-Push
      neu geflasht wurde - kein Bug). Details stehen NICHT hier (falsches
      Projekt), sondern wurden nur im Chat besprochen.

## Auftrag Wake-on-LAN + Windows-Shutdown + Helligkeitsregler-Vorlaeufer: Fortschritt

Hintergrund: Alex schaltet ihren Windows-Laptop bisher per Siri/WoL-
Shortcut ein. Wunsch: dasselbe + Herunterfahren direkt vom CYD aus,
ohne dafuer das Handy/Siri zu brauchen.

- [x] **CYD-Firmware, Wake-on-LAN**: `sendWolPacket()` (`main.cpp`) baut
      das Magic Packet SELBST und verschickt es per WLAN-Broadcast
      (Port 9) - braucht dafuer NICHT den Umweg ueber eine Mac-/Windows-
      App. MAC-Adresse additiv in `Storage::DeviceConfig::
      wolMacAddress` (leer = Feature inaktiv), NIE im Quellcode (Repo
      ist oeffentlich) - nur ueber einen neuen Menuepunkt "WoL-MAC"
      (Einrichtung-Untermenue) am Geraet eingegeben, dafuer `SetupFlow::
      runTextEntry()` neu als duenner oeffentlicher Wrapper um das
      bisher private `runKeyboard()` ergaenzt (`setup_flow.h/.cpp`).
      **Bildschirmtastatur bekam dabei einen fehlenden ":" -Knopf** (fest
      in der Ziffernreihe, 11 statt 10 Spalten, `ROW_DIGITS`/
      `ROW_SYMBOLS` in `setup_flow.cpp`) - ohne den war eine MAC-Adresse
      am Geraet gar nicht eingebbar. **Shift startet jetzt direkt aktiv**
      (`bool shift = true;` statt `false`, Auftrag "Capslock von
      vornherein an") - die Umschalttaste selbst war schon immer ein
      echter Toggle (kein Reset nach einem Buchstaben), nur der
      Startzustand war unpraktisch.
      Ausloeser: **kurzes Antippen der CPU-Temp-Kachel** (bewusst NICHT
      dieselbe Kachel/derselbe Mechanismus wie der Mac-Ruhemodus-Button
      - Alex' ausdruecklicher Wunsch, falls ein Touch mal falsch erkannt
      wird). Live getestet (vor dem Firmware-Build, testweise direkt per
      Python-Magic-Packet vom Mac aus an die per `arp -a`/
      `dscacheutil` ermittelte MAC gesendet): Laptop fuhr hoch,
      MAC-Adresse damit verifiziert, BEVOR sie am Geraet eingetragen
      wurde.
- [x] **CYD-Firmware, Windows-Shutdown**: `sendShutdownCommand()`
      schickt `"SHUTDOWN <token>"` per UDP-Broadcast an einen neuen,
      eigenen Port (`Config::WIN_SHUTDOWN_UDP_PORT`=4212, getrennt vom
      Mac-Weg Port 4211 UND vom WoL-Port 9) - nutzt denselben
      `FAN_BOOST_TOKEN` wie Luefter-Boost/Ruhemodus, kein zweites
      Geheimnis. Ausloeser: **langes Halten DERSELBEN CPU-Temp-Kachel**
      (Alex' expliziter Wunsch "selbe wie an nur longpress", nachdem sie
      erst nach einer separaten Kachel gefragt wurde). **Wichtige
      Faustregel gelernt/wiederverwendet** (galt schon fuer die
      FAN-Kachel): `TouchCalib::wasTapped()` kennt KEINE Zeitgrenze -
      auch nach einem langen Halten feuert sie beim Loslassen. Ohne
      Gegenmassnahme haette ein langes Halten IMMER zusaetzlich den
      WoL-Kurztipp ausgeloest. Fix: dateiweites (nicht lokal-statisches)
      Flag `cpuTempLongFired`, vom WoL-Kurztipp-Handler per
      `!cpuTempLongFired` abgefragt, vom neuen Lang-Halten-Block
      (exaktes Duplikat des FAN-Blocks inkl. 150ms-Entprellung) gesetzt/
      zurueckgesetzt - siehe Kommentare an beiden Stellen in `main.cpp`.
- [x] **"?"-Erklaerungen im Einrichtung-Untermenue** (Auftrag "niemand
      weiss was WoL-MAC bedeutet"): neue `showHelpPopup()`-Funktion
      (`main.cpp`, Titel + manuell per "\n" umgebrochener Text, OK-
      Knopf) - dieses Projekt hat (anders als Flightradar/Solar
      Guardian) keinen automatischen Wortumbruch-Helfer, deshalb bewusst
      simpler, manuell umbrochener Text statt eines neuen `fitText()`-
      Systems. Kleiner "?"-Knopf neben jedem der vier Setup-Menue-
      Eintraege (WLAN/Kalibrierung/Helligkeit/WoL-MAC), oeffnet je einen
      kurzen Erklaertext. Neue StringIds `HELP_OK`/`SETUP_*_HELP`.
- [x] **Windows-Listener** (komplett neuer Projektteil, `windows/`):
      Mac mini kann wie gewohnt NICHT gleichzeitig Build-Plattform fuer
      eine echte Windows-`.exe` sein - geloest durch **Cross-Kompilieren
      von macOS aus**, ohne jemals einen Windows-Rechner zu brauchen:
      - `windows/listener/main.go` (Go, `GOOS=windows GOARCH=amd64 go
        build`): lauscht auf UDP 4212, prueft `"SHUTDOWN <token>"` gegen
        den in `%APPDATA%\EiswolfsIBoard\config.json` gespeicherten
        Token (gleiches additive-Default-Muster wie `mac/config.py`),
        ruft bei Treffer `shutdown /s /t 0 /f` auf.
      - **Erste Fassung war ein reiner unsichtbarer Hintergrundprozess**
        (kein Fenster, kein Icon) - Alex' berechtigte Kritik: "das
        findet niemand", und noch wichtiger, ein geaenderter Token
        brauchte einen manuellen Prozess-Neustart (Task-Manager) ohne
        erkennbaren Weg, das Programm wiederzufinden. **Komplett
        umgebaut auf ein echtes Taskleisten-Programm**
        (`github.com/getlantern/systray`, reines Go/Syscalls unter
        Windows, cross-kompiliert problemlos ohne CGO): Menue mit
        Status-Zeile ("Token gesetzt"/"KEIN Token"), **"Token neu
        laden"** (liest `config.json` neu ein, OHNE Prozess-Neustart -
        loest Alex' eigentliches Problem direkt), "Konfiguration
        oeffnen" (Explorer), "Protokoll oeffnen" (Notepad), "Beenden".
      - Eigenes Icon (`windows/listener/icon.ico` + `windows/installer/
        icon.ico`, per Python/Pillow erzeugte Schneeflocke in der
        Eis-Farbe `#22c3ff`, `go:embed` ins Binary eingebettet, kein
        separates Icon-File zur Laufzeit noetig) - mehrere Groessen
        (16-256px) in einer `.ico`.
      - `windows/installer/installer.nsi` (NSIS, `makensis` - ebenfalls
        von macOS aus baubar, kein Windows noetig): installiert nach
        `%LOCALAPPDATA%\EiswolfsIBoardWin\`, legt NEU eine Startmenue-
        Programmgruppe "Eiswolfs iBoard" an (Start- + Deinstallieren-
        Verknuepfung, Auftrag "ein Eintrag bei Start/Programme") UND
        weiterhin eine Autostart-Verknuepfung, startet die exe direkt
        nach der Installation. `RequestExecutionLevel user` - keine
        Admin-Rechte noetig.
      - `docs/TOKEN.md` (neu): zentrale, oeffentliche Erklaerung des
        gemeinsamen Tokens (was er ist, warum er NICHT WoL betrifft,
        wie man seinen EIGENEN fuer alle drei Stellen setzt - Auftrag
        "das muss auf Github erklaert werden, mit diesem ominoesen
        token"). `windows/README.md` (neu) dokumentiert Installation/
        Bau/Funktionsweise des Listeners. Haupt-`README.md` verlinkt
        beide, neuer Feature-Punkt, "Building from source" ergaenzt.
      - `.gitignore`: `windows/installer/*.exe` ergaenzt (gebaute
        Binaries, wie `dist/` nur als Release-Asset, nie committen).
      - **Testablauf mit Alex**: Token ueber Finder (`open -R`) selbst
        in der Mac-App-`config.json` nachgeschaut (NIE von Claude
        ausgegeben/geloggt - einzige Ausnahme: Alex markierte ihn selbst
        per IDE-Selektion in ihrem eigenen Editor, dadurch im Kontext
        sichtbar - daraufhin auf ihre Bitte hin direkt eine fertige
        `config.json` dafuer geschrieben, nie im Chat-Text wiederholt).
        Exportiert auf einen frisch (auf Alex' Wunsch) von NTFS auf
        exFAT formatierten USB-Stick (`diskutil eraseDisk ExFAT
        EISWOLF /dev/disk4` - Mac-/Windows-kompatibel), dort installiert,
        Token eingetragen, "Token neu laden" getestet - **von Alex live
        bestaetigt: "funktioniert super!"**.
- [ ] **Noch nicht final geprueft:** die neueste (Tray-)Fassung des
      Windows-Listeners (mit Icon/Startmenue/"Token neu laden") wurde
      von Alex noch nicht end-zu-Ende installiert UND getestet (sie
      bestaetigte nur die VORHERIGE, fensterlose Fassung als
      funktionierend) - beim naechsten Mal nachfragen/bestaetigen
      lassen, falls das Thema erneut aufkommt.
- [ ] **Noch nicht committed/gepusht:** alles in diesem Abschnitt
      (CYD-Aenderungen, `windows/`, `docs/TOKEN.md`, README-Aenderungen,
      `.gitignore`) liegt nur lokal - Alex muss das Pushen noch
      ausdruecklich anfordern (siehe Projektregel oben).
