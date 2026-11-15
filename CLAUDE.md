# Eiswolfs iBoard - Projektkontext für Claude Code

Lies diese Datei ZUERST, bevor du irgendwas an diesem Projekt änderst.

## Auftrag Release v1.2.0, Teil 2: Fortschritt (15.11., laufend)

Teil 2 = Release-Dateien bauen, durchsuchen, PDFs, Schlüssel erneuern. Weiter NUR lokal: kein Commit/Tag/Push/Release,
nichts auf GitHub. Entscheidung Alex: vorgebaute Firmware bleibt OHNE echten Schlüssel (Beispiel-Token), nur dokumentiert
(kein Schlüssel-Eintrag am Gerät). Bei "weiter": zuerst diesen Abschnitt lesen.

- [x] Prüfung (nur lesend): Alex' echter Schlüssel kommt in KEINER versionierten/untracked Datei, in keinem Blob der
      bereinigten Historie (85 Blobs) und in nichts unter `dist/` vor. Ein Neu-Erzeugen ist daher keine Pflicht (Hygiene,
      siehe letzter Schritt).
- [x] Versionsnummern: `manifest.json` 1.2.0, `index.html` v1.2.0, Mac-App (`sender.py` + `.spec`) 1.2.0, `INSTALLATION.md` "Version 1.2", Bootscreen `Config::APP_VERSION` "v1.2" (4 Zeichen wie vorher "v1.0", Layout unverändert; war schon bei v1.1 nicht mitgezogen)
- [x] Firmware für die Veröffentlichung (Beispiel-Token `EXAMPLE-TOKEN-NOT-A-REAL-KEY-REPLACE-ME`) gebaut: Flash 1.036.677 Bytes
      (79,1 %), RAM 49.060 Bytes (15,0 %); `bootloader.bin`/`partitions.bin`/`firmware.bin` im Hauptordner (Web-Installer)
      ersetzt und `dist/Eiswolfs-iBoard-CYD-merged.bin` (`esptool merge-bin`, 0x1000/0x8000/0x10000, dio/40m/4MB). Echte
      `secrets.h` danach über Sicherung (`~/Eiswolfs-iBoard_secrets_BACKUP_<Datum>.h`, chmod 600, NICHT löschen) wiederhergestellt
      und per Prüfsumme bestätigt; Dev-Firmware mit echtem Schlüssel neu gebaut UND aufs iBoard geflasht (Bootscreen "v1.2").
- [x] Windows-Listener (`go build -trimpath -buildvcs=false`) + `EiswolfsIBoardListener-Setup.exe` (NSIS, inkl. `LICENSE`) gebaut.
- [x] Mac-App 1.2.0 (PyInstaller, ad-hoc signiert, Signatur gültig) + `Eiswolfs-iBoard.dmg` (aus `dmg_staging` inkl. `LICENSE`,
      `THIRD-PARTY-LICENSES.md`, `licenses/`) gebaut, in `/tmp` zusammengestellt, `mac/dmg_build/` unangetastet.
- [x] Durchsucht (auch Binär, PYZ der Mac-App, Inhalt des DMG): Name, Benutzername, private Pfade, echter Schlüssel: 0 Treffer in
      allen Release-Dateien und allen Repo-Dateien. (Komprimierte Inhalte von PDF/DMG/Setup sind per Rohsuche nicht lesbar: Quellen
      bzw. Inhalte wurden einzeln geprüft: Markdown, App-Bundle, entpackter DMG-Inhalt, Listener-exe.)
- [x] PDFs: `tools/build_guide_pdfs.py <Ordner>` (Python-Paket `markdown` im Wegwerf-venv + Chrome/Edge headless; interne
      Redaktionsnotiz wird entfernt, Foto-Platzhalter erscheinen als graue Kästen) -> `Eiswolfs-iBoard-Anleitung-DE.pdf` (13 Seiten),
      `Eiswolfs-iBoard-Guide-EN.pdf` (12 Seiten). ENTWURF, bis die Fotos eingesetzt sind; enthalten noch `[Generation]` und den
      `[OFFEN]`/`[TODO]`-Hinweis zum Schlüssel (Alex entschied: bleibt so, nur dokumentiert).
- [x] Alle Release-Dateien liegen in `dist/release-v1.2.0/` mit `SHA256SUMS.txt` (merged.bin, Web-Installer-Dateien, DMG, Setup.exe,
      FLASHEN.md, beide PDFs).
- [x] Schlüssel erneuert (15.11. abends, auf Alex' Bitte): neuer 48-Zeichen-Schlüssel (`secrets.token_hex(24)`, nie ausgegeben) in Mac-App-`config.json`
      (App vorher beendet, danach neu gestartet, Rechte 600, alle 17 Felder erhalten) und `cyd/src/secrets.h`; Firmware neu gebaut und aufs iBoard
      geflasht (MAC geprüft). Sicherungen mit dem ALTEN Stand: `~/Eiswolfs-iBoard_Mac-config_BACKUP_2026-11-15_190447.json` und
      `~/Eiswolfs-iBoard_secrets_vor_Schluesselwechsel_2026-11-15_190447.h` (chmod 600, enthalten den alten Schlüssel, nach erfolgreichem Test löschen).
      Der Lüfter-Hilfsdienst nutzt den Schlüssel nicht. Windows-Laptop am 15.11. erledigt, Alexa-Ausschalten funktioniert. LEHRE: die vom Mac bereitgestellte Datei enthält NUR `{"token": ...}`; sie
      NICHT über die Listener-`config.json` kopieren (verliert `device_name`, `port`, `monitor_*` -> Monitor-Rückschaltung weg, musste im
      Listener-Fenster "Einstellungen..." neu gesetzt werden), sondern dort "Aus Datei wählen..." nutzen; Doku (TOKEN.md, Anleitung)
      entsprechend korrigiert;
      bis dahin geht Herunterfahren per CYD/Alexa nicht (Aufwecken schon). Suche nach altem und neuem Schlüssel in Repo und Release-Dateien: 0 Treffer.

## Auftrag Release v1.2.0, Teil 1: Fortschritt (15.11., laufend)

Alles nur LOKAL: bauen und flashen ja, NICHT committen/taggen/pushen, kein Release, nichts auf GitHub ändern.
Flightradar, Toolbox, Solar Guardian, Silo unangetastet. Befehlsprotokoll unverändert. Bei "weiter": zuerst
diesen Abschnitt lesen und dort weitermachen, Erledigtes nicht wiederholen.

- [x] **Schritt 1 Sicherung**: `~/Eiswolfs-iBoard_VOR_v1.2.0_Teil1_2026-11-15_171532.tar.gz` (156 MB, ohne
      `cyd/.pio` und `mac/build`, gzip-Test ok, 7940 Einträge).
- [x] **Schritt 2 Lizenz MIT** (15.11.): `LICENSE` (MIT, "Copyright (c) Eiswolf-BG") angelegt, `LICENSE.md` gelöscht
      (nur noch EINE Lizenzdatei). Hinweise "Private use only"/"All rights reserved" ersetzt in README (Badge, Zeile 16,
      neuer Abschnitt "License"), `index.html`, `SECURITY.md`, `tools/fan-helper/README.md`, `THIRD-PARTY-LICENSES.md`;
      `windows/installer/installer.nsi` legt jetzt auch `LICENSE` ab (und entfernt es beim Deinstallieren). In Mac-App
      und Listener-Code stand kein Lizenz-/"Private use"-Text. `THIRD-PARTY-LICENSES.md` im Hauptordner ist die EINZIGE
      Quelle, ergänzt um: fauxmoESP (MIT), AsyncTCP (LGPL-3.0), ESP-IDF (Apache-2.0), Python-Laufzeit (PSF), Go-Stdlib,
      NSIS (zlib/libpng) und einen Abschnitt "What the LGPL parts mean for the MIT license"; neue Texte in `licenses/`
      (`fauxmoesp-MIT.txt`, `asynctcp-LGPL-3.0.txt`, `python-PSF-NOTICE.txt`, `nsis-zlib-NOTICE.txt`).
      `mac/sync_licenses.sh` kopiert LICENSE + THIRD-PARTY-LICENSES.md + `licenses/` nach `mac/dmg_staging/` (VOR jedem
      DMG-Bau ausführen, danach nach `mac/dmg_build/` kopieren) - kein doppelter Pflegeaufwand mehr.
      **Lizenz-Befund (an Alex gemeldet):** Streng sind nur AsyncTCP (LGPL-3.0, neu durch fauxmoESP im Standard-Build) und
      der ESP32-Arduino-Core (LGPL-2.1), beide statisch im Firmware-Binary; MIT für den eigenen Code bleibt möglich, weil
      der komplette Quelltext öffentlich ist und gegen die unveränderten Upstream-Bibliotheken gebaut wird (relinkbar);
      Lizenztexte liegen bei. PyInstaller (GPL-2.0+ mit Bootloader-Ausnahme) ist nur Build-Werkzeug. Nichts Unumkehrbares getan.
- [x] **Schritt 3 Alexa im öffentlichen Build** (15.11.): `ALEXA_TEST` entfernt, fauxmoESP immer einkompiliert, aber
      nur aktiv wenn `alexaActive` (aus `deviceCfg.alexaEnabled`, Standard AUS, wird in `setup()` einmal gesetzt):
      aus = kein Server, kein SSDP, kein Port 80, nichts im `loop()`. Neues Untermenü `runAlexaMenu()` (Einrichtung >
      "Alexa", Zeile 6 der Einrichtung zeigt gefüllt = an): "Alexa einschalten", "Name: <Name>" (Bildschirmtastatur,
      `sanitizeAlexaName()`: nur A-Z a-z 0-9 Leerzeichen Bindestrich, max. 24 Zeichen), der bisherige Schalter "Alexa:
      Ausschalten" (hierher verschoben, wirkt sofort), Hinweis "Danach in der Alexa-App: Geräte suchen", jeder Punkt mit
      "?". Änderung von Ein/Aus oder Name speichert sofort (config.json: `alexa_enabled`, `alexa_name`; NVS `alexa_en`,
      `alexa_nm`; fehlen die Felder: AUS/"Laptop"; `alexa_shutdown_enabled` bleibt unverändert, Alex' Gerät behält Name
      "Laptop" + Ausschalten EIN) und löst beim Verlassen des Menüs eine Meldung + `ESP.restart()` aus (Dienst wird in
      `setup()` entweder gar nicht oder komplett gestartet; kein Abbau zur Laufzeit möglich). NEUE StringIds `ALEXA_MENU_*`
      etc. in `i18n.h/.cpp` (DE+EN, Zeilen ≤33 Zeichen von Hand umbrochen); der alte `ALEXA_SHUTDOWN_HELP` hatte keine
      Umbrüche (wäre abgeschnitten worden) und wurde neu umbrochen. WICHTIG: Nach dem Update ist Alexa auf Alex' Gerät
      AUS, bis sie unter Einrichtung > Alexa eingeschaltet wird (das frühere Test-Build hatte sie immer an).
      Platzbedarf (`pio run`): vorher Flash 1.031.905 Bytes (78,7 %), RAM 49.028 Bytes (15,0 %); nachher Flash 1.036.581
      Bytes (79,1 %), RAM 49.060 Bytes (15,0 %), also +4.676 Bytes Flash, +32 Bytes RAM, keine neuen Warnungen. Kein
      dokumentiertes Limit überschritten.
- [x] **Schritt 4 Pfad-Maskierung**: Firmware: `cyd/trim_paths.py` (pre-script in `cyd/platformio.ini`, `-ffile-prefix-map`
      für PlatformIO-Core, Projektordner und Home -> `/pio`, `/project`, `/home`, Pfade werden zur Bauzeit ermittelt, nie
      im Repo). Windows-Listener: `go build -trimpath -buildvcs=false` (steht in `windows/README.md`/`docs/DEVELOPMENT.md`).
      Mac-App: PyInstaller legt im PYZ nur relative `co_filename` ab, geprüft. Prüfwerkzeug `tools/scan_release_files.py`
      (Suchbegriffe NICHT in Dateien: Benutzername/Name werden zur Laufzeit aus dem Systemkonto abgeleitet, Zusatzbegriffe
      über `SCAN_EXTRA`, Ausgabe nur Trefferzahlen). Ergebnis jetzt: Firmware-Build, `firmware.bin`/`bootloader.bin`/
      `partitions.bin`, Listener-exe und -Setup, Mac-App inkl. PYZ und DMG, alle Repo-Dateien: 0 Treffer. Die zwei
      "/Users/"-Treffer in der Firmware sind ein fremder CI-Pfad des vorkompilierten Espressif-Cores, nicht unserer.
- [x] **Schritt 5 Anleitungsquellen**: `docs/ANLEITUNG.de.md` und `docs/ANLEITUNG.en.md` (12 Kapitel, Fotoplatzhalter
      `[Foto: ...]`/`[Photo: ...]`, Schlüssel immer `<DEIN-SCHLUESSEL>`/`<YOUR-KEY>`, Echo Dot `[Generation]`); README mit
      Kurzfassung, Links und Alexa-Zeile. Das ist die EINZIGE Quelle, PDFs/Kopien erst in Teil 2. OFFEN markiert
      (`[OFFEN: ...]`/`[TODO: ...]`): die vorgebaute Firmware hat keinen echten Schlüssel (Beispiel-Token in
      `secrets.h` beim Bauen der `merged.bin`), am Gerät lässt sich keiner eintragen - Entscheidung für Teil 2.
- [x] Schritt 6 Bauen/Flashen (15.11.): `pio run` ohne neue Warnungen (Flash 79,1 %, RAM 15,0 %), geflasht auf
      `/dev/cu.wchusbserial31410` (MAC per esptool bestätigt, das ist das iBoard), Hash verifiziert, Start läuft normal,
      kein Alexa-Log (Alexa ist AUS). Flightradar/Toolbox/Solar Guardian/Silo nur gelesen, unverändert (letzte Änderungen
      13.11. bzw. Silo vor diesem Auftrag).
- [x] **Live-Test 15.11. abends (Alex)**: Alexa-Menü erreichbar, Texte ok (Fotos); Alexa AN, Name "Laptop"->"Rechner"
      geändert, "Alexa, suche neue Geräte" findet "Rechner" (Sprache; Suche per App-Menü führt nur zur Herstellerliste),
      "schalte Rechner an" löst WoL + HDMI-Umschaltung aus, "aus" fährt den Laptop herunter. WICHTIG: fauxmoESP 3.4.1
      emuliert eine Philips-HUE-Lampe (`/description.xml`, `/api/<user>/lights`), KEINE WeMo-Steckdose; Anleitung und
      Menütexte entsprechend korrigiert. Die Kennung der Lampe kommt aus der MAC und bleibt nach Namensänderung gleich.
      Frühere "Alexa findet nichts"-Meldungen kamen von (a) schon bekanntem Gerät und (b) einem überlasteten Netgear-AP
      (WNR2000v5, ~35 Geräte an LAN 3 mit 100 Mbit/s), nicht von der Firmware; iBoard liegt jetzt im FritzBox-2,4-GHz-Netz.
- [x] **Tastatur (15.11., Wunsch Alex)** in `setup_flow.cpp::runKeyboard()`: erster Buchstabe gross, danach automatisch klein
      (`shiftAuto`/`afterLetter()`; von Hand getippte Umschaltung bleibt Dauer-Gross; leeres Feld nach Rücktaste = wieder
      Anfangszustand; vorbelegter Text = klein), Passwort startet im Klartext (`showPassword = true`, "Verbergen" schaltet auf
      Sterne). Betrifft alle Texteingaben des iBoards (WLAN, WoL-Name, Alexa-Name). Anleitungen angepasst.
- [x] **Alle Live-Tests bestanden (15.11. abends, Alex)**: Alexa an und aus per Sprache (neuer Schlüssel, Windows-Listener neu eingestellt), HDMI-Umschaltung
      in beide Richtungen sofort, Touch/Monitor wie vorher. Teil 1 und Teil 2 sind damit abgeschlossen; offen bleibt Teil 3 (Veröffentlichen) nach
      Alex' ausdrücklichem OK, sowie Fotos/`[Generation]` für die PDFs.
- [x] (alter Punkt, erledigt) **Live-Tests**: a) Alexa-Menü erreichbar (Menü > Einrichtung > Alexa), Standard AUS. b) Alexa EIN,
      "Alexa, suche neue Geräte" findet "Laptop". c) Name ändern, neu suchen. d) "an"/"aus" wie vorher (aus nur mit
      "Alexa: Ausschalten" EIN). e) Touch und Monitor-Umschaltung unverändert. Teil 2 (Schlüssel neu erzeugen, alle
      Release-Dateien bauen, durchsuchen, PDFs) und Teil 3 (Veröffentlichen) erst nach Alex' OK.

### Bauablauf Release v1.2.0 (wiederholbar, ohne persönliche Pfade/Daten)

1. Firmware: `cd cyd && pio run` (trim_paths.py maskiert die Pfade automatisch). Mit dem EIGENEN `secrets.h` bauen
   und flashen; für die öffentliche `merged.bin` `secrets.h` kurz durch ein Beispiel-Token ersetzen, bauen,
   `esptool.py merge_bin --flash_mode dio --flash_freq 40m --flash_size 4MB -o ... 0x1000 bootloader.bin 0x8000
   partitions.bin 0x10000 firmware.bin`, danach SOFORT die echte `secrets.h` zurück und neu bauen.
2. Windows-Listener: `cd windows/listener && GOOS=windows GOARCH=amd64 go build -trimpath -buildvcs=false -ldflags
   "-H=windowsgui -s -w" -o ../installer/EiswolfsIBoardListener.exe .`, dann `cd ../installer && makensis installer.nsi`.
3. Mac-App: `cd mac && ./sync_licenses.sh && ./venv/bin/pyinstaller eiswolfs_iboard.spec --noconfirm`, ad-hoc signieren
   (`codesign --force --deep --sign - ...`), DMG aus `dmg_staging` bauen (siehe "Installierbare App > Bauen").
4. PRÜFEN vor dem Hochladen: `./mac/venv/bin/python tools/scan_release_files.py <alle Release-Dateien> "mac/dist/Eiswolfs iBoard.app"`
   (muss "Treffer gesamt: 0" melden, Exit-Code 0; mit `SCAN_EXTRA="privateadresse"` die private E-Mail zusätzlich suchen).

## Auftrag Alexa Aus: Fortschritt (15.11., laufend)

"Alexa, schalte Laptop aus" fährt den Laptop jetzt wirklich herunter -
OHNE Countdown/Rückfrage (Alex' ausdrücklicher Wunsch, ein früherer
Entwurf mit 30s-Countdown wurde verworfen). Nur CYD-Firmware geändert,
Mac-App/Windows-Listener/Protokoll unverändert. NICHT committen/
taggen/pushen.

### Schritt 1: Sofortiger Shutdown über denselben Weg wie Halten (abgeschlossen)

- Neues Merker-Flag `alexaShutdownRequested` (gleiches Muster wie
  `alexaWolRequested`) - `fauxmo.onSetState()`-Callback setzt bei
  `state==false` NUR dieses Flag (AsyncTCP-Kontext, nichts Schweres).
- `loop()` (direkt nach dem bestehenden `alexaOffRevertAtMs`-Block):
  prüft der Reihe nach `deviceCfg.alexaShutdownEnabled` (aus =
  weiterhin nur Logzeile "Befehl AUS (Test, keine Aktion)", wie
  bisher), `wolDeviceCount`, `wolDevices[0].shutdownIp` (leer -> Log +
  kurze Anzeige "IP nötig zum Herunterfahren", wiederverwendet
  `showDeviceNotSetUp()`/dieselbe Darstellung wie beim Touch-Pfad,
  falls Controls/"Keine Verbindung" gerade sichtbar ist), die
  bestehende 3s-Sperre `deviceTapLockedUntilMs[0]`, und ruft bei Erfolg
  **exakt dieselbe Funktion wie beim langen Antippen**,
  `sendDeviceShutdown(0)` - keine zweite Kopie der Shutdown-Logik.
  Bei Erfolg kurzes rotes Feedback auf der Controls-Kachel bzw. dem
  "Keine Verbindung"-Button (falls gerade sichtbar), via
  `startDeviceFeedback()` (dieselbe Funktion wie beim Touch-Halten).
- **Kein eigener `fauxmo.setState()`-Rücksprung nötig** (anders als bei
  "an"): fauxmoESP meldet Alexa "aus" bereits VOR diesem Callback -
  passt so oder so zum Ergebnis (nichts getan ODER tatsächlich
  heruntergefahren), kein "Gerät reagiert nicht" möglich.
- Zweimal "aus" kurz hintereinander: die 3s-Sperre fängt das exakt wie
  beim Touch-Pfad ab (zweiter Aufruf loggt "Sperre aktiv, ignoriert").

### Schritt 2: Schalter im Menü (abgeschlossen)

- `Storage::DeviceConfig.alexaShutdownEnabled` (neues `bool`, additiv,
  Standard `false` - sicher für GitHub-Nutzer, die das nie bewusst
  eingeschaltet haben). JSON-Feld `"alexa_shutdown_enabled"`, NVS-Key
  `"alexa_sdn"` - fehlt das Feld in einer älteren config.json/NVS,
  bleibt der bisherige reine-Logzeile-Zustand erhalten.
- `runSetupMenu()` bekommt eine 6. Zeile "Alexa: Ausschalten" (gefüllte
  Akzentfarbe = an, Standard aus) mit "?"-Hilfe-Knopf (gleiches Muster
  wie die anderen 5 Einträge dort) - Tippen schaltet sofort um und
  speichert (`Storage::save()`), kein Bestätigungsdialog nötig (reiner
  Lokal-Schalter, keine Netzwerk-Aktion). Zeilenhöhe/-abstand im
  gesamten Menü leicht verkleinert (28px/32px-Schritt -> 24px/28px-
  Schritt), damit die neue 6. Zeile + Zurück-Taste weiterhin auf den
  240px-Bildschirm passen.

### Schritt 3: Randfälle (geprüft)

- Mac im Schlafzustand/Display dunkel: bewusst NICHTS Neues eingebaut -
  die Feedback-Anzeige greift nur, wenn `currentPage==Controls` oder
  `showingNoConnection` gerade sichtbar sind; im dunklen Ruhezustand
  ist keins von beidem der Fall (Display zeigt dort nichts, siehe
  bestehende Schlaf-Logik), es passiert dadurch automatisch NUR die
  Logzeile, kein ungewolltes Aufwecken/Zeichnen.
- WLAN weg: `sendDeviceShutdown()` prüft `WiFi.status()==WL_CONNECTED`
  bereits selbst (unverändert) und gibt `false` zurück - kein Absturz,
  Logzeile "Shutdown konnte nicht gesendet werden (kein WLAN)".
- Touch-Halten-Pfad UND Monitor-Rückschaltung unverändert (ruft
  weiterhin exakt dieselbe `sendDeviceShutdown()`, siehe Regressionstest
  unten).

### Schritt 4: Bauen/Flashen (abgeschlossen); Test mit Alex - OFFEN

- `pio run`: **keine neuen Warnungen** (nur die bereits vorher
  bestehenden ArduinoJson-Deprecation-Hinweise, unverändert zur
  Codebasis vor diesem Auftrag). Flash **1.031.905 Bytes (78,7%,
  +1.956 Bytes)**, RAM **49.028 Bytes (15,0%, +8 Bytes)**.
- MAC vor dem Flashen bestätigt (`f8:b3:b7:74:7f:fc`, Port 31410),
  Mac-App-Verbindung laut `lsof` sofort (<1s) zurück, Boot-Log
  fehlerfrei (SD-Karte, WoL-Gerät, Alexa-Emulation nach 8,36s).
- **Regressionstest selbstständig durchgeführt** (Alexa-HTTP-
  Simulation, Schalter noch auf Standard AUS): "aus" -> weiterhin
  NUR "Befehl AUS (Test, keine Aktion)", keine Aktion. "an" -> weiterhin
  Monitor-Umschaltung + WoL wie zuvor (Log: "Monitor: Befehl 2
  gesendet..." / "WoL angefordert..." / "Quittung nach 42ms") -
  bestätigt: bestehendes Verhalten unverändert, solange der neue
  Schalter aus bleibt.
- **Von Alex bestätigt (15.11.): "funktioniert hervorragend!"** -
  Schalter auf EIN gestellt, "Alexa, schalte Laptop aus" hat den
  echten Shutdown samt Monitor-Rückschaltung ausgelöst. Auftrag damit
  abgeschlossen.
- Flightradar/Solar-Guardian-CYD per `git status` bestätigt unverändert.

### WIE WEITER (Live-Test mit Alex)

1. Menü → Einrichtung → ganz unten "Alexa: Ausschalten" antippen (wird
   grün/gefüllt).
2. Laptop läuft, Monitor auf HDMI 2. "Alexa, schalte Laptop aus" sagen.
3. Erwartet: Monitor wechselt auf HDMI 1 (Mac-Bild), Laptop fährt
   herunter, Alexa meldet "Okay" (kein "Gerät reagiert nicht").
4. Schalter wieder auf AUS stellen: "aus" sollte wieder nur loggen,
   nichts am Laptop tun.
5. Touch-Halten auf dem Acer-Button prüfen: weiterhin unverändert.
6. "Alexa, schalte Laptop an" prüfen: weiterhin unverändert.

Nach erfolgreichem Test: Ergebnis hier nachtragen, Anleitung (DE/EN,
Stick EISWOLF) um den neuen Menüpunkt ergänzen. NICHT committen/
taggen/pushen ohne expliziten Wunsch.

## Auftrag Listener-Fenster: Fortschritt (15.11., laufend)

Bug im Windows-Listener-Einrichtungsfenster (seit Auftrag "Monitor-
Umschaltung"): Monitor-Felder unsichtbar, Fenster unten abgeschnitten,
Schlüssel im Klartext. NICHT committen/taggen/pushen.

### Schritt 1: Ursache (bestätigt per Foto von Alex)

- **Layout abgeschnitten**: `installer.nsi` nutzt KEIN MUI (`!include
  "MUI2.nsh"` fehlt komplett, reines `Name`/`OutFile`/`Page custom`) -
  das klassische NSIS-Dialogfenster hat eine FESTE, kleine Innenhöhe.
  Die bisherige einzelne Seite (`SettingsPageShow`) zeichnete Inhalte
  bis 146u (Name/Port/Schlüssel/Autostart/Hinweistext), die drei neuen
  Monitor-Felder kamen ab 150u dazu - beides lag faktisch ausserhalb
  des sichtbaren Fensters (Alex' Foto bestätigt: schon der alte
  Hinweistext ist unten abgeschnitten, die Monitor-Felder fehlen
  komplett). Kein Skalierungs-/DPI-Bug im engeren Sinn - die Seite war
  schlicht zu lang fürs Fenster, bei JEDER Windows-Skalierung.
- **Klartext-Schlüssel**: `SettingsPageShow`, Zeile mit
  `${NSD_CreateText} 0 67u 64% 12u $ExistingToken` - das Textfeld wurde
  direkt mit dem gespeicherten Schlüssel vorbelegt. Der Hinweistext
  darunter ("leer lassen, um ihn zu behalten") war korrekt, aber das
  Feld selbst zeigte den Wert trotzdem an.
- **"Einstellungen..."** (Taskleiste UND Startmenü) ruft laut
  `main.go::openSettings()` dieselbe im Programmordner liegende
  `Setup.exe /SETTINGS` auf - also denselben (fehlerhaften) Dialog, kein
  separater Code.

### Schritt 2: Layout behoben

- Seite in ZWEI `Page custom`-Seiten aufgeteilt: `SettingsPage1Show/
  Leave` (Name/Port/Schlüssel/Autostart/Hinweis, endet jetzt bei 125u)
  und `SettingsPage2Show/Leave` (Monitor-Umschaltung, endet bei 129u) -
  beide mit deutlichem Sicherheitsabstand zur beobachteten
  Abschneidegrenze (~140-146u).
- Monitor-Seite: Checkbox, Dropdown "Eingang" (HDMI1/HDMI2/DP1/DP2/
  "Anderer Wert", füllt das Rohwert-Feld automatisch,
  `OnMonitorInputChoiceChange`), Textfeld "VCP-Rohwert" (frei änderbar,
  auch bei "Anderer Wert"), editierbare ComboBox "Monitor" - vorbelegt
  mit den tatsächlich erkannten Monitoren.
- **Monitor-Erkennung im Installer**: `windows/listener/main.go` bekommt
  einen neuen, verdeckten Aufruf `<exe> --list-monitors` (ganz am Anfang
  von `main()`, vor jeder Log-/Taskleisten-/Sperren-Logik - reine
  Lese-Funktion, keine Nebenwirkung auf eine evtl. laufende normale
  Instanz) - gibt jede per `EnumDisplayMonitors`/
  `GetPhysicalMonitorsFromHMONITOR` gefundene Monitor-Beschreibung
  zeilenweise aus (`listAllMonitorDescriptions()`, Schwester-Funktion
  zu `findMonitorHandle()`, die nur den ERSTEN Treffer sucht).
  `installer.nsi::DetectMonitors()` extrahiert dafür die im Installer
  eingebettete exe einmalig nach `$PLUGINSDIR\monitorlist.exe`
  (funktioniert so sowohl bei einer Neuinstallation, wo `$INSTDIR` noch
  leer ist, als auch bei einem Update) und liest die Ausgabe per
  `nsExec::ExecToStack`. Kein Monitor gefunden -> eigener Hinweistext,
  das Namensfeld bleibt frei editierbar (Rückfallebene).
- "Einstellungen..." zeigt dieselben zwei neuen Seiten mit den
  gespeicherten Werten vorbelegt (Monitor-Seite komplett, Seite 1 ohne
  den Schlüssel selbst, siehe Schritt 3) - kein separater Code nötig,
  da beide Aufrufer dieselbe `Setup.exe /SETTINGS` nutzen.
- Update einer bestehenden Installation übernimmt weiterhin alle Werte
  (`ReadExistingConfig`, unverändert), neue Monitor-Felder stehen ohne
  vorhandene Konfiguration auf AUS/leer.

### Schritt 3: Schlüssel nie im Klartext (behoben)

`${NSD_CreateText} 0 67u 64% 12u $ExistingToken` -> `... ""` - das
Schlüsselfeld wird jetzt IMMER leer angezeigt, unabhängig davon, ob/
welcher Schlüssel gespeichert ist. Der Hinweistext (Zeichenzahl) bleibt
wie bisher. `SettingsPage1Leave` behält die bestehende Regel
unverändert bei (leeres Feld beim Verlassen der Seite = bestehenden
Schlüssel NICHT überschreiben) - das war schon vorher korrekt
implementiert, nur die Anzeige selbst war das Leck. "Aus Datei
wählen..." und manuelles Eintippen funktionieren unverändert. Schlüssel
steht weiterhin nicht im Log und nicht in der `.exe` (nur in
`%APPDATA%\EiswolfsIBoard\config.json`, 0600-Dateirechte, unverändert).

### Schritt 4: Zwischenweg (an Alex gegeben, siehe Chat)

Manuelles Ergänzen von drei Zeilen in
`%APPDATA%\EiswolfsIBoard\config.json` (`monitor_switch_enabled`,
`monitor_vcp_value`, `monitor_screen_name_substring`) + "Token neu
laden" über das Taskleisten-Symbol - KEIN Neustart/Installation nötig.
Text dafür auch in `Anleitung.txt` auf dem Stick EISWOLF ergänzt.

### Schritt 5: Bauen/Stick (abgeschlossen); Live-Test mit Alex - OFFEN

- Go-Listener neu gebaut (`--list-monitors`-Erweiterung), NSIS-Installer
  neu kompiliert: **0 Warnungen/Fehler** bei `makensis` (zweimal
  geprüft, auch nach dem Layout-Feintuning). `strings` auf beiden
  `.exe`s: weiterhin 0 Treffer für `/Users/`-Pfade.
  `EiswolfsIBoardListener-Setup-2026-11-15.exe`,
  SHA-256 `ae9351ab103086c9154e8ceb2ca82a9f2dff46b1ce01c69450e1aa2f28ed8d30`.
  Auf dem Stick EISWOLF ersetzt (alte `...-2026-11-14.exe` gelöscht),
  `token-fuer-windows.json`/`DDC-Test/` unberührt.
- **Seitenhöhen durchgerechnet** (siehe Schritt 2: 125u/129u, deutlich
  unter den beobachteten ~140-146u) - das ist das Einzige, was ohne
  Windows-Rechner wirklich prüfbar war. **Tatsächliches Rendering bei
  100/125/150% Skalierung NICHT selbst sichtbar** - NSIS-Klassik-Dialoge
  sind nicht DPI-manifest-deklariert, Windows skaliert das ganze Fenster
  bei abweichender Textgrösse per Bitmap-Stretching (gleiches
  Seitenverhältnis bleibt erhalten) - sollte dadurch bei 125/150% nicht
  schlechter aussehen als bei 100%, aber das ist eine Schlussfolgerung,
  keine Beobachtung. Braucht Alex' Fotos zur Bestätigung.
- Flightradar (`git status`: clean) und Solar-Guardian-CYD (clean)
  bestätigt unverändert.
- **Live-Tests a-f mit Alex: NOCH OFFEN** - neue `setup.exe` ausführen,
  beide Seiten ansehen/fotografieren, Einstellungen speichern, Ausschalten
  testen, "Einstellungen..." erneut öffnen (Schlüssel darf nicht
  erscheinen), Skalierung 125% bestätigen.

### Nachtrag: "Beide Dropdownmenüs sind leer" (Alex' Rückmeldung nach dem ersten Test)

**Ursache**: Ein Win32-`DropList` (das "Eingang"-Feld) zeigt OHNE eine
AUSDRÜCKLICH gesetzte Auswahl grundsätzlich GAR NICHTS an, auch wenn
die 5 Einträge korrekt vorhanden sind (anders als bei einer normalen
`ComboBox` mit Eingabefeld) - der Code rief nie `NSD_CB_SelectString`
auf, das Feld war dadurch IMMER leer dargestellt, unabhängig von
vorhandenen Werten. Für die "Monitor"-`ComboBox` zusätzlich defensiv
den Text nach dem Erzeugen nochmal explizit gesetzt (`NSD_SetText`),
falls die Vorbelegung beim Erzeugen selbst aus irgendeinem Grund nicht
griff.

**Behoben**: Eingang-Dropdown wählt jetzt beim Öffnen automatisch den
zum vorhandenen Rohwert passenden Eintrag aus (4369/4626/3855/4112),
sonst "Anderer Wert". Neu gebaut, geprüft (`makensis`: 0 Warnungen,
`strings`: 0 Pfad-Treffer), SHA-256
`7c55ee6f7255a2805b813c1d46938c52e480f26f4f530b47661cf7fad1872550`,
als `EiswolfsIBoardListener-Setup-2026-11-15b.exe` auf den Stick gelegt
(alte `...-2026-11-15.exe` entfernt), `Anleitung.txt` entsprechend
aktualisiert (neuer Dateiname + Prüfsumme + kurzer Hinweis auf den
Nachfix). Noch NICHT von Alex erneut getestet.

### Nachtrag 2: Dropdown "flackert nur" beim Anklicken (Alex' zweite Rückmeldung)

**Echte Ursache gefunden**: `${NSD_CreateDropList}`/`${NSD_CreateComboBox}`
brauchen als Höhenparameter NICHT nur die geschlossene Zeile, sondern
auch Platz für den AUFGEKLAPPTEN Zustand mit allen Einträgen - mit nur
13u (wie zuvor gesetzt) öffnet Windows die Liste und klappt sie sofort
wieder zu, weil kein Raum dafür reserviert war (das von Alex
beschriebene "Flackern"). Behoben: Höhe auf 70u (Eingang-Dropdown,
5 Einträge) bzw. 60u (Monitor-ComboBox) angehoben - ändert NICHTS an
der sichtbaren Größe der GESCHLOSSENEN Box, nur am Platz für die Liste.

**Nebenbefund (kein Bug, nur Datenverlust als Folge des ersten Bugs)**:
Beim allerersten Testlauf (vor jedem Fix) waren die Dropdowns nicht
bedienbar - Alex hat trotzdem einmal auf "Install" geklickt, wodurch
`SettingsPage2Leave`s Leerwert-Fallback (`$NewMonitorVcpValue==""` ->
`"0"`) die vorher per Zwischenweg (Auftrag Schritt 4) eingetragenen
Werte (4369/AW2720HFA) in der `config.json` überschrieben hat - sichtbar
als VCP-Rohwert "0" und leeres Monitor-Feld beim zweiten Öffnen. Kein
Datenverlust-Bug im eigentlichen Sinn, nur eine Folge des Dropdown-Bugs
- behoben, sobald das Dropdown bedienbar ist und die Werte einmal
bewusst neu gesetzt werden (siehe aktualisierte `Anleitung.txt`).

Neu gebaut (`makensis`: 0 Warnungen, `strings`: 0 Pfad-Treffer), SHA-256
`6311a3f2b332eec3eee3845e48128854d2369c789436b4c69bbb85a2abb7ef64`, als
`EiswolfsIBoardListener-Setup-2026-11-15c.exe` auf den Stick gelegt
(ältere `...-15b.exe` entfernt), `Anleitung.txt` aktualisiert (neuer
Dateiname, neue Prüfsumme, Hinweis auf den Datenverlust-Nebenbefund).

**Von Alex bestätigt (15.11.): Dropdown-Fix funktioniert** ("jetzt
gehts") - beide Dropdowns lassen sich jetzt bedienen, Werte wurden
gesetzt.

**Von Alex bestätigt (15.11.): End-to-End-Funktionstest erfolgreich**
("laptop über cyd ausgeschaltet und hdmi 1 wurde gewählt! Super!") -
der komplette Ablauf (CYD-Ausschalten per Halten → Windows-Listener
schaltet den Monitor per DDC/CI auf HDMI 1 zurück → Laptop fährt
herunter) funktioniert damit nachweislich auf echter Hardware. Der
zentrale Zweck dieses und des vorangegangenen Auftrags
("Monitor-Umschaltung") ist damit erreicht. Übrige Live-Tests (a/c/e/f,
Skalierungsfotos) aus dem ursprünglichen Auftrag optional, nicht mehr
blockierend.

### Dokumentation

- `tools/ddc-test/windows-scripts/ANLEITUNG.txt`: NICHT betroffen (reine
  DDC-Testskripte, separat vom Listener-Installer).
- `Anleitung.txt` auf dem Stick EISWOLF: aktualisiert (neue Seite-1/
  Seite-2-Beschreibung, neuer Dateiname, neue Prüfsumme, Zwischenweg-
  Zeilen ergänzt). Nur Deutsch (war schon immer nur Deutsch, rein für
  Alex - kein EN-Gegenstück vorhanden; `docs/WOL_SETUP.md`, DE+EN, ist
  die oeffentliche Variante und bleibt für diesen Auftrag unverändert,
  da sie den Dialogaufbau nicht im Detail beschreibt).

## Auftrag Monitor-Umschaltung: Fortschritt (14.11., laufend)

Koppelt den Monitor-Eingangswechsel an das CYD-Ein-/Ausschalten des
Laptops (Ergebnis des Machbarkeitstests oben). NICHT committen/taggen/
pushen.

### Schritt 1: Entwurf (abgeschlossen)

**Neuer Befehl "MONITOR n" (CYD -> Mac-App)**, exakt im Muster von
`SLEEP`/`FAN BOOST` (`CmdAuth::signCommand()`, Versand ueber
`Config::FAN_CMD_UDP_PORT`=4211 per UDP-Unicast an die gelernte Mac-IP
ODER per USB-Zeile, je nach `usbIsActiveSource()` - siehe
`sendSleepCommand()`/`sendFanCommand()` in `main.cpp` als Vorbild).
`n` ist der LOGISCHE Eingang (1=HDMI1, 2=HDMI2, 3=DisplayPort1,
4=DisplayPort2) - die Zuordnung zu den echten VCP-0x60-Rohwerten (siehe
Machbarkeitstest: 4369/4626/vermutlich 3855/... fuer dieses Modell)
liegt NUR in der Mac-App-Konfiguration, NICHT im CYD/Protokoll - andere
Monitore koennten andere Rohwerte brauchen.

**Quittung**: neues, additives Feld `"monitor_ack"` im normalen
JSON-Datenpaket der Mac-App (gleiches Muster wie `"sleep_ack"` -
ein bei jedem verarbeiteten MONITOR-Befehl hochgezaehlter Zaehler,
NICHT nur ein Vorhanden-Flag, damit das CYD einen soeben gesendeten
Befehl von einem alten unterscheiden kann).

**CYD-Ablauf (neue zentrale Funktion `triggerWolForDevice(deviceIdx)`,
ersetzt die bisherigen beiden direkten `sendWolPacket()`-Aufrufe in
`handleDeviceWidgets()` [Controls-Kachel UND "Keine Verbindung"-Button,
beide laufen durch dieselbe Funktion] UND im Alexa-Flow in `loop()`,
siehe Auftrag "Alexa-Test 2")**:
- `wolDevices[i].monitorInputOnPowerOn == 0` (Standard/"Aus") ODER Mac
  nicht erreichbar (`computeLastAnyPacketMs()` aelter als
  `Config::NO_DATA_TIMEOUT_MS`, deckt "schlaeft" UND "App aus"
  gleichermassen ab) -> **sofort** `sendWolPacket()`, keine Verzoegerung,
  Log "Monitor: Mac nicht erreichbar, nur WoL" (nur wenn das Feld
  eigentlich gesetzt waere, sonst kein Log - Normalfall "Aus" bleibt
  stumm).
- Sonst: MONITOR-Befehl senden, EIGENER nicht-blockierender
  Zustandsautomat (`MonitorPreWolState::WaitingForAck`, kein `delay()`)
  wartet in `loop()` auf ein verändertes `monitor_ack` (Momentaufnahme
  beim Senden, exakt wie `macSleepBaselineLastPacketMs`-Muster) bis zu
  `MONITOR_ACK_TIMEOUT_MS`=2000ms, sendet DANACH (Erfolg oder Timeout)
  das WoL-Paket. Alexa bekommt ihre HTTP-Antwort weiterhin sofort (der
  Zustandsautomat laeuft unabhaengig vom `fauxmo`-Callback, der nur noch
  das Merker-Flag setzt, siehe Auftrag "Alexa-Test 2").
- Die bestehende 3s-Sperre pro Geraet (`deviceTapLockedUntilMs[]`)
  bleibt UNVERAENDERT am Anfang (Tap-Zeitpunkt) - sie verhindert
  Doppel-Ausloesung, nicht die Monitor-Wartezeit selbst.

**Mac-App-Ablauf**: `CydCommandListener._dispatch()` bekommt einen
neuen Zweig `message.startswith("MONITOR ")` (Wert 1-4, sonst
ignoriert), ruft `sender.bump_monitor_ack()` + `sender.send_now()`
SOFORT auf (Quittung "angekommen", nicht "DDC sicher gelungen" - exakt
wie bei SLEEP), startet danach den eigentlichen DDC-Schreibvorgang in
einem EIGENEN Thread (Timeout 3s + 1 Wiederholung, blockiert nichts).
Eigene Ratenbegrenzung (hoechstens 1 Monitor-Befehl alle 3s,
UNABHAENGIG von der CYD-Sperre - Verteidigung in der Tiefe, da Alexa
denselben Befehl manchmal doppelt zustellt).

**Windows-Listener-Ablauf (Schritt 4)**: Beim empfangenen,
signaturgeprueften `SHUTDOWN`-Befehl setzt `runShutdown()`
(`windows/listener/main.go`, ~Zeile 281) VOR dem eigentlichen
`exec.Command("shutdown", ...)` per `dxva2.dll`
(`GetVCPFeatureAndVCPFeatureReply`/`SetVCPFeature`, siehe
`tools/ddc-test/windows-scripts/ddc-umschalten.ps1` als Vorlage, jetzt
aber in Go statt PowerShell) den konfigurierten Eingang, wartet ~1,5s
und faehrt DANACH wie bisher herunter - auch wenn DDC fehlschlaegt
oder laenger als 3s braucht (Log, kein Blockieren des Shutdowns).

**Sicherheitsgrenzen** (wie im Auftrag verlangt): Mac-App schreibt
NUR VCP 0x60 und NUR die 4 erlaubten logischen Werte, NUR an den in
den Einstellungen explizit ausgewaehlten Bildschirm (per UUID
gespeichert, `m1ddc display <uuid> ...` statt Index) - ist keiner
ausgewaehlt/gefunden, passiert NICHTS (kein Fallback auf "alle
Bildschirme"). Alex' zweiter Mac-Monitor (U28E590) wird dadurch
strukturell nie angefasst.

**Migration/Abwaertskompatibilitaet**: `WolDevice.monitorInputOnPowerOn`
(neues `uint8_t`-Feld, additiv wie `shutdownIp`/`legacyBroadcastShutdown`)
Default 0 ("Aus") - fehlt es in einer aelteren `config.json`/NVS, bleibt
der bisherige WoL-Ablauf 1:1 erhalten. Keine Breaking Changes am
bestehenden Protokoll (HMAC/Zaehler/Ports/alle bisherigen Befehle
unveraendert) - eine aeltere Mac-App ohne MONITOR-Unterstuetzung
ignoriert den neuen Befehl einfach (fehlender Treffer in `_dispatch()`),
das CYD wartet dann die vollen 2s auf eine Quittung, die nie kommt, und
sendet danach trotzdem WoL - 2s zusaetzliche Verzoegerung als einzige
Nebenwirkung einer alten Mac-App, kein Fehlerfall.

**Keine Rueckfrage an Alex noetig** - der Entwurf ist additiv/
abwaertskompatibel, keine bestehende Einrichtung wird gefaehrdet.

### Schritt 2: Mac-App (abgeschlossen, siehe `mac/`)

- `monitor_switch.py` (neu): `MonitorSwitcher` - `request(logical_input)`
  (normaler CYD-Betrieb, einmaliges Umschalten, KEIN Auto-Revert) und
  `test_switch_to_hdmi2()` (Menue-Test-Knopf, liest den Eingang EINMAL
  VORHER, schaltet, nach 15s automatisch zurueck). `_m1ddc_path()`
  nutzt dasselbe `Contents/Resources/<name>`-Aufloesungsmuster wie
  `sender.py::resource_path()` (hier dupliziert, um keinen Kreis-Import
  zu erzeugen). `m1ddc`-Binary + Lizenz liegen in
  `mac/bundle_resources/` (Binary selbst `.gitignore`t wie `macmon`),
  im `.spec` als `binaries=[...]` eingebettet (ad-hoc mitsigniert durch
  den bestehenden `codesign --deep`-Schritt).
- `config.py`: additiv `monitor_switch_enabled` (Standard `False`),
  `monitor_switch_screen_uuid`/`_name`, `monitor_vcp_1..4` (vorbelegt
  mit den beim Machbarkeitstest bestaetigten Dell-Werten 4369/4626/3855/
  4112 - letzterer ungetestet/generisch).
  `get_monitor_switch_screen_uuid()` leer = "nichts tun" (Sicherheits-
  grenze, kein Fallback).
- `fan_boost.py`: `CydCommandListener` bekommt `on_monitor`-Callback +
  eigene 3s-Ratenbegrenzung (`MONITOR_CMD_MIN_INTERVAL_S`), neuer
  Dispatch-Zweig `message.startswith("MONITOR ")` (nur Werte 1-4).
- `sender.py`: `monitor_ack_counter`/`bump_monitor_ack()` (Muster wie
  `sleep_ack`), neues Feld `"monitor_ack"` im Datenpaket,
  `_on_cyd_monitor()` (Quittung sofort, DDC asynchron danach ueber
  `self.monitor_switch.request()`). Neues Menue "Monitor-Umschaltung…":
  Ein/Aus-Haken, Untermenue "Bildschirm" (per `m1ddc display list`
  erkannte Bildschirme, UUID-basiert, "neu suchen"), Untermenue
  "Eingangswerte (VCP 0x60)" (4 Eintraege, `rumps.Window`-Dialog zum
  Bearbeiten), "Test: auf HDMI 2 schalten".
- `i18n.py`: neue DE/EN-Texte fuer alle obigen Menuepunkte/Dialoge.
- **Gefundener und behobener Bug** (erst beim echten App-Start
  aufgefallen, nicht beim reinen Syntax-Check): `rumps.MenuItem.clear()`
  wirft `AttributeError: 'NoneType' object has no attribute
  'removeAllItems'`, wenn das Untermenue noch NIE etwas enthalten hat
  (rumps legt das interne `NSMenu` erst lazy beim ERSTEN `.add()` an,
  siehe `rumps.py::MenuItem.__setitem__`) - `_rebuild_monitor_screen_menu()`
  rief `.clear()` aber schon beim allerersten Aufbau auf. Behoben mit
  `try/except AttributeError` um den `.clear()`-Aufruf. Die App ist
  beim ersten Testlauf dadurch sofort nach dem Start abgestuerzt -
  durch Start mit `open -W --stderr <datei>` sofort gefunden und
  gefixt, zweiter Testlauf lief fehlerfrei.
- Version `1.1.3` -> `1.1.4` (`eiswolfs_iboard.spec` + `sender.py`).

### Schritt 3: CYD-Firmware (abgeschlossen, siehe `cyd/src/`)

- `storage.h`: `WolDevice.monitorInputOnPowerOn` (additiv, Default 0).
- `storage.cpp`: JSON-Feld `"monitor_input"` + NVS-Key `wol%u_min` -
  fehlt das Feld (aeltere config.json/NVS), bleibt 0 ("Aus"), bestehende
  Geraete bleiben dadurch unveraendert funktionsfaehig.
- `i18n.h`/`i18n.cpp`: neue StringIds `MONITOR_INPUT_*` (DE+EN).
- `main.cpp`:
  - `runMonitorInputPicker()` - neuer Dialogschritt (5 grosse Tasten:
    Aus/HDMI1/HDMI2/DP1/DP2, "?"-Hilfe, Kopfzeilen-Zurueck behaelt den
    bisherigen Wert bei), angehaengt ans Ende von
    `runWolDeviceEntryFlow()` (jetzt mit zusaetzlichem
    `uint8_t& monitorInput`-Parameter), beide Aufrufstellen (Hinzufuegen/
    Bearbeiten in `runWolDevicesScreen()`) angepasst.
  - `sendMonitorCommand()` - sendet `"MONITOR n"` signiert, exakt
    gleicher Weg/Port wie `sendSleepCommand()`.
  - `triggerWolForDevice(deviceIdx)` - NEUE zentrale Stelle fuer JEDEN
    WoL-Ausloeser (ersetzt die beiden bisherigen direkten
    `sendWolPacket()`-Aufrufe in `handleDeviceWidgets()` UND im
    Alexa-Flow in `loop()`). Mac nicht erreichbar ODER
    `monitorInputOnPowerOn==0` -> sofortiges WoL wie bisher. Sonst:
    `MonitorPreWolState`-Zustandsautomat (kein `delay()`) wartet bis zu
    `Config::MONITOR_ACK_TIMEOUT_MS`=2000ms auf `current.monitorAck`,
    sendet danach (Erfolg oder Timeout) das WoL-Paket - in `loop()`
    AUSSERHALB von `#ifdef ALEXA_TEST`, da auch der normale Touch-Weg
    darueber laeuft.
  - `Metrics.monitorAck` (additiv wie `sleepAck`), gelesen in
    `parsePacket()` aus `"monitor_ack"`.
  - Alexa-Flow-Logzeile angepasst: "WoL angefordert fuer..." statt
    "WoL gesendet an..." (der tatsaechliche Versand kann jetzt um bis zu
    2s verzoegert sein).

### Schritt 4: Windows-Listener (abgeschlossen, siehe `windows/`)

- `main.go`: `config` bekommt additiv `MonitorSwitchEnabled`/
  `MonitorVcpValue`/`MonitorScreenNameSubstring` (leer = "nichts tun",
  gleiche Sicherheitsgrenze wie Mac-App). `findMonitorHandle()`
  (EnumDisplayMonitors+GetPhysicalMonitorsFromHMONITOR per
  `syscall.NewLazyDLL`/`syscall.NewCallback`, exakt das in
  `tools/ddc-test/windows-scripts/ddc-umschalten.ps1` erprobte Verfahren,
  jetzt in Go) sucht den ERSTEN Monitor, dessen Windows-Beschreibung den
  Suchtext enthaelt, gibt alle anderen Handles sofort frei.
  `switchMonitorInputBeforeShutdown()` laeuft in einem eigenen
  Goroutine mit `select`+`time.After(monitorDdcTimeout=3s)` - blockiert
  `runShutdown()` nie laenger als 3s, auch bei haengendem DDC-Aufruf.
  `runShutdown()` ruft das jetzt VOR `exec.Command("shutdown", ...)`
  auf (plus 1,5s Wartezeit danach), `cfg` wird dafuer unter `cfgMu`
  kopiert statt direkt gelesen (gleiches Schutzmuster wie an anderen
  `cfg`-Lesestellen).
- `installer.nsi`: 3 neue Felder in der bestehenden Einstellungen-Seite
  (Checkbox "Monitor-Eingang beim Herunterfahren automatisch
  zurueckschalten", Text "VCP-Rohwert", Text "Monitorname") - gleiches
  Lese-(`ReadExistingConfig`)/Schreib-Muster
  (`FileWrite`) wie `device_name`/`allow_legacy_commands`.
  **Visuell NICHT selbst ueberprueft** (kein Windows-Rechner verfuegbar) -
  `makensis` compiliert fehlerfrei, aber ob die 3 neuen Dialogfelder im
  bestehenden (nicht sehr hohen) Einstellungsfenster tatsaechlich lesbar/
  nicht abgeschnitten dargestellt werden, muss Alex beim Live-Test
  (Schritt 7d) bestaetigen.
- **Offene Frage aus dem Auftrag beantwortet, NICHT umgesetzt** ("soll
  der Monitor auch beim Windows-eigenen Herunterfahren zurueckschalten?"):
  NICHT trivial - der Listener ist aktuell ein reiner UDP-Listener +
  Taskleisten-Icon ohne eigenes sichtbares Fenster; ein selbst
  ausgeloestes Herunterfahren abzufangen braucht entweder eine
  `WM_QUERYENDSESSION`-Behandlung auf einem (bei `systray` intern
  verwalteten, nicht direkt zugaenglichen) Fenster-Handle oder einen
  Dienst mit `SERVICE_ACCEPT_SHUTDOWN` - beides ein eigener, groesserer
  Auftrag, hier nur geprueft/dokumentiert, nicht gebaut.

### Schritt 5: Fehlerfaelle (durchgegangen, siehe Code oben)

Mac-App aus/Mac schlaeft -> `computeLastAnyPacketMs()` veraltet,
`triggerWolForDevice()` sendet sofort WoL (regressionsgetestet, siehe
Schritt 7). Monitor/Bildschirm nicht gefunden -> Mac-App/Listener tun
nichts, WoL/Shutdown laufen unveraendert weiter. Doppelte Alexa-Befehle
-> Mac-seitige 3s-Ratenbegrenzung UNABHAENGIG von der CYD-3s-Sperre
(zwei Ebenen). Ungueltiger Wert (nicht 1-4) -> Mac-App ignoriert den
Befehl kommentarlos (`_dispatch()`). Aeltere Mac-App ohne MONITOR-
Unterstuetzung -> CYD wartet die vollen 2s, sendet WoL trotzdem (siehe
Schritt 1, "Keine Rueckfrage noetig").

### Schritt 6: Doku

- CLAUDE.md: dieser Abschnitt (ausfuehrlich).
- GitHub-Doku (README DE/EN) + licenses/THIRD-PARTY-LICENSES.md:
  **NOCH OFFEN** - folgt nach dem Live-Test mit Alex (Schritt 7), damit
  die Anleitung den tatsaechlich bestaetigten Ablauf beschreibt statt
  eines nur theoretisch geplanten.

### Schritt 7: Bauen/Flashen/Testen

- **Mac-App**: `pyinstaller` gebaut (v1.1.4), ad-hoc signiert
  (`codesign --verify`: "valid on disk"), auf Alex' Mac installiert
  (laufende Kopie per AppleScript sauber beendet, `/Applications`-
  Kopie ersetzt, neu gestartet) - lief beim zweiten Versuch fehlerfrei
  (siehe Bugfix oben), `m1ddc` im Bundle bestaetigt vorhanden
  (`Contents/Frameworks/m1ddc`, Symlink in `Contents/Resources/`).
- **CYD**: MAC vor dem Flashen per `esptool chip_id` bestaetigt
  (`f8:b3:b7:74:7f:fc`, Port 31410), geflasht, Mac-App-Verbindung laut
  `lsof` **sofort (<1s)** zurueck. Flash **1.029.949 Bytes (78,6%,
  +2.532 Bytes ggü. Alexa-Test-2)**, RAM **49.020 Bytes (15,0%,
  +32 Bytes)**. Boot-Mitschnitt fehlerfrei (SD-Karte gefunden,
  WoL-Geraet geladen, Alexa-WeMo-Emulation aktiv nach 8,34s, Heap nach
  60s stabil bei 197.140 Bytes). `ALEXA_TEST` bleibt wie gefordert aktiv.
- **Regressionstest selbststaendig durchgefuehrt** (Alexa-HTTP-Simulation,
  siehe "Eigenstaendige Seriell-Diagnose"): `monitorInputOnPowerOn==0`
  (unveraendertes Geraet "Acer Predator", noch nie am neuen Dialog-
  Schritt eingestellt) -> Alexa "an" loest weiterhin SOFORT WoL aus,
  Log "Alexa: Befehl AN, WoL angefordert fuer 'Acer Predator'" - KEINE
  Verzoegerung, KEIN "Monitor:"-Log, wie vor diesem Auftrag. Bestaetigt:
  bestehende Konfigurationen/Geraete sind durch diesen Auftrag nicht
  beeintraechtigt.
- **Windows-Listener**: Go-Kreuzbau (`-trimpath -buildvcs=false -ldflags
  "-H=windowsgui -s -w"`) + `makensis installer.nsi` erfolgreich,
  `strings` auf beiden `.exe`s: 0 Treffer fuer `/Users/`-Pfade. Alte
  `EiswolfsIBoardListener-Setup-2026-11-10.exe` auf dem Stick EISWOLF
  durch `EiswolfsIBoardListener-Setup-2026-11-14.exe` ersetzt (SHA-256
  `e00985f3...bfc1ef7`), `token-fuer-windows.json`/`DDC-Test/`
  unberuehrt gelassen.
- Flightradar (`git status`: clean) und Solar-Guardian-CYD (clean)
  bestaetigt unveraendert.
- **Live-Tests a-j mit Alex: NOCH OFFEN** - brauchen echte Touch-
  Bedienung am CYD (Monitor-Eingang im WoL-Dialog einstellen),
  Installation der neuen `setup.exe` auf dem Laptop, und visuelle
  Bestaetigung am Monitor - keines davon ist von hier aus simulierbar.
  Siehe "WIE WEITER" unten fuer die konkreten Schritte.

### WIE WEITER (Live-Tests mit Alex, Testpunkte a-j aus dem Auftrag)

1. **Mac-App**: Menü "Monitor-Umschaltung…" öffnen, einschalten,
   unter "Bildschirm" den AW2720HFA auswählen, bei Bedarf die
   Eingangswerte prüfen (sollten schon auf 4369/4626/3855 vorbelegt
   sein), dann "Test: auf HDMI 2 schalten" antippen - der Monitor
   sollte kurz auf HDMI 2 springen und nach 15s zurück, der zweite
   Mac-Monitor (U28E590) darf sich NICHT verändern.
2. **CYD**: Menü → Einrichtung → WoL-Geräte → "Acer Predator"
   bearbeiten → ganz am Ende "Monitor beim Einschalten" → "HDMI 2"
   wählen.
3. Laptop ausschalten. Acer-Button auf Controls antippen - der Monitor
   sollte nach ca. 1-2s auf HDMI 2 wechseln, der Laptop startet, das
   Bild sollte schon während des Bootens da sein und bleiben (Zeit
   nennen).
4. Zweimal kurz hintereinander antippen (Laptop zwischendurch wieder
   aus) - nur EIN Umschalt-/WoL-Vorgang, keine Fehlermeldung.
5. Mac über die Mac-Kachel in den Ruhezustand schicken (CYD wird
   dunkel), dann Acer-Button antippen - Laptop sollte trotzdem SOFORT
   starten (ohne Monitor-Befehl/Verzögerung), im Log "Monitor: Mac
   nicht erreichbar, nur WoL" (falls Serial mitgeschnitten wird).
6. Die neue `EiswolfsIBoardListener-Setup-2026-11-14.exe` vom Stick auf
   dem Laptop installieren (Update über die bestehende Installation),
   in den Einstellungen "Monitor-Eingang beim Herunterfahren
   automatisch zurückschalten" aktivieren, VCP-Rohwert `4369`
   (HDMI 1/Mac) eintragen, Monitorname `AW2720HFA` eintragen - **bitte
   kurz bestätigen, ob die 3 neuen Felder im Einstellungsfenster
   überhaupt sichtbar/nicht abgeschnitten sind** (siehe Hinweis oben,
   konnte hier nicht selbst geprüft werden).
7. Laptop über das CYD ausschalten (Halten auf dem Acer-Button) - der
   Monitor sollte auf HDMI 1 zurückwechseln (Mac-Bild da), DANACH fährt
   der Laptop herunter.
8. "Alexa, schalte Laptop an" - gleiche Kette wie Schritt 3.
9. "Alexa, schalte Laptop aus" - weiterhin nur Log, keine Aktion (wie
   in Alexa-Test 2).
10. Prüfen, ob die Eingangswahl des Monitors auf "Auto" oder "aus"
    besser funktioniert (eigene Beobachtung notieren).

Nach erfolgreichem Test: Ergebnis hier nachtragen, README/GitHub-Doku
(Schritt 6) ergänzen. Testwerkzeuge (`tools/ddc-test/`, `DDC-Test/` auf
dem Stick) bleiben bis zur ausdrücklichen Freigabe liegen. NICHT
committen/taggen/pushen ohne expliziten Wunsch.

## Auftrag Monitor-Eingang Machbarkeit: Fortschritt (14.11., laufend)

Reiner Machbarkeitstest/Bericht - NICHTS wird in Firmware/Mac-App/
Listener eingebaut, NICHTS committen/taggen/pushen. Testskripte liegen
in `tools/ddc-test/` (per `.gitignore` ausgeschlossen).

### Schritt 1: Bestandsaufnahme/Vorbereitung (abgeschlossen)

- **Mac-Anschluss**: `system_profiler SPDisplaysDataType` bestaetigt
  Anzeige "AW2720HFA", 1920x1080 @240Hz, am Apple M4 (Mac mini). Die
  Anschlussart (HDMI vs. DisplayPort) wird von `system_profiler` auf
  Apple Silicon fuer externe Monitore NICHT mit ausgegeben (bekannte
  Einschraenkung, auch per `ioreg` nicht auffindbar) - verlasse mich
  hier auf Alex' eigene Angabe (HDMI 1 = Mac mini), was auch zum
  Mac mini M4 passt (kein nativer DisplayPort-Ausgang, nur HDMI/USB-C).
- **Monitor-Recherche (AW2720HFA)**: DDC/CI wird vom Monitor
  grundsaetzlich unterstuetzt (siehe unten, Lesen funktioniert). Dell/
  Alienware-Monitore dieser Generation sind in mehreren unabhaengigen
  Quellen (Dell-Community-Forum zu U3821DW, `ddccontrol-db`-Eintraege
  anderer Dell-Modelle) dafuer bekannt, bei VCP-Code 0x60 (Eingangswahl)
  einen UNUEBLICHEN, "verdoppelten" Rohwert zu melden (z.B. 4369=0x1111
  statt des in der VESA-MCCS-Norm vorgesehenen einfachen Werts 17 fuer
  HDMI 1) - das ist ein bekannter Dell-Firmware-Eigenheit, kein
  Mess- oder Code-Fehler unsererseits. **Wichtiger, in mehreren
  Quellen uebereinstimmender Befund:** Dells eigene Support-FAQ nennt
  explizit AW2720HF/AW2720HFA als Modelle, bei denen die Monitor-
  Einstellung "Eingangswahl: Auto" dazu fuehrt, dass der Monitor NICHT
  automatisch wechselt, solange am aktuellen Eingang ein Signal anliegt
  - deckt sich 1:1 mit Alex' eigener Beobachtung.
- **Mac-Werkzeug**: `m1ddc` (github.com/waydabber/m1ddc), MIT-Lizenz,
  quelloffen, kompiliert gegen Apples CoreDisplay-Framework. NICHT per
  `brew install` (das waere ein Systemordner/Homebrew-Prefix) - Quelltext
  per `git clone` in `tools/ddc-test/m1ddc-src/` geholt und dort lokal
  mit `make` gebaut (reine `clang`+CoreDisplay-Abhaengigkeit, keine
  weiteren Downloads). Binary liegt NUR dort, nirgendwo installiert.
  **Wichtiger Befund aus der Recherche (noch vor dem eigentlichen
  Test)**: mehrere unabhaengige Quellen (u.a. Apple-Support-Community-
  Threads, MonitorControl-Projekt-Wiki) berichten, dass DDC/CI auf
  Apple-Silicon-Macs **nur ueber USB-C/DisplayPort Alt Mode
  zuverlaessig funktioniert, nicht ueber HDMI** - das Risiko, dass der
  Mac-mini-Weg (HDMI) deshalb gar nicht erst funktioniert, stand von
  vornherein im Raum. Der tatsaechliche Test unten widerlegt das fuer
  diesen konkreten Mac mini M4 + dieses Monitor-Modell aber zumindest
  fuers LESEN (siehe Schritt 2) - interessant, da es der verbreiteten
  Erwartung widerspricht.
- **Windows-Skripte**: `tools/ddc-test/windows-scripts/ddc-lesen.ps1`
  und `ddc-umschalten.ps1`, reines PowerShell + P/Invoke gegen
  `dxva2.dll` (`GetVCPFeatureAndVCPFeatureReply`/`SetVCPFeature`/
  `GetCapabilitiesStringLength`/`CapabilitiesRequestAndCapabilitiesReply`,
  `EnumDisplayMonitors`) - keine Installation, kein Zusatzprogramm.
  `ddc-umschalten.ps1` liest IMMER zuerst den aktuellen Wert, bevor es
  umschaltet, und schaltet nach `-ZurueckNach` Sekunden (Standard 15)
  automatisch darauf zurueck. Kopiert nach `/Volumes/EISWOLF/DDC-Test/`
  (Alex hat den Stick-Inhalt vorher gezeigt bekommen, NICHTS geloescht,
  nur der neue Unterordner `DDC-Test/` ergaenzt) - dort zusaetzlich
  `ANLEITUNG.txt` mit Startbefehlen.

### Schritt 2: Tests am Mac, nur lesen (abgeschlossen)

- `m1ddc display list` erkennt den Monitor sofort (`[1] AW2720HFA`).
- `m1ddc display 1 get input` (VCP 0x60) **dreimal wiederholt, JEDES
  Mal identisch und erfolgreich**: Rohwert **4369 (0x1111)** - passt
  exakt zu dem oben recherchierten Dell-"verdoppelten"-Encoding fuer
  HDMI 1 (ein unabhaengiger Dell-Community-Bericht zu einem anderen
  Dell-Modell nennt genau diesen Wert 4369 fuer HDMI 1). Stimmt mit
  der Erwartung zusammen (Mac haengt laut Alex an HDMI 1).
- `m1ddc` hat KEINEN eigenen Befehl fuer die DDC-Faehigkeiten-
  Zeichenkette (`get caps` existiert nicht, nur fest kodierte VCP-Werte
  fuer bekannte Funktionen) - auf dem Mac konnte daher nur der
  Eingangswert, keine rohe Faehigkeiten-Zeichenkette gelesen werden.
  Das Windows-Skript `ddc-lesen.ps1` kann das zusaetzlich (eigener
  `dxva2.dll`-Aufruf dafuer vorhanden), siehe Schritt 3.
- Eingangswahl am Monitor laut Alex auf "Auto" (siehe Dell-FAQ-Befund
  oben - genau die Konstellation, in der "Auto" bei diesem Modell NICHT
  automatisch wechselt, solange ein Signal anliegt).
- **Noch NICHT getestet** (bewusst, nur nach Alex' "jetzt" erlaubt):
  tatsaechliches Umschalten vom Mac aus, Lesen/Schreiben vom Windows-
  Laptop aus (aktiver/nicht-aktiver Eingang).

### Schritt 3: Umschalttests (Kernergebnisse abgeschlossen)

VCP-0x60-Rohwerte fuer dieses Geraet jetzt empirisch bestaetigt (Dells
"verdoppelte" Kodierung, siehe Schritt 1/2): **HDMI 1 = 4369 (0x1111)**,
**HDMI 2 = 4626 (0x1212)** (DisplayPort vermutlich 3855/0x0F0F nach
demselben Muster, NICHT getestet). Die vom Monitor selbst gemeldete
Faehigkeiten-Zeichenkette nennt dagegen die STANDARD-Einzelbyte-Werte
(`60(0F 11 12 )` = 15/17/18) - das ist genau der in der Dell-Recherche
beschriebene Widerspruch zwischen deklarierten und tatsaechlich
genutzten Werten. **Wichtig: Beim SCHREIBEN funktionierten sowohl der
Standardwert (17) als auch der verdoppelte Wert (4369) gleichermassen
zum Umschalten** - nur das LESEN liefert zuverlaessig ausschliesslich
die verdoppelte Form.

| Test | Rechner (Rolle) | Eingang aktiv? | Ergebnis | Hinweise |
|---|---|---|---|---|
| Windows-Lesen | Laptop | Ja (HDMI2 aktiv) | OK - Rohwert 4626, Faehigkeiten-String vollstaendig gelesen | Einziger erfolgreicher Capabilities-Read des gesamten Tests (Mac kann das mit `m1ddc` gar nicht) |
| Windows->HDMI2 (Wert 18) | Laptop (AKTIV) | Ja | Kein sichtbarer Wechsel (Ziel=Ist, nur Flackern) | Trivialfall, keine Aussagekraft fuer "aktiv->aktiv" |
| Windows->HDMI1 (Wert 17) | Laptop (AKTIV) | Ja | **Wechsel erfolgreich**, kein Signalproblem | Bestaetigt Test C aus dem Auftrag: aktiver Rechner kann sich selbst wegschalten |
| Windows->HDMI1 (Wert 4369) | Laptop (AKTIV) | Ja | Wechsel erfolgreich, aber Ruecksprung auf HDMI2 nach 15s zeigte "kein Signal" | Siehe Kernbefund unten - Ursache ist NICHT die Wertkodierung |
| Windows->HDMI2 (Wert 4626, Ziel=Ist) | Laptop (AKTIV) | Ja | Kein Problem, blieb auf HDMI2 | Ebenfalls Trivialfall |
| Mac->HDMI2, 1. Versuch | Mac (AKTIV, HDMI1) | **War tatsaechlich schon auf HDMI2** (Rest vom vorigen Windows-Test) | Kein echter Test - Zielzustand war schon aktiv | Mac-seitiges Auslesen (`m1ddc get input`) zeigte waehrenddessen fehlerhaft weiter ~HDMI1-nahen Wert (4370) - **Lesen ueber HDMI am Mac ist unzuverlaessig/kann veraltete Werte liefern**, siehe Risiken unten |
| Mac->HDMI2, 2. Versuch (sauber, Alex hat vorher per Hand auf HDMI1 geschaltet) | **Mac (AKTIV, HDMI1)** | Ja | **Wechsel erfolgreich** (Alex: "hat auf 2 umgeschalten"), ABER "kein Signal" bis Kabel am Laptop neu gesteckt wird | Bestaetigt Test A aus dem Auftrag - DDC-Schreiben vom Mac ueber HDMI funktioniert entgegen der verbreiteten Erwartung ("nur USB-C/DP") grundsaetzlich! |
| Ruecksprung Mac->HDMI1 (von Karl sofort manuell nachgeholt, kein Auto-Revert bei diesem einzelnen Testbefehl) | Mac | - | Erfolgreich, Mac-Bild kam sofort zurueck (Mac liefert durchgehend Signal) | - |

**Kernbefund (von Alex bestaetigt, deckt sich mit einem ihr schon lange
bekannten, bisher ungeloesten Problem):** Das "kein Signal"-Symptom
beim Wechsel AUF HDMI 2 ist **kein DDC-/Kodierungsproblem**, sondern ein
seit laengerem bestehender Eigenheit des Laptops/Windows-Grafiktreibers:
**Nur WAEHREND der Bootphase (BIOS/POST/Windows-Ladebildschirm) liefert
der Laptop ein Signal, das der Monitor bei einem Eingangswechsel sofort
annimmt.** Ist Windows bereits vollstaendig hochgefahren und man schaltet
den Monitor-Eingang weg und wieder zurueck auf HDMI 2, bleibt der
Bildschirm auf "kein Signal", bis das HDMI-Kabel am Laptop neu gesteckt
wird (Alex hat dafuer in der Vergangenheit bereits selbst nach einem
Tastenkuerzel gesucht, das Windows zum "HDMI-Port-Refresh" bewegt -
ohne Erfolg). Betrifft NUR die Richtung "Wechsel auf HDMI 2" - der
umgekehrte Weg (Windows schaltet sich selbst weg auf HDMI 1, waehrend es
noch laeuft) hatte in keinem Versuch dieses Problem, ebenso wenig wie
der Mac, der durchgehend ein Signal liefert.

**Test B (Laptop, NICHT aktiver Eingang) sowie die Wiederholung mit
Eingangswahl "Aus"/fest wurden NICHT mehr durchgefuehrt** - nach dem
Kernbefund oben (das eigentliche Hindernis ist eine Boot-Zeit-Frage,
keine DDC-/Monitor-Einstellungsfrage) erschien der Mehrwert gering;
siehe Abschlussfrage an Alex, ob das noch gewuenscht ist.

### Schritt 4: Auswertung

**Empfehlung: Variante (b), aber in angepasster/hybrider Form - mit
direkter CYD->Mac-Kopplung an den WoL-Zeitpunkt (siehe Nachtrag unten)
grundsaetzlich machbar, nicht aber in der urspruenglich gedachten
einfachsten Form (Mac raet/wartet selbst, wann der Laptop bootet).**

- **"An"-Richtung (Monitor auf HDMI 2 beim Einschalten)**: Nur
  realistisch, wenn der Umschalt-Befehl WAEHREND der Bootphase des
  Laptops gesendet wird, nicht erst nach vollstaendigem Hochfahren.
  Der **Mac** ist dafuer besser geeignet als der **Windows-Listener**:
  Der Mac kennt den exakten Zeitpunkt des WoL-Versands (ueber die
  Alexa-/Touch-Funktion) und kann den DDC-Befehl mit einer kurzen,
  gezielten Verzoegerung danach senden, waehrend der Laptop
  nachweislich noch bootet. Der Windows-Listener dagegen startet
  typischerweise erst beim Login - zu diesem Zeitpunkt ist das
  "Boot-Signal-Fenster" laut Alex' Erfahrung oft schon vorbei, der
  Versuch waere dadurch unzuverlaessiger, nicht zuverlaessiger.
- **"Aus"-Richtung (Monitor zurueck auf HDMI 1 vor dem Ausschalten)**:
  HIER ist die urspruengliche Variante (a) nachweislich zuverlaessig
  (Test C bestanden, zweimal wiederholt) - der Windows-Listener kann
  das beim Empfang des signierten SHUTDOWN-Befehls (siehe
  `runShutdown()` in `windows/listener/main.go`, Zeile ~281) VOR dem
  eigentlichen `shutdown /s` zuverlaessig selbst erledigen, da Windows
  in diesem Moment noch die aktive, durchgehend signalgebende Quelle
  ist - genau der Fall, der in den Tests nie das Signalproblem zeigte.
- **Empfohlene Aufteilung**: Mac-App schaltet (nach dem WoL-Versand,
  mit kurzer Verzoegerung, nur wenn der Mac selbst laeuft/wach ist) auf
  HDMI 2 um; Windows-Listener schaltet beim Empfang des Shutdown-
  Befehls VOR dem Herunterfahren zurueck auf HDMI 1. Das kombiniert die
  jeweils zuverlaessig getestete Richtung jeder Maschine, statt eine
  Maschine beides machen zu lassen.

**Aufwand**: ueberschaubar auf beiden Seiten (Mac-App: ein zusaetzlicher
`m1ddc`-Subprozessaufruf oder eine eigene kleine CoreDisplay-Anbindung
nach dem WoL-Versand; Windows-Listener: ein P/Invoke-Aufruf wie in
`ddc-umschalten.ps1` erprobt, eingebaut in Go z.B. via `golang.org/x/sys/windows`
oder einen eigenen `dxva2.dll`-Syscall, VOR `exec.Command("shutdown", ...)`
in `runShutdown()`). Kein neues Drittanbieter-Tool noetig, `m1ddc` selbst
muesste fuer eine echte Einbindung in die Mac-App aber zusaetzlich lizenz-
rechtlich/technisch bewertet werden (Subprozess-Aufruf vs. eigene
CoreDisplay-Anbindung ohne fremden Code) - hier bewusst NICHT entschieden,
da dieser Auftrag reine Machbarkeit war.

**Nachtrag (Alex' Einwand, direkt nach der ersten Fassung dieser
Empfehlung)**: Das urspruengliche Risiko 1 (Timing) entschaerft sich
erheblich, wenn die Kopplung architektonisch richtig gemacht wird: Der
Ausloeser ist IMMER das CYD (Touch oder Alexa) - sendet das CYD in
genau diesem Moment zusaetzlich zum WoL-Paket eine kleine Benachrichtigung
an die Mac-App ("WoL jetzt ausgeloest"), kann die Mac-App ihren DDC-
Umschaltbefehl praktisch GLEICHZEITIG mit dem Stromeinschalten des
Laptops senden - also exakt im Boot-Fenster, nicht erst nach einer
geschaetzten Verzoegerung. Das bisherige Risiko war nur relevant, wenn
die Mac-App selbst raten muesste, WANN der Laptop gebootet wird (z.B.
per Polling); bei direkter Benachrichtigung durch das CYD entfaellt
dieses Raten komplett. **Fehlender Baustein dafuer**: Es gibt aktuell
KEINEN Weg, wie die Mac-App vom CYD erfaehrt, dass gerade WoL gesendet
wurde - das CYD sendet das Magic Packet bisher direkt und autonom
(`sendWolPacket()`), ohne die Mac-App zu informieren (anders als beim
Windows-Shutdown, wo das CYD einen signierten Befehl an einen
lauschenden Listener schickt). Eine kuenftige Umsetzung braucht daher
zusaetzlich eine kleine neue Benachrichtigung CYD->Mac-App (z.B. ueber
einen neuen, eigenen UDP-Port nach demselben Muster wie
`WIN_SHUTDOWN_UDP_PORT`) - das ist selbst KEIN Timing-Risiko mehr,
sondern nur noch ein ueberschaubarer Implementierungsschritt fuer einen
spaeteren Auftrag.

**Risiken**:
1. **Restliches Timing-Risiko (deutlich kleiner als urspruenglich
   gedacht, siehe Nachtrag oben)**: Auch bei sofortiger Benachrichtigung
   bleiben Netzwerk-/Verarbeitungslaufzeiten (UDP-Zustellung, Mac-App-
   Reaktionszeit, `m1ddc`-Prozessstart) im Bereich von Millisekunden bis
   wenigen Sekunden - wie gross das reale Boot-Signal-Fenster ist
   (wahrscheinlich mehrere Sekunden bis Minuten, siehe Alex'
   Erfahrungswert "waehrend des Bootens"), wurde NICHT gemessen. Fuer
   eine echte Umsetzung lohnt trotzdem ein kurzer Praxistest (einmal
   WoL+Umschaltbefehl ohne kuenstliche Verzoegerung durchspielen), aber
   das urspruengliche "zu spaet dran"-Szenario entfaellt bei korrekter
   Kopplung an das WoL-Ereignis selbst.
2. **Mac-seitiges Lesen ueber HDMI ist unzuverlaessig** (siehe Tabelle) -
   eine kuenftige Mac-App-Logik sollte sich NICHT auf eine Lese-
   Rueckmeldung verlassen, um Erfolg zu pruefen, nur auf das Senden des
   Schreibbefehls.
3. **Ruhezustand des Mac**: Wenn der Mac selbst schlaeft (siehe Auftrag
   "Ruhezustand-Fix"), kann er wahrscheinlich gar keinen DDC-Befehl
   senden (ungetestet) - UND wenn der Windows-Listener in diesem Moment
   auf HDMI 1 zurueckschaltet, liefert ein schlafender Mac vermutlich
   ebenfalls kein Signal (symmetrisches Problem zum Laptop-Boot-Fenster,
   nur umgekehrt) - noch ungetestet, aber naheliegend.
4. **Windows-Treiber/Grafikkarten-Wechsel** (z.B. Hybrid-Grafik Intel/
   Nvidia) koennten das Boot-Fenster-Verhalten bei einem Treiber-Update
   veraendern - nicht ueberprueft, welche GPU der Acer Predator nutzt.
5. **Was vor dem Ausschalten passieren muss**: Der DDC-Umschaltbefehl
   (Windows->HDMI1) muss zeitlich VOR dem eigentlichen `shutdown /s`
   passieren (nicht danach) - sobald Windows herunterfaehrt, kann es
   keine DDC-Befehle mehr senden. In `runShutdown()` ist das eine reine
   Reihenfolge-Frage beim Einbau, kein grundsaetzliches Problem.

**Variante (c) "nicht zuverlaessig machbar" gilt NICHT pauschal** - beide
Einzelrichtungen SIND zuverlaessig machbar, nur eben nicht symmetrisch
von derselben Maschine aus und nicht ohne die Boot-Zeit-Frage zu klaeren.

### Aufräumen (Werkzeuge)

- **Mac**: `tools/ddc-test/m1ddc-src/` (geklonter Quelltext + gebautes
  `m1ddc`-Binary, NUR dort, keine Installation in Homebrew/Systemordner).
- **Windows (USB-Stick EISWOLF, Ordner `DDC-Test/`)**: `ddc-lesen.ps1`,
  `ddc-umschalten.ps1`, `ANLEITUNG.txt`, plus Alex' eigene
  `auswertung.txt` (NICHT von Karl erzeugt, bleibt ihr Eigentum/ihre
  Notiz). Rest des Sticks unberuehrt (siehe Schritt 1).
- Werden erst geloescht, wenn Alex das ausdruecklich sagt - bis dahin
  bleibt alles liegen (z.B. falls das Boot-Fenster noch zeitlich
  vermessen werden soll).

## Auftrag Alexa-Test 2: Fortschritt (14.11., laufend)

Test 1 (siehe unten) hat funktioniert. Jetzt: "Alexa, schalte Laptop an"
loest denselben WoL-Aufruf aus wie der Touch auf den Acer-Button/die
Acer-Kachel. "Aus" bleibt weiterhin reine Logzeile, keine Aktion. Nur
`cyd/` betroffen, NICHT committen/taggen/pushen.

### Schritt 1: Fundstellen (nur gelesen, abgeschlossen)

- **WoL-Versand**: `sendWolPacket(const String& macStr)`, `main.cpp`
  Zeile 604 - reines Magic-Packet-Broadcast an Port 9, unveraendert
  wiederverwendet (keine zweite Kopie).
- **3s-Sperre, gemeinsam fuer Touch UND Alexa**:
  `deviceTapLockedUntilMs[Config::MAX_WOL_DEVICES]` (Zeile 1817),
  Konstante `CTRL_TAP_LOCKOUT_MS = 3000` (Zeile 1789). Der Touch-Pfad
  prueft/setzt das in `handleDeviceWidgets()` (Zeile ~2145-2157) fuer
  den jeweils angetippten Geraete-Index - Alexa nutzt jetzt denselben
  Array-Slot fuer Index 0 (das WoL-Geraet Nr. 1, der migrierte Acer),
  siehe Schritt 2.
- **Alexa-Callback aus Test 1**: `fauxmo.onSetState(...)` in `setup()`
  (damals ~Zeile 3037, schrieb bei AN/AUS nur jeweils dieselbe
  "Test, keine Aktion"-Logzeile) - fuer "an" jetzt ersetzt durch das
  Merker-Flag `alexaWolRequested` (siehe Schritt 2), "aus" unveraendert
  als reine Logzeile stehen gelassen.
- **Geraeteliste**: `deviceCfg.wolDevices[0]` ist immer der migrierte
  Acer (Index 0, siehe Abschnitt "WoL-Geraete" oben, "Index 0 ist der
  ggf. migrierte erste Eintrag") - exakt das Geraet, das Alexa laut
  Auftrag ansteuern soll, keine Namenssuche noetig.

### Schritt 2: Umsetzung (abgeschlossen, siehe `main.cpp`)

- Neue globale Zustaende (anonymer Namespace, nur `#ifdef ALEXA_TEST`):
  `volatile bool alexaWolRequested` (Merker-Flag, von `onSetState()`
  gesetzt) und `uint32_t alexaOffRevertAtMs` (Taster-Ruecksetzen,
  0 = nichts ausstehend).
- `onSetState()`-Callback (laeuft im AsyncTCP-Kontext, NICHT im
  Haupt-`loop()`-Task): bei `state==true` wird NUR
  `alexaWolRequested = true` gesetzt - kein WoL-Versand, kein
  Sperre-Check, keine Zustandsaenderung direkt im Callback. Bei
  `state==false` unveraendert nur die Logzeile
  "Alexa: Befehl AUS (Test, keine Aktion)".
- `loop()` (nach dem bestehenden `fauxmo.handle()`/Heap-Log-Block):
  liest `alexaWolRequested`, loescht es sofort wieder, und dann:
  - `wolDeviceCount==0` -> Log "Alexa: kein WoL-Geraet eingerichtet",
    keine Aktion.
  - `millis() < deviceTapLockedUntilMs[0]` -> Log "Alexa: Befehl AN,
    Sperre aktiv, ignoriert" (greift z.B. bei doppelt zugestellten
    Alexa-Kommandos oder kurz nach einem Touch-WoL).
  - sonst: `sendWolPacket(deviceCfg.wolDevices[0].mac)` (dieselbe
    Funktion wie der Touch), `deviceTapLockedUntilMs[0]` auf +3s
    gesetzt (DIESELBE Sperre wie beim Touch, kein eigener Zaehler),
    Log "Alexa: Befehl AN, WoL gesendet an \"<Name>\"", danach
    `alexaOffRevertAtMs = millis() + 5000`.
  - Jeder "an"-Befehl loest WoL aus, auch wenn Alexa den Zustand
    bereits fuer "an" hielt - fauxmoESP ruft `onSetState()` bei JEDEM
    PUT auf, unabhaengig vom vorherigen Zustand (im Bibliothekscode
    gegengeprueft, `fauxmoESP.cpp::_onTCPRequest()`), kein Sonderfall
    noetig.
  - Separat (unabhaengig vom obigen Flag-Block): ist
    `alexaOffRevertAtMs!=0` und die Zeit erreicht, wird es auf 0
    zurueckgesetzt und `fauxmo.setState((unsigned char)0, false, 0)`
    aufgerufen - das aendert NUR den internen Zustand (keine eigene
    Callback-Ausloesung), damit der naechste Abfrage-/`GetFullState`-
    Aufruf von Alexa wieder "aus" zeigt (Taster-Verhalten).
- Kein Anzeige-Hinweis eingebaut (wie in Test 1 bewusst auf reines
  Serial-Log beschraenkt, um jedes Layoutrisiko zu vermeiden) - im
  dunklen Ruhezustand bleibt das Display dadurch ohnehin unberuehrt.
- Weiterhin nur ueber `ALEXA_TEST` aktiv, echter Menueschalter und
  Ausschalten per Sprache bleiben spaeteren Auftraegen vorbehalten.

### Schritt 2b: Eigener Funktionsnachweis OHNE Alexa (abgeschlossen)

Per `curl`-Simulation (siehe "Eigenstaendige Seriell-Diagnose"):
- Einzelner `PUT .../lights/1/state {"on":true}` -> Serial-Log sofort
  `Alexa: Befehl AN, WoL gesendet an "Acer Predator"`.
- Zwei weitere `PUT ... on:true` innerhalb von 3s direkt danach ->
  BEIDE korrekt `Alexa: Befehl AN, Sperre aktiv, ignoriert` - kein
  zweites WoL-Paket.
- Nach >5s: `GET .../lights/1` zeigt `"state":{"on": false, ...}` -
  Taster-Ruecksetzen funktioniert, naechster "an"-Befehl waere wieder
  ein Zustandswechsel.
- `PUT ... on:false` -> unveraendert nur
  `Alexa: Befehl AUS (Test, keine Aktion)`, keine Aktion.
- Heap waehrend des gesamten Tests stabil (~196,7KB frei).

### Schritt 3: Bauen, flashen - abgeschlossen; Live-Test mit Alex - OFFEN

- Baseline-Kontrolle MIT auskommentiertem `ALEXA_TEST`: exakt wie vor
  Test 1 (998.493 Bytes/76,2% Flash, 48.792 Bytes/14,9% RAM) - keine
  Verschiebung durch die neuen Codepfade, da sie komplett hinter
  `#ifdef ALEXA_TEST` liegen.
- Mit `ALEXA_TEST` aktiv: 1.027.417 Bytes (78,4% Flash, +416 Bytes
  gegenueber Test 1), 48.988 Bytes (14,9% RAM, +8 Bytes) - wie erwartet
  minimal, da nur ein paar zusaetzliche Zeilen in bereits bestehenden
  Funktionen/Variablen.
- MAC des Testgeraets vor dem Flashen erneut per `esptool chip_id`
  bestaetigt (`f8:b3:b7:74:7f:fc`, Port `/dev/cu.wchusbserial31410`).
  Geflasht, Mac-App-Verbindung war laut `lsof` bereits **1 Sekunde**
  nach Upload-Ende wieder da (identisch zu Test 1).
- Flightradar (`git status`: clean) und Solar-Guardian-CYD (clean)
  per Verzeichnis/`git status` bestaetigt unveraendert (Toolbox hat
  eigene, von fruehereren/anderen Sitzungen stammende uncommittete
  Aenderungen - nicht von dieser Aufgabe beruehrt, nur zur Kenntnis
  genommen, nicht veraendert).

### WIE WEITER (Live-Test mit Alex, Testpunkte a-f aus dem Auftrag)

Laptop muss dafuer AUSGESCHALTET sein, bevor es losgeht:
1. "Alexa, schalte Laptop an" -> Laptop sollte starten (Zeit nennen),
   im Serial-Log steht `Alexa: Befehl AN, WoL gesendet an "Acer
   Predator"`.
2. Laptop wieder ausschalten (ueber CYD lang-Druck oder von Hand).
   "Alexa, schalte Laptop an" ZWEIMAL kurz hintereinander sagen (oder
   in der App zweimal antippen) -> nur EIN WoL-Paket, kein Fehler von
   Alexa.
3. Mac ueber die Mac-Kachel in den Ruhezustand schicken, CYD wird
   dunkel. "Alexa, schalte Laptop an" -> Laptop startet, CYD-Display
   bleibt dunkel (keine Beruehrung durch den Alexa-Befehl ausgeloest).
4. "Alexa, schalte Laptop aus" -> Alexa sagt "Okay", am Laptop
   passiert NICHTS (Auftrag: Ausschalten per Sprache kommt erst
   spaeter).
5. Acer-Button auf Controls UND auf "Keine Verbindung zum Mac" pruefen
   - sollte sich wie vorher verhalten (Tippen=WoL, Halten=Ausschalten),
   unveraendert durch diesen Auftrag.
6. CYD vom Strom trennen/wieder einstecken, dann OHNE neue Geraetesuche
   nochmal "an" sagen - sollte weiter funktionieren, KEIN doppeltes
   Geraet in der Alexa-App (MAC-basierte `uniqueid` bleibt stabil).

Nach erfolgreichem Test: Ergebnis hier nachtragen. `ALEXA_TEST` bleibt
bis auf Weiteres aktiv (fuer diesen Live-Test notwendig) - vor einem
Commit/Normalbetrieb an einem oeffentlichen Stand wieder auskommentieren.

## Auftrag Alexa-Test 1: Fortschritt (14.11., laufend)

Ziel: CYD gibt sich im Heimnetz als smartes Geraet "Laptop" aus, Alexa
findet es und reagiert auf "an"/"aus" - in DIESEM Test OHNE jede Aktion
am Laptop/Mac. Nur `cyd/` betroffen, NICHT committen/taggen/pushen.

### Schritt 1: Bestandsaufnahme (abgeschlossen)

- **Vorher kein Webserver/SSDP**: `main.cpp` kannte nur `WiFiUdp` auf
  Port 4210 (Mac-Daten) und UDP-Einzelpakete an 4211/4212/9 (Luefter/
  Shutdown/WoL) - kein TCP-Server, kein SSDP, Port 80 ungenutzt.
- **Kern-Version**: PlatformIO `espressif32 @ 7.0.1`,
  `framework-arduinoespressif32 @ 3.20017.241212` (ESP-IDF-5-Basis, die
  "Core 3.x"-Generation), ArduinoJson 7.4.3. Baseline-Flash vor diesem
  Auftrag: 998.493 Bytes (76,2%), RAM 48.792 Bytes (14,9%).
- **fauxmoESP** (`vintlabs/fauxmoESP`, MIT-Lizenz, Fortsetzung von Xose
  Pérez' Original seit 2020 unter Paul Vint, letzter groesserer Release
  3.1.2 Ende 2020, danach nur noch sporadisch gepflegt): kompiliert
  OHNE Fehler gegen die aktuelle Core-Version (nur eine harmlose
  Deprecation-Warnung fuer `AsyncClient::close(bool)`), zieht
  `AsyncTCP` automatisch mit. In einem isolierten Test-Projekt
  (nur `WiFi.h` + fauxmoESP) lag der Flash-Mehrverbrauch bei ca.
  **+40 KB** gegenueber einem reinen WLAN-Projekt.
- **Espalexa** (`Aircoookie/Espalexa`, MIT-Lizenz, letzter Release
  2.7.0 von 2021-03-22, seither offenbar keine Aktivitaet mehr):
  kompiliert ebenfalls fehlerfrei (nutzt den im Core eingebauten
  synchronen `WebServer`), Flash-Mehrverbrauch im selben Testaufbau ca.
  **+80 KB** - doppelt so viel wie fauxmoESP, daher NICHT als erste
  Wahl verwendet (siehe Schritt 2).
- **Ueberraschender Befund (wichtig fuer spaetere Auftraege)**: Die
  Bibliothek heisst "fauxmoESP" und wird meist als "WeMo-Emulation"
  beschrieben, SENDET AUF DEM DRAHT aber tatsaechlich eine
  **Philips-Hue-Bruecken-Antwort** (SSDP-`SERVER: FreeRTOS/6.0.5,
  UPnP/1.0, IpBridge/1.17.0`, `description.xml` meldet
  "Philips hue bridge 2012", Geraete-Liste unter `/api/.../lights`) -
  das ist technisch die robustere der beiden im Auftrag genannten
  Varianten, Alex bekommt sie also automatisch "gratis" mit einer
  einzigen Bibliothek, ohne Espalexa extra zu brauchen.
- **Geraete-ID/Seriennummer**: fauxmoESP leitet die `uniqueid` jedes
  Geraets bereits selbst aus `WiFi.macAddress()` ab
  (`fauxmoESP::addDevice()`) - bleibt nach einem Neustart automatisch
  stabil, keine eigene Ableitung noetig.
- **Multicast in Alex' Netz entgegen der Befuerchtung UNAUFFAELLIG**:
  Die bestehende Stolperfalle "Router filtern Multicast" (siehe
  Abschnitt "Behoben: Abbrueche im Modus WLAN" oben, fast kompletter
  Broadcast-Verlust) betraf reinen UDP-**Broadcast**. Ein per
  `WiFiUDP::beginMulticast()` sauber der Gruppe 239.255.255.250:1900
  beigetretener Client bekam im Live-Test ein von einem Mac-Skript
  gesendetes M-SEARCH **zuverlaessig** zugestellt (vermutlich IGMP-
  Snooping-Switch, der echte IGMP-Join-Mitgliedschaften korrekt
  weiterleitet, reinen Broadcast aber dämpft) - die Sorge aus dem
  Auftrag ("Router filtern Multicast") hat sich HIER nicht bestaetigt,
  sollte aber bei anderen Netzen/Repeatern weiterhin im Hinterkopf
  bleiben.

### Schritt 2: Testumsetzung (abgeschlossen, siehe `main.cpp`/`config.h`)

- Build-Flag `ALEXA_TEST` in `config.h` (gleiches Muster wie
  `LAYOUT_DEBUG`/`SCREEN_TOUR`, Standard AUSKOMMENTIERT = aus). Build
  MIT auskommentiertem Flag ist Byte-fuer-Byte identisch zur Baseline
  (998.493 Bytes/76,2% Flash, 48.792 Bytes/14,9% RAM) - der
  Linker (`--gc-sections`) entfernt fauxmoESP/AsyncTCP komplett, wenn
  `fauxmoESP.h` nicht includiert wird (passiert nur hinter
  `#ifdef ALEXA_TEST`). Mit aktiviertem Flag: 1.027.001 Bytes (78,4%
  Flash, +28,5 KB), 48.980 Bytes (14,9% RAM, kaum Aenderung).
- `fauxmo.createServer(true)`/`setPort(80)`/`addDevice("Laptop")`/
  `onSetState(...)` in `setup()` (Konstante `Config::ALEXA_DEVICE_NAME`)
  - der `onSetState`-Callback schreibt NUR eine Logzeile
  ("Alexa (Test): Befehl AN/AUS fuer \"Laptop\" (Test, keine Aktion)"),
  keine Aktion am Laptop/Mac.
  `fauxmo.enable(true)` (tritt der SSDP-Multicast-Gruppe bei) passiert
  BEWUSST ERST in `loop()`, sobald `WiFi.status()==WL_CONNECTED`
  zum ersten Mal zutrifft (vorher haette `beginMulticast()` keine
  gueltige Interface-IP) - danach laeuft `fauxmo.handle()` jede
  Runde mit. `DEBUG_FAUXMO Serial` ist (nur innerhalb des
  `ALEXA_TEST`-Zweigs) gesetzt, bevor `fauxmoESP.h` includiert wird -
  liefert die internen "[FAUXMO] ..."-Logs (M-SEARCH/HTTP) kostenlos.
  Alle 60s (`Config::ALEXA_HEAP_LOG_INTERVAL_MS`) ein Heap-Log.
- `WiFi.setSleep(false)` war bereits an jeder `WiFi.begin()`-Stelle
  aktiv (Stolperfallen-Fix von 29.10.) - keine eigene Aenderung dafuer
  noetig, betrifft ohnehin den Dauerbetrieb, nicht nur den Alexa-Test.
- **Kein Anzeige-Hinweis am Display eingebaut** (bewusst, Risiko-
  Abwaegung): der Auftrag erlaubt das nur "wenn das ohne Layoutrisiko
  geht" - fuer diesen ersten Test genuegt das Serial-Log, ein kurzer
  Display-Hinweis kann bei Bedarf in einem spaeteren, eigenen Auftrag
  ergaenzt werden.

### Schritt 2b: Eigener Funktionsnachweis OHNE Alexa (abgeschlossen)

Vor dem Test mit dem echten Echo-Geraet selbststaendig per Mac-Terminal
verifiziert (siehe CLAUDE.md "Eigenstaendige Seriell-Diagnose" oben):
- `M-SEARCH`-Multicast von einem Python-Skript gesendet -> CYD
  (192.168.178.10) antwortet korrekt mit der Hue-Bruecken-SSDP-Antwort
  (siehe Schritt 1), **zuverlaessig bei jedem Versuch**.
- `curl http://<CYD-IP>/description.xml` -> korrektes Hue-Bridge-XML.
- `curl -X POST http://<CYD-IP>/api` + `curl .../api/dummy/lights` ->
  liefert `{"1":{"type":"Extended color light","name":"Laptop",
  "uniqueid":"00:F8:B3:B7:74:7F:FC:00:00"}}` - Geraet "Laptop" korrekt
  gelistet, ID aus der MAC abgeleitet.
- `curl -X PUT -d '{"on":true}' http://<CYD-IP>/api/.../lights/1/state`
  -> Serial-Log zeigt sofort
  `[Eiswolfs iBoard] Alexa (Test): Befehl AN fuer "Laptop" (Test, keine Aktion)`,
  HTTP-Antwort `[{"success":{"/lights/1/state/on":true}}]` - exakt das
  im Auftrag verlangte Verhalten (Alexa sollte "Okay" sagen, keine
  echte Aktion).
- Heap ueber einen kurzen Zeitraum (~70s) stabil (196.760-197.004 Bytes
  frei) - **laengere Beobachtung (mehrere Stunden) noch nicht
  moeglich**, siehe "WIE WEITER" unten.
- Normalbetrieb (USB-Handshake "IBOARD?"/"IBOARD_OK" mit der Mac-App)
  lief waehrenddessen unverändert weiter.

### Schritt 3: Bauen, flashen - abgeschlossen; Live-Test mit Alex - OFFEN

- `pio run` fehlerfrei mit `ALEXA_TEST` aktiv (siehe Schritt 2), MAC des
  Testgeraets vor dem Flashen per `esptool chip_id` bestaetigt
  (`f8:b3:b7:74:7f:fc`, Port `/dev/cu.wchusbserial31410`, laut CLAUDE.md
  Flightradar-Projekt "immer das iBoard"). Geflasht, danach per
  esptool `hard_reset` neu gestartet und der komplette Boot
  mitgeschnitten - lief fehlerfrei durch (SD-Karte gefunden, 1
  WoL-Geraet geladen, "Alexa (Test): WeMo-Emulation 'Laptop' aktiv,
  IP=192.168.178.10" nach 8,35s).
- `ALEXA_TEST` ist in `config.h` **aktuell bewusst eingeschaltet**
  (nicht auskommentiert) - damit Alex direkt mit dem echten Echo testen
  kann. **Nach Abschluss des Live-Tests unbedingt wieder auskommentieren**
  (zurueck zu `// #define ALEXA_TEST 1`), bevor irgendetwas committet
  oder das Geraet wieder in Normalbetrieb/an einem Stand gezeigt wird.
- Flightradar/Toolbox/Solar-Guardian per `git status`/Verzeichnis
  bestaetigt unveraendert.

### WIE WEITER (Live-Test mit Alex, Testpunkte a-f aus dem Auftrag)

1. In der Alexa-App unter "Geräte" -> "+" -> "Gerät hinzufügen" (oder
   "Alexa, suche neue Geräte" sagen). Erwartet: "Laptop" erscheint
   (Typ Glühbirne/Licht, da fauxmoESP sich technisch als Hue-Lampe
   ausgibt, siehe Schritt 1 - das ist normal und kein Fehler).
2. "Alexa, schalte Laptop an" -> sollte "Okay" sagen, KEINE
   Fehlermeldung. Im Serial-Log erscheint
   `Alexa (Test): Befehl AN fuer "Laptop" (Test, keine Aktion)`.
3. "Alexa, schalte Laptop aus" -> entsprechend mit "AUS".
4. CYD per Strom-Aus/-Ein neu starten, danach OHNE neue Geraetesuche
   nochmal "an"/"aus" testen - sollte weiter funktionieren, KEIN
   doppeltes Geraet in der Alexa-App (dank MAC-basierter uniqueid).
5. Normalbetrieb parallel pruefen: Dashboard mit Mac-Werten, Controls,
   Acer-Button, Ruhezustand mit Display aus/Wecken per Beruehrung.
6. Nach 30+ Minuten nochmal "an" testen - Alexa sollte das Geraet
   weiterhin finden/steuern.
7. Laengere Heap-Beobachtung: Serial-Log zeigt alle 60s den freien
   Heap - bei Gelegenheit ueber mehrere Stunden gegenpruefen
   (sinkt er immer weiter, waere das ein Speicherleck).

Nach erfolgreichem Test: `ALEXA_TEST` zurueck auskommentieren (siehe
oben), ggf. normal weiterflashen. Ergebnis hier unter diesem Abschnitt
nachtragen, NICHT committen/taggen/pushen ohne expliziten Wunsch.

## Auftrag Modularer Listener: Fortschritt (10.11., laufend)

Windows-Listener + Projekt sollen "modular" werden - keine persoenlichen
Werte im Code/in der setup.exe, jeder richtet nach dem Download selbst
ein. NUR `windows/` und Doku duerfen veraendert werden; CYD/Mac-App/
Protokoll nur nach Ruecksprache (siehe unten).

### Schritt 1: Fundstellen-Tabelle (persoenliche/fest eingebaute Werte)

| Fundstelle | Wert | Bedeutung | Vorschlag |
|---|---|---|---|
| `cyd/src/secrets.h` | Token (vorhanden, gitignored) | Gemeinsames HMAC-Geheimnis fuer Fan-Boost/Ruhemodus/Shutdown, fest in die Firmware kompiliert | **Bereits modular** - Datei ist gitignored (Zeile 10 `.gitignore`), nie im Repo. Kein Aenderungsbedarf. |
| `windows/listener/main.go` `cfg.Token` | leer bei Erstanlage | Wird aus `%APPDATA%\EiswolfsIBoard\config.json` gelesen, NIE im Code/in der exe | **Bereits modular.** Nur die Eingabe (Schritt 2) ist heute umstaendlich (Datei von Hand bearbeiten). |
| `windows/listener/main.go:45` `udpPort = 4212` | Konstante | Muss zum CYD-weiten `Config::WIN_SHUTDOWN_UDP_PORT` passen (EIN Protokoll-Port fuer ALLE Geraete, nicht pro Geraet) | In die config.json aufnehmen, Standard 4212, mit Warnhinweis bei Aenderung (CYD kennt nur EINEN festen Port fuer alle Geraete - siehe Kompatibilitaets-Hinweis unten). |
| `windows/listener/main.go` | kein Geraetename-Feld | Fehlt komplett, hilfreich bei mehreren Listenern | Neues optionales Feld `device_name`, nur Anzeige (Tray-Tooltip/Log), keine Protokollbedeutung. |
| `windows/installer/installer.nsi` | `RequestExecutionLevel user`, keine Eingabemaske, keine Firewall-Regel | Setup installiert nur, Alex muss Token/Namen per Hand in config.json eintragen, Firewall-Hinweis nur als Text in einer MessageBox | Neue nsDialogs-Eingabeseite (Schritt 2) + optionale Firewall-Regel (braucht Admin - siehe Rueckfrage unten). |
| `cyd/src/storage.h`/`storage.cpp` (+ `main.cpp` SCREEN_TOUR-Testdaten) | Standardwert `"Acer Predator"` fuer `labelLaptop` (Alt-Feld) bzw. Migrations-Default | **Alex' eigenes Geraet als Code-STANDARDWERT** fuer jeden neuen Nutzer (nicht nur Alex' eigene gespeicherte Daten - die bleiben davon unberuehrt, da ihre config.json den Wert schon explizit enthaelt) | **Rueckfrage an Alex noetig (siehe Chat-Nachricht), CYD-Code bisher NICHT geaendert.** |
| `cyd/tools/generate_ui_font.py`, `preview_controls.py`, `generate_controls_icons.py` | Alex' lokaler Home-Pfad (`/Users/<Benutzername>/...`), "Acer Predator" als Vorschau-Text | Lokale, **nicht committete** Entwickler-Werkzeuge (`git status` zeigt "??", nie `git add`) | Kein Repo-Risiko heute, aber bisher nur durch Zufall nie hinzugefuegt - zur Sicherheit in `.gitignore` aufgenommen (Schritt 3). |
| Restliches Projekt (mac/, docs/, dist/, Beispieldateien) | - | Durchsucht nach Schluesseln/IPs/MACs/Hostnamen/WLAN-Namen/Pfaden | **Keine weiteren Funde.** Kein `config.json`/`secrets.h` im Repo getrackt (`git ls-files` geprueft). |

**Kompatibilitaet bestaetigt**: Acer wird heute per Broadcast
ausgeschaltet (`legacyBroadcastShutdown`-Flag am migrierten Eintrag,
siehe Auftrag "WoL-Geraete"), neue Geraete gezielt per Unicast an ihre
eingetragene IP - BEIDE Wege nutzen denselben Port 4212 und denselben
EINEN CYD-weiten Token. Die neue Listener-Loesung muss daher weiterhin
GENAU diesen einen Token entgegennehmen (kein Schluessel pro Geraet).

**Zwei Rueckfragen an Alex (siehe Chat-Nachricht, vor der Umsetzung
gestellt):**
1. Soll der Code-Standardwert "Acer Predator" (NUR fuer brandneue,
   noch nie konfigurierte Geraete - dein eigenes bleibt unberuehrt) auf
   einen neutralen Platzhalter wie "Laptop" geaendert werden?
2. Die vom Auftrag gewuenschte automatische Firewall-Freigabe braucht
   Admin-Rechte (`netsh advfirewall`) - bisher kommt das Setup OHNE
   Admin-Bestaetigung aus (nur der einmalige Windows-eigene Dialog beim
   ersten Senden). Soll das Setup kuenftig einmalig um Administrator-
   Rechte bitten (UAC-Fenster), um die Regel automatisch anzulegen?

### Schritt 2: Listener modular gemacht

`windows/listener/main.go`: `config`-Struct um `DeviceName`/`Port`/
`Autostart` erweitert (`effectivePort()` faellt auf 4212 zurueck, wenn
Feld fehlt/ungueltig). `listen()` bindet jetzt auf den konfigurierten
Port, stuerzt bei belegtem Port NICHT mehr ab (`log.Fatalf` entfernt) -
loggt stattdessen, zeigt den Fehler im Tray-Status, versucht alle 10s
erneut. Neuer Tray-Menuepunkt "Einstellungen..." (`openSettings()`)
startet die vom Installer mitkopierte `Setup.exe` mit `/SETTINGS`.
Mehrfachstart-Sperre ergaenzt (benanntes Windows-Mutex,
`acquireSingleInstanceLock()`) - noetig, seit das Setup den Listener
nach einer Aenderung neu startet.

`windows/installer/installer.nsi` komplett neu: `RequestExecutionLevel
admin` (nach Ruecksprache), eigene nsDialogs-Einrichtungsseite
(Geraetename/Port/Schluessel mit Datei-Auswahl-Knopf/Autostart-
Haekchen), liest eine BESTEHENDE config.json zuerst ein und fuellt die
Felder damit vor (Update ohne Neueingabe) - `last_cmd_counter`/
`allow_legacy_commands` werden dabei UEBERNOMMEN, nie zurueckgesetzt
(sonst waere der Wiederholungsschutz kurzzeitig ausgehebelt). Legt
automatisch eine Windows-Firewall-Regel an (`netsh advfirewall`, idempotent:
erst loeschen, dann neu anlegen). Kopiert sich selbst als `Setup.exe`
in den Programmordner + neuer Startmenue-Eintrag "...Einstellungen"
(beide starten dieselbe Datei mit `/SETTINGS` - zeigt NUR die
Einrichtungsseite erneut, kein Neu-Installieren). Uninstall entfernt
jetzt auch die Firewall-Regel und die Setup.exe-Kopie.

Neue `%APPDATA%\EiswolfsIBoard\config.json`-Felder (Werte NIE hier
notiert): `device_name` (String), `port` (Zahl), `autostart` (bool) -
zusaetzlich zu den bestehenden `token`/`last_cmd_counter`/
`allow_legacy_commands`.

Go-Kreuzbau + `makensis installer.nsi` beide lokal erfolgreich getestet
(macOS -> Windows, wie bisher).

### Schritt 3: Weitere persoenliche Werte

- `cyd/src/storage.h`/`storage.cpp`: Standardwert `"Acer Predator"` ->
  `"Laptop"` geaendert (Alex' Rueckfrage-Antwort: Ja). Betrifft NUR
  neue, nie konfigurierte Geraete - Alex' eigener gespeicherter Wert
  (ihre config.json hat das Feld explizit gesetzt) bleibt unveraendert.
  CYD gebaut+geflasht (Flash 76,2%/998277 Bytes, RAM 14,9%), Verbindung
  nach ~1s zurueck.
- `cyd/tools/*.py` (Alex' Home-Pfad, "Acer Predator" als Vorschautext):
  waren nie committet (`git status` "??"), jetzt zusaetzlich explizit
  in `.gitignore` aufgenommen (`cyd/tools/`) - kein Verhaltensunterschied,
  nur Absicherung gegen ein versehentliches `git add -A`.
- Keine weiteren Funde (`config.json`/`secrets.h` bereits projektweit
  gitignored, nichts davon im Repo getrackt, geprueft per `git ls-files`).
- NEBENBEFUND (nicht Teil dieses Auftrags, nur notiert): `cyd/src/fonts/
  ui_bold_font.h` und `cyd/src/icon_fan.h` (generierte Font-/Icon-
  Header, von main.cpp zum Bauen gebraucht) sind ebenfalls nie
  committet (`git status` "??") - enthalten keine persoenlichen Werte,
  nur Bitmap-/Font-Tabellen, daher hier nicht angefasst, aber fuer eine
  andere Gelegenheit interessant (ohne sie laesst sich das Projekt
  nicht aus einem frischen Checkout bauen).

### Schritt 4: Anleitung

Neu: `docs/WOL_SETUP.md` (DE/EN im selben Dokument, wie
`docs/DEVELOPMENT.md`) - feste IP (Router-Reservierung zuerst,
FritzBox-Beispiel, Windows/macOS-Ersatzweg), MAC/IP finden, WoL
einschalten (BIOS+Netzwerkkarte, Kabel bevorzugt), private WLAN-Adresse
am Mac aus, Listener einrichten, Fehlersuche. Aus README.md und
`docs/DEVELOPMENT.md` (DE/EN) verlinkt. `windows/README.md` auf den
neuen Einrichtungsdialog/Firewall-Automatik/Einstellungen-Wiederaufruf
aktualisiert.

### Schritt 5: USB-Stick, Tests

- USB-Stick "EISWOLF" (30,9 GB, `/Volumes/EISWOLF`) identifiziert -
  NICHT das zweite externe Laufwerk "Daten" (1,9 TB, kein Stick).
  Alte `EiswolfsIBoardListener-Setup.exe` (03.11.) mit Alex' Freigabe
  geloescht. Neue Datei `EiswolfsIBoardListener-Setup-2026-11-10.exe`
  (kein bisheriges Versionsschema fuer den Listener vorhanden, daher
  Datum als Versionskennzeichnung) + `Anleitung.txt` draufgelegt,
  Pruefsumme nach dem Kopieren verglichen (identisch:
  `0bf67eeff6d066c420fa8c5860397c899da77cf204dbd1ece25303445c017f88`),
  Stick sicher ausgeworfen.
- Flightradar/Solar-Guardian per `git status` unveraendert (leer).
- [x] Test a) mit Alex live durchgefuehrt, dabei DREI echte Bugs
      gefunden und behoben (siehe Chat 10.11., alle Fixes neu gebaut +
      auf den Stick kopiert + Pruefsumme verglichen):
      1. **Datei-Sperre beim Update**: `File "EiswolfsIBoardListener.exe"`
         schlug fehl, weil der ALTE Listener noch lief (gesperrte Datei).
         Fix: `taskkill /IM ... /F` + kurze Pause VOR dem Ueberschreiben.
      2. **Leeres Token-Feld loeschte einen bestehenden Schluessel**:
         Ein abgebrochener/uebersprungener erster Versuch (wegen Bug 1)
         schrieb trotzdem die config.json mit leerem Token - echter Mac-
         App-Token ging dabei verloren (per `mac/.../config.json` lokal
         wiederhergestellt und als `token-fuer-windows.json` auf den
         Stick gelegt). Fix: ein leeres Token-Feld behaelt jetzt IMMER
         den bisherigen Wert (`${If} $NewToken == "" -> $ExistingToken`),
         loescht nie mehr etwas durch blosses Leerlassen.
      3. **Kaputte Stack-Verwaltung in `ExtractQuotedField`/
         `ExtractNumberField`**: Push/Exch/Pop war nicht balanciert,
         Stack-Reste summierten sich ueber mehrere Feld-Extraktionen
         auf und verfaelschten spaetere Felder (u.a. "token" selbst,
         bestaetigt per `type config.json` am Windows-Laptop: Datei
         hatte wirklich `"token": ""` auf der Platte, kein reines
         Anzeigeproblem). Komplett umgeschrieben auf feste Variablen
         (`JsonInPath`/`JsonInText`/`JsonInPattern`/`JsonOutValue`)
         statt Stack-Jonglage - nachvollziehbar korrekt balanciert.
      NEBENBEI gefunden+behoben: CYD-Tastatur hatte KEINEN Punkt "."
      auf irgendeiner Ebene (nur ":") - die IP-Eingabe fuers Ausschalten
      war dadurch unmoeglich. `ROW_DIGITS`/`ROW_SYMBOLS` auf 12 Spalten
      erweitert (setup_flow.cpp). UND: `runWolDeviceEntryFlow()`/
      `runKeyboard()`/`runTextEntry()` um ein `initialText`-Vorbelegen
      erweitert (Alex: "wenn ich auf einen existierenden Namen tippe,
      moechte ich nur etwas aendern") - CYD dafuer zweimal neu geflasht.
      **Ergebnis bestaetigt: Acer startet (WoL) UND schaltet sich aus
      (Shutdown) wieder wie vor dem Auftrag.**
- [x] Test b) mit Alex live bestätigt, dabei VIERTER Bug gefunden+
      behoben: Taskleisten-Menuepunkt "Einstellungen..." tat NICHTS
      (Log: "fork/exec ...Setup.exe: Der angeforderte Vorgang
      erfordert erhoehte Rechte") - `exec.Command().Start()` nutzt
      CreateProcess, das NIE automatisch UAC-erhoeht; der Listener
      selbst laeuft bewusst OHNE Admin, Setup.exe braucht aber Admin
      (Firewall-Regel). Fix: `shellExecuteRunAs()` (eigener
      `ShellExecuteW`-Aufruf mit "runas"-Verb statt exec.Command) -
      loest das UAC-Fenster jetzt korrekt aus. Danach von Alex
      bestätigt: Einstellungen oeffnen sich, Port/Gerätename änderbar,
      echter Schlüssel korrekt als "48 Zeichen" erkannt (bestätigt
      erneut, dass Bug 3 wirklich behoben ist).
- [x] Test c) (Windows-Neustart, Autostart, Funktion danach) mit Alex
      durchgefuehrt. Autostart hat funktioniert (Taskleisten-Symbol kam
      von selbst). Danach zunaechst KEINE Reaktion auf Ausschalten -
      gemeinsam eingegrenzt (Firewall-Regel korrekt vorhanden, Profil
      auf Alex' Wunsch von "Oeffentlich" auf "Privat" umgestellt,
      Firewall testweise komplett aus, IP unveraendert) bis zu einem
      eigenen Testpaket von diesem Mac (siehe Test e) - danach
      funktionierte es wieder. Einordnung: ein kurzzeitiger Netzwerk-
      Haenger nach dem Windows-Neustart (veraltete ARP-/Switch-Tabelle,
      ausserhalb unseres Codes) - kein Bug in Listener/Installer/CYD.
      Firewall-Profil-Umstellung von Oeffentlich auf Privat bleibt als
      zusaetzliche, sinnvolle Haertung bestehen (Windows klassifiziert
      ein Heimnetz nach einem Neustart manchmal zunaechst als
      "oeffentlich" - ohne "Privat" greift die automatisch angelegte
      Firewall-Regel nicht, Alex jetzt dafuer sensibilisiert).
- [x] Test e) (falscher Schluessel -> Logzeile) bereits ueber das
      Diagnose-Testpaket bestätigt: `2026/11/10 03:01:48 Befehl mit
      ungueltiger Signatur verworfen (falscher Schluessel oder kein
      bekanntes Format)` - exakt wie vom Auftrag gefordert.
- [x] ZUSAETZLICHER Test (Alex-Initiative): kompletter zweiter Neustart-
      Zyklus (herunter- und wieder hochgefahren) - Einschalten (WoL)
      UND Ausschalten liefen diesmal SOFORT sauber durch, kein erneuter
      Netzwerk-Haenger. Bestätigt: Test c)s fruehere Verzoegerung war
      wirklich ein einmaliger Ausreisser, kein reproduzierbares Problem.
- [x] Test d) (Deinstallieren+Neu einrichten) mit Alex live durchgefuehrt.
      Deinstallation sauber bestaetigt (Firewall-Regel korrekt entfernt,
      `netsh advfirewall firewall show rule` -> "Keine Regeln stimmen
      mit den angegebenen Kriterien ueberein"). Nach Neuinstallation
      (Token erneut ueber `token-fuer-windows.json` eingelesen) zunaechst
      wieder KEINE Reaktion auf Ausschalten trotz korrekt vorhandener
      Firewall-Regel (Profil Privat, UDP 4212, Programm-Pfad exakt
      passend zu `$INSTDIR\EiswolfsIBoardListener.exe` - per
      `netsh ... verbose` verglichen, Alex' eigener Verdacht, stimmte
      aber ueberein) und sichtbarem Taskleisten-Icon. Diagnose-Testpaket
      vom Mac direkt an die IP (Unicast) kam beim Listener an (als
      "ungueltige Signatur" geloggt) - Netzwerkpfad also grundsaetzlich
      offen. ABER: eigene CYD-Pakete kamen trotzdem nicht an, obwohl ein
      per USB-Protokoll mitgelesener Diagnose-Log direkt im CYD bestaetigte,
      dass es ein (korrekt adressiertes) Unicast-Paket tatsaechlich
      abschickte.

      **Fuenfter Fix (CYD-seitig, nach Ruecksprache mit Alex umgesetzt -
      "ja bitte")**: Der migrierte Acer-Eintrag sendete Ausschalten
      IMMER per Broadcast (`255.255.255.255`, `legacyBroadcastShutdown`
      blieb dauerhaft `true`, auch wenn man im Bearbeiten-Dialog
      zusaetzlich eine IP eintraegt - die IP wurde bisher nie benutzt).
      Alex' eigene Vermutung: manche Router/Switches behandeln die
      globale Broadcast-Adresse `255.255.255.255` anders (strenger) als
      eine gezielte Unicast-Adresse oder die subnetz-eigene Broadcast-
      Adresse (`255.255.255.0`-Maske -> z.B. `192.168.178.255`) - exakt
      das deckt sich mit dem beobachteten Verhalten (Unicast vom Mac
      kam an, Broadcast vom CYD nicht). Fix in
      `cyd/src/main.cpp::handleDevicesPage()`: sobald beim Bearbeiten
      eines Geraets eine IP eingetragen/bestaetigt wird, wechselt
      `legacyBroadcastShutdown` automatisch auf `false` - der Acer nutzt
      ab sofort gezielten Unicast (wie neue Geraete auch), sobald einmal
      kurz der Bearbeiten-Dialog bestaetigt wurde (bei Alex bereits
      geschehen). Verifiziert per temporaerer (wieder entfernter)
      Seriell-Diagnose direkt im Listener-Code: `legacy=0
      ip='192.168.178.9' wifi=1` + tatsaechlich abgeschicktes Unicast-
      Paket, bestaetigt. **Danach funktionierte der erste Ausschalt-
      Versuch sofort** - vermutlich derselbe Netzwerk-Haenger-Effekt
      wie in Test c) (veraltete ARP-Tabelle direkt nach Neuinstallation/
      Prozessneustart), diesmal zusaetzlich durch den Wechsel auf
      Unicast dauerhaft geloest (Unicast ist unabhaengig von einer
      moeglichen Broadcast-Eigenart des Routers generell robuster).
      Betrifft NUR den Acer-Eintrag; neu angelegte Geraete nutzten
      ohnehin schon immer Unicast. CYD zweimal neu geflasht (einmal mit
      Diagnose-Ausgabe, danach bereinigt ohne).

### Broadcast-Regression pruefen (10.11., auf Alex' ausdruecklichen
    Verdacht hin: koennte am NEUEN Listener/seiner Firewall-Regel
    liegen statt am Router)

- [x] **Bindung verglichen (alt vs. neu)**: Beide Versionen binden
      identisch an `net.UDPAddr{Port: port, IP: net.IPv4zero}`
      (`0.0.0.0`, alle Schnittstellen) - KEIN Unterschied, war schon
      vorher so (`git show ce0a219:windows/listener/main.go`). Keine
      Ursache hier.
- [x] **Firewall-Regel alt vs. neu verglichen**: Der ALTE Installer
      (Commit `ce0a219`) legte GAR KEINE eigene Firewall-Regel an -
      Alex muss den automatischen Windows-Dialog ("Windows Defender
      Firewall hat den Zugriff blockiert") irgendwann mit "Zulassen"
      bestaetigt haben, was eine automatische Regel mit Profilen
      Privat+Domaene, Quelladresse "Beliebig" erzeugt - vom Prinzip her
      SEHR aehnlich zur neuen, explizit per `netsh` angelegten Regel
      (Profil Privat, Quelladresse "Beliebig", siehe Test d) oben).
- [x] **Empirisch getestet (ausschlaggebend)**: Zwei Broadcast-
      Testpakete vom Mac an den AKTUELL LAUFENDEN neuen Listener
      gesendet - einmal an die globale Adresse `255.255.255.255` und
      einmal an die netzeigene gerichtete Adresse `192.168.178.255`
      (beide Port 4212, `SO_BROADCAST` gesetzt). **BEIDE kamen an**
      (von Alex bestaetigt: "ja 2 mal ungueltige sig" im Protokoll).
      **Ergebnis: Der neue Listener/seine Firewall-Regel blockiert
      Broadcast NICHT - keine Regression im Listener/Installer.** Die
      urspruengliche Stoerung (CYD-Broadcast kam nach der
      Neuinstallation nicht an) war damit eher CYD-seitig/netzwerk-
      seitig (z.B. derselbe kurzzeitige ARP-/Switch-Haenger wie in
      Test c) - durch die IP-Vorrang-Aenderung unten zusaetzlich
      dauerhaft entschaerft, unabhaengig von der genauen Ursache.
- [x] Netzwerkprofil/Firewall-Regel von Alex bestaetigt (PowerShell 7
      hatte bei `Get-NetFirewallRule` einen bekannten Kompatibilitaets-
      Bug mit `-DisplayName`, Fehlermeldung bezog sich faelschlich auf
      "InstanceID" - stattdessen `netsh advfirewall firewall show rule
      ... verbose` genutzt, liefert dieselbe Information): Firewall-
      Regel korrekt (Aktiviert Ja, Eingehend, Profil Privat, UDP 4212,
      Programm-Pfad exakt passend, Zulassen). `Get-NetConnectionProfile`
      bestaetigt zusaetzlich: aktive Verbindung ("Gigabit Turbo 5 GHZ",
      Ethernet) hat `NetworkCategory: Private`. Beides einwandfrei -
      bestaetigt nochmal, dass der neue Listener/die Firewall-Regel
      keine Regression ist (deckt sich mit dem empirischen Broadcast-
      Test oben).
- [x] **Quelladresse pro empfangenem Paket mitgeloggt** (`main.go`):
      `handleLine()`/`runShutdown()` bekommen jetzt die Absenderadresse
      als Parameter (aus `conn.ReadFromUDP()`, bisher verworfen) und
      haengen sie an JEDE Logzeile an (gueltig, ungueltig, Token
      fehlt, RESYNC, Zaehler-Reject, Legacy) - NIE den Token selbst.
      Dadurch sofort sichtbar, ob ueberhaupt ein Paket (von welcher
      Adresse) ankommt, auch bei Ablehnung.
- [x] **Schritt 3 (IP hat immer Vorrang)**: `cyd/src/main.cpp::
      sendDeviceShutdown()` umgebaut - ist eine IP eingetragen, wird
      IMMER sofort gezielt an diese gesendet (auch bei einem
      migrierten/"legacy"-Eintrag, auch OHNE vorheriges Bearbeiten/
      Bestaetigen des Eintrags). Broadcast bleibt NUR Ersatz fuer
      Eintraege ganz ohne IP. Bedienung (Tippen/Halten, Screens, Optik)
      unveraendert - reine interne Logik-Aenderung. CYD neu gebaut +
      geflasht (Flash: 998.505 Bytes / 76,2%).
- [x] Doku ergaenzt: `docs/WOL_SETUP.md` (DE+EN) - Schritt 5 empfiehlt
      jetzt ausdruecklich eine eingetragene IP (zuverlässiger als
      Broadcast), Fehlersuche-Abschnitt um die neue Quelladress-Logzeile,
      Netzwerkprofil-Pruefung (`Get-NetConnectionProfile`) und einen
      eigenen Broadcast-vs-Unicast-Hinweis erweitert.
- [x] Listener + Installer neu gebaut (`go build -trimpath -buildvcs=false
      -ldflags "-H=windowsgui -s -w"` + `makensis`), auf den Stick
      kopiert (`EiswolfsIBoardListener-Setup-2026-11-10.exe` ueberschrieben),
      Pruefsumme nach dem Kopieren identisch verglichen:
      `6a50a7d15ddba9443d9fd7d46ffe02f4d8c5d82e2b5c76f2222aaa0ab6690857`.
- [x] Abschlusstest mit Alex:
      - **a) Ausschalten mit eingetragener IP**: Nach Update auf den
        neuen Listener zunaechst (einmalig) keine Reaktion trotz
        offenem Pfad (eigenes Unicast-Testpaket kam an, CYD-Paket
        zunaechst nicht) - zweiter Versuch lief sofort durch
        ("jetzt faehrt er runter"). Gleiches bekanntes Transient-
        Verhalten wie zuvor in Test c) des vorherigen Auftrags, kein
        neuer Bug.
      - **b)**: Neue setup.exe ("neu drueber installiert") +
        Wiederholung von a) erfolgreich, siehe oben.
      - **c) Broadcast-Weg isoliert getestet**: IP am Acer-Eintrag
        entfernt (Name/MAC blieben) - dabei EIGENEN Bug gefunden:
        Zeigte "Ausschalten nicht eingerichtet" statt zu broadcasten,
        weil der fruehere Fix (legacyBroadcastShutdown beim Bearbeiten
        mit IP auf false setzen) das Broadcast-Flag beim ERSTEN
        Bearbeiten mit IP dauerhaft geloescht hatte - ohne Weg, es
        ueber die Oberflaeche je wieder auf true zu setzen. Sobald die
        IP dann entfernt wurde, blieb gar kein Versandweg mehr uebrig.
        **Fix**: `sendDeviceShutdown()` bindet den Broadcast-Ersatzweg
        jetzt an Geraete-Platz 0 (`deviceIdx == 0`, der feste Platz des
        urspruenglich migrierten Eintrags) statt an das mutable Flag;
        die ueberfluessige (und jetzt als Ursache erkannte) Flag-
        Loeschung beim Bearbeiten in `runWolDevicesScreen()` entfernt.
        CYD neu geflasht. Danach bestaetigt: Broadcast funktioniert
        wieder ("faehrt runter!"). IP danach wieder eingetragen.
      - **d) Laptop-Neustart**: Listener startete automatisch (Taskleisten-
        Symbol da), Ausschalten mit eingetragener IP lief danach sofort
        sauber durch ("geht!"). Bestaetigt.
- [x] Gesamtbericht geschrieben (siehe Chat 10.11.).

## Auftrag WoL-Geräte: Fortschritt (09.11., laufend)

Mehrere WoL-Geräte statt nur dem Acer (bis zu 4 insgesamt). NUR `cyd/`
betroffen, NICHT committen/taggen/pushen.

### Schritt 1: Fundstellen (nur gelesen)

- **Speicherung heute**: `deviceCfg.wolMacAddress`/`labelLaptop`
  (`storage.h`), gelesen/geschrieben in `storage.cpp::parseJson()`/
  `toJson()` (SD, `StaticJsonDocument<896>`) UND `loadFromPrefs()`/
  `saveToPrefs()` (NVS-Fallback) - additiv, Default beim Fehlen des
  Feldes.
- **WoL senden**: `sendWolPacket()` (~Zeile 580) - reines Magic-Packet-
  Broadcast an Port 9, adressiert NUR ueber die Ziel-MAC (keine IP
  noetig, funktioniert fuer beliebig viele Geraete unveraendert).
- **Ausschalten senden**: `sendShutdownCommand()` (~Zeile 604) -
  signierter "SHUTDOWN"-Befehl (`CmdAuth::signCommand()`) per
  **Broadcast** an `Config::WIN_SHUTDOWN_UDP_PORT`=4212. **Wichtiger
  Befund**: der Windows-Listener (`windows/listener/main.go`) bindet
  per `net.ListenUDP` nur auf den Port, unterscheidet NICHT zwischen
  Broadcast/Unicast-Empfang - ein gezielter Unicast an eine bestimmte
  IP braucht daher KEINE Aenderung am Listener/Protokoll, nur eine
  andere Zieladresse beim Senden (siehe Nachricht an Alex).
- **Controls-Kachel/Icon**: `CtrlTileIdx`/`ctrlTileRects[4]`
  (146x80, Positionen (10,40)/(164,40)/(10,128)/(164,128)),
  `drawCtrlTileBase()`/`drawCtrlIcon()`/`drawCtrlOutline()`/
  `drawCtrlAcerIdle()`, Tap/Halten in `handleControlsPage()`.
- **"Keine Verbindung"-Button**: aus Vorauftrag, `acerNoConnButtonRect`/
  `drawNoConnAcerButton()`/`startNoConnAcerFeedback()` + Touch-Block im
  `showingNoConnection`-Zweig von `loop()`.
- **Seiten/Wischen**: `enum class Page { Main, Controls }` (bisher NUR
  2 Seiten!) - `drawPageDots()` (~Zeile 2049) zeichnet bisher FEST 2
  Punkte an hartkodierten Positionen (150,228)/(170,228) - muss fuer
  2 ODER 3 Seiten verallgemeinert werden.
- **WLAN-Liste als Vorbild** (Flightradar, nur gelesen, nicht
  veraendert): `wifi_manage_screen.cpp` - Zeilen mit Name+rotem "X",
  "Hinzufuegen"-Button, der ab Maximalzahl verschwindet. ABWEICHUNG
  bewusst: Flightradar loescht beim X SOFORT ohne Rueckfrage - der
  neue iBoard-Auftrag verlangt ausdruecklich eine Ja/Nein-Rueckfrage,
  also eigene (nicht kopierte) Bestaetigungslogik.
- **Name-/MAC-Eingabe**: `SetupFlow::runTextEntry()`/`runKeyboard()`
  (setup_flow.cpp) - hat bereits eine Umlaut-Ebene (`ROW1_UMLAUT`) ueber
  den "#+-"-Umschalter, direkt wiederverwendbar fuer Geraete-Namen.

**Hinweis an Alex zum Ausschalten (Schritt 6, siehe Chat-Nachricht):**
Entscheidung: Acer (migrierter Eintrag) behaelt unveraendertes
Broadcast-Verhalten (`legacyBroadcastShutdown`-Flag, NUR bei der
Migration gesetzt). Neue Geraete bekommen eine OPTIONALE IP beim
Hinzufuegen/Bearbeiten - vorhanden -> gezielter Unicast-Shutdown, fehlt
-> Halten zeigt kurz "Ausschalten nicht eingerichtet", sendet nichts.
Keine Aenderung am Windows-Listener/Protokoll noetig.

### Schritt 2: Datenmodell und Migration

`Storage::WolDevice` (storage.h): `name`/`mac`/`color` (RGB565, einmalig
zufaellig vergeben)/`shutdownIp` (optional)/`legacyBroadcastShutdown`
(nur beim migrierten Eintrag true). `DeviceConfig::wolDevices[4]` +
`wolDeviceCount`. SD-JSON: neues Array `wol_devices`
(`StaticJsonDocument` 896->2560 Bytes), NVS-Fallback: flache Keys
`wol_cnt`/`wol{i}_name`/`wol{i}_mac`/`wol{i}_col`/`wol{i}_sip`/
`wol{i}_leg`. Migration zentral in `Storage::load()` (`migrateWolDevices()`,
NICHT in `parseJson()`/`loadFromPrefs()` einzeln) - nur wenn
`wolDeviceCount==0` UND eine alte `wol_mac` existiert, Name aus
`labelLaptop`, Farbe `0xFFE0` (TFT_YELLOW, das bisherige Acer-Gelb),
`legacyBroadcastShutdown=true`. KEIN SD-Schreiben in `load()` selbst -
erst der naechste normale `Storage::save()` (z.B. durch eine beliebige
andere Einstellungsaenderung) persistiert das Ergebnis, wie gefordert.
Abwaertskompatibilitaet: `wol_mac`/`label_laptop` werden beim Speichern
weiterhin mitgeschrieben, gespiegelt aus Eintrag 1 der neuen Liste.
Palette fuer NEUE Geraete (`WOL_NEW_DEVICE_PALETTE`): Orange/Pink/
Skyblue/Magenta - bewusst ohne Gelb/Tuerkis/Gruen/Violett (bestehende
Kacheln) und ohne Rot (Ausschalten-Rueckmeldefarbe). `pickRandomDeviceColor()`
vermeidet Dopplungen mit bereits vergebenen Farben, erlaubt Wiederholung
bei Erschoepfung.

### Schritt 3: Einstellungen "WoL-Geraete"

Menuepunkt umbenannt (gleicher `StringId::SETUP_WOL_MAC`-Slot, dient wie
`SETUP_BRIGHTNESS` als Button-Label UND Bildschirmtitel). Neue Liste
(`runWolDevicesScreen()`): Name+kurze MAC je Zeile, rotes "X" rechts
(eigene `confirmYesNo()`-Rueckfrage, NICHT wie Flightradars WLAN-Liste
ohne Rueckfrage sofort loeschen, siehe Schritt 1), "Geraet hinzufuegen"
unten (verschwindet ab 4). Tippen auf eine Zeile -> Bearbeiten (gleiche
`runWolDeviceEntryFlow()`-Funktion wie Hinzufuegen, keine Kopie): Name
(`SetupFlow::runTextEntry()`, hat bereits die Umlaut-Tastaturebene),
MAC (dieselbe Pruef-/Fehlermeldungs-Logik wie bisher, nur jetzt mit
sichtbarer `WOL_INVALID_MAC`-Meldung statt stillem Verwerfen), optionale
Shutdown-IP (Zurueck = ueberspringen, behaelt beim Bearbeiten den
bisherigen Wert bei statt ihn zu loeschen). Leerer Name wird abgelehnt
(`WOL_EMPTY_NAME`).

### Schritt 4: Controls, Seite "Geraete", Wischen

`Page` um `Devices` erweitert. Controls' Kachel 1 (bisher fest "Acer")
liest jetzt `deviceCfg.wolDevices[0]`, bleibt bei leerer Liste komplett
ungezeichnet (`drawControlsStaticLayout()`). Neue Seite `Devices`
(`drawDevicesStaticLayout()`/`handleDevicesPage()`) nur per Wischen von
Controls erreichbar (`wolDeviceCount>=2`), zeigt Geraete 2-4 an exakt
denselben drei Positionen wie Controls' Acer/Mac/Luefter-Kacheln (eigenes
`deviceTileRects[3]`, NICHT `ctrlTileRects[]` wiederverwendet - das
haette sonst faelschlich das Luefter-/Mac-Symbol statt des Power-Symbols
gezeigt, da `drawCtrlIcon()` nach Kachel-INDEX statt Zweck unterscheidet).
`drawPageDots()` verallgemeinert auf 2 oder 3 Punkte (bei 2 Seiten exakt
dieselben Positionen wie vorher, bei 3 symmetrisch erweitert), `currentPage`-
abhaengig statt fest "Controls aktiv".

### Schritt 5: "Keine Verbindung zum Mac"

Buttonspalte rechts (`noConnDeviceButtonRect(slot)`): Geraet 0 unten
(y=200, wie bisher), weitere darueber (34px Abstand). WLAN/IP/Status-
Zeilen links auf 164px Breite verengt, sobald mindestens ein Geraet
existiert (reicht fuer bis zu 4 Buttons uebereinander, geprueft im
SCREEN_TOUR-Lauf mit 4 Geraeten + langem WLAN-Namen, 0 Verstoesse).
Leere Liste: kein Button, volle Textbreite wie vor diesem Auftrag.

### Schritt 6: Ausschalten pro Geraet

Wie in der Chat-Nachricht nach Schritt 1 erklaert: `legacyBroadcastShutdown`
(nur migrierter Acer) -> `sendShutdownCommand()` unveraendert (Broadcast).
Neue Geraete -> `sendDeviceShutdown()` sendet gezielten Unicast an die
eingetragene `shutdownIp` (Port 4212, identisches `CmdAuth::signCommand()`-
Protokoll - der Windows-Listener unterscheidet beim `net.ListenUDP`-Empfang
nicht zwischen Broadcast/Unicast, siehe `windows/listener/main.go`,
KEINE Aenderung dort noetig). Ohne IP: Halten zeigt kurz
"Ausschalten nicht eingerichtet" (`showDeviceNotSetUp()`), sendet nichts.

### Gemeinsame Tap/Halten-Logik (`handleDeviceWidgets()`)

EINE Funktion fuer alle Geraete-Buttons auf allen drei Screens (Controls-
Kachel 1, Geraete-Seiten-Kacheln 2-4, "Keine Verbindung"-Buttons) -
"rects"/"deviceIdx" sind parallele Arrays (welches Rect gehoert zu
welchem `wolDevices[]`-Index), "style" waehlt Kachel- oder Button-Optik
(`DeviceWidgetStyle::Tile`/`Button`, gemeinsame `drawDeviceWidget()`/
`flashDeviceWidget()`). EIN gemeinsamer `DeviceTouchState` (nicht je
Geraet) reicht, weil das Touch-Panel ohnehin nur eine Beruehrung
gleichzeitig kennt - Rueckmeldung/3s-Sperre dagegen SIND je Geraet
(`deviceFbColor[]`/`deviceTapLockedUntilMs[]`), da z.B. zwei
uebereinanderliegende Buttons auf "Keine Verbindung" gleichzeitig in
einem laufenden Feedback-Fenster stecken koennen. Ersetzt die komplette
fruehere Acer-spezifische Logik (`acerFb`/`acerTapLockedUntilMs`/
`acerLongFired`/`drawCtrlAcerIdle()`/`drawNoConnAcerButton()`/
`startNoConnAcerFeedback()`, allesamt entfernt).

### Schritt 7+8: Pruefung, Bauen, Flashen

- LAYOUT_DEBUG/SCREEN_TOUR erweitert (0/1/2/4 Geraete je Bildschirm-Typ,
  DE+EN, langer Name "Müller-Äußerst Laptop"): **0 Verstoesse** (20
  Bildschirme durchlaufen, per `pyserial` live mitgelesen, siehe Chat).
  Hauptscreen/Bootscreen/Mac-Mini-M4-/Luefter-/Menue-Kachel optisch
  nicht angefasst (nur Acer-Kachel-Quelle generalisiert, exakt gleiche
  Zeichenfunktion/-logik).
- `pio run` + Flashen (final, Debug AUS): Flash 76,2% (998433 Bytes),
  RAM 14,9% (48792 Bytes). Verbindung nach ~1s wieder da.
- Neue Boot-Diagnosezeile (`setup()`): zeigt WoL-Geraeteanzahl +
  Name/MAC von Geraet 1 - genutzt, um Alex' migrierte Acer-MAC zu
  sichern, OHNE die SD-Karte selbst auslesen zu koennen (kein
  Datei-Zugriff vom Mac aus moeglich, nur USB-Seriell): **Geraet 1:
  "Acer Predator", 78:F2:9E:F4:6B:8D** (per Reflash+sofortigem
  `pyserial`-Mitlesen abgefangen, da die Mac-App den Port sonst
  schneller uebernimmt als eine zweite Verbindung sich oeffnen laesst -
  bekannte Einschraenkung aus fruehren Auftraegen).
- Flightradar/Solar-Guardian per `git status` unveraendert (leer).
- [x] Test a) von Alex bestätigt (10.11.): Controls/"Keine Verbindung"
      unverändert, Acer funktioniert ohne Neueingabe.
- [x] Test b) von Alex bestätigt (Foto): Liste zeigt "Acer Predator"
      mit MAC, "Gerät hinzufügen" darunter.
- [x] Test c) von Alex bestätigt (nach einem Fix): "Test 1" mit Acer-
      MAC hinzugefügt, IP-Schritt übersprungen -> 3 Seitenpunkte,
      Seite "Geräte" mit 1 Kachel, "Keine Verbindung" zeigt "Test1" +
      "Acer Predator" uebereinander ohne Ueberlappung. BUG gefunden+
      behoben: die WoL-Geräte-Liste zeigte Name+MAC in EINER Zeile/
      EINEM UiText::draw()-Aufruf verkettet - der lange Gesamttext
      zwang UiText::draw() auf die kleinste Schriftstufe, auch fuer
      den Namen (wirkte "anders"/"weniger schoen" als ueberall sonst).
      Fix: Name und MAC jetzt GETRENNTE Boxen (ROW_H 30->40px, Name
      oben gross/fett wie sonst, MAC darunter klein/grau) - neu
      geflasht, von Alex bestätigt ("jetzt passts").
- [x] Test d) von Alex bestätigt (nach einem Fix): Tippen/Halten auf
      der "Test1"-Kachel funktioniert. BUG gefunden+behoben: Die
      "Ausschalten nicht eingerichtet"-Meldung auf einer Kachel nutzte
      `pickClearFont()+drawString()` statt `UiText::draw()` - der lange
      Text ("Ausschalten nicht eingerichtet") lief dadurch sichtbar
      links/rechts ueber die 146px-Kachel hinaus (Foto von Alex:
      graue Textreste ausserhalb des Kachelrahmens). Fix: beide Stile
      (Kachel+Button) nutzen jetzt `UiText::draw()`, das echt kuerzt/
      schrumpft statt nur eine Schriftgroesse zu waehlen. Ursprünglich
      als "nur oberer Bereich reagiert" gemeldeter Effekt war davon
      getrennt nur die normale 500ms-Halteschwelle (dieselbe wie beim Acer
      seit jeher) - ein zu lang gehaltener "Tipp" wird als Halten
      gewertet, kein Bug, nach Hinweis von Alex selbst bestätigt
      ("passt!" bei kurzem Tippen).
- [x] Test e) bereits beim Test c)-Foto mitbestätigt: "Keine Verbindung"
      zeigt "Test1" über "Acer Predator", keine Überlappung.
- [x] Test f) von Alex bestätigt (Fotos, 4 Geräte "Acer Predator"/
      "Test1"/"Karl is awsome"/"Karl ftw"): "Gerät hinzufügen"
      verschwindet bei 4, Seite "Geräte" zeigt 3 Kacheln in
      unterschiedlichen Farben, "Keine Verbindung" zeigt alle 4 Buttons
      übereinander ohne Überlappung, alle funktional getestet ("sogar
      auf Funktion getestet. Alles geht!").
      NEBENBEI (nicht Teil dieses Auftrags, nur notiert): Alex meldet,
      dass bei dunklem Display oft mehrfach angetippt werden muss,
      bis es angeht - gehört zum frueheren "Ruhezustand-Fix"/
      "Display aus im Ruhezustand"-Auftrag, hier NICHT angefasst/
      untersucht, da ausserhalb des Umfangs "WoL-Geraete".
- [x] Test g) von Alex bestätigt (Fotos): "Karl is awsome" ->
      "Karl Büro" umbenannt (Umlaut), überall korrekt aktualisiert
      (Liste, Geräte-Seite, Farbe blieb erhalten).
- [x] Test h) von Alex bestätigt: "Karl ftw" per rotem X gelöscht, Ja/
      Nein-Rückfrage kam zuerst, Button verschwand überall ("Keine
      Verbindung" inklusive).
- [x] Test i) von Alex bestätigt (Foto): alle Geräte gelöscht ->
      Controls-Kachel 1 bleibt leer (Mac/Lüfter/Menü unverändert), nur
      2 Seitenpunkte. Legt den Acer gerade mit der gemerkten MAC wieder
      an.
- [x] Test j) von Alex bestätigt: Acer nach CYD-Neustart weiterhin da
      und funktionsfähig - SD-Speicherung bestätigt.
- [x] Test k) von Alex bestätigt: Englisch ("WoL devices", Controls)
      funktioniert einwandfrei.

**Alle Live-Tests a)-k) von Alex bestätigt (10.11.) - Auftrag inhaltlich
abgeschlossen**, inkl. zweier waehrend der Tests gefundener und noch
selbst behobener Bugs (Zeilen-Schriftgroesse in der Liste, Text-
Overflow bei "Ausschalten nicht eingerichtet" auf Kacheln).
README.md + docs/DEVELOPMENT.md (DE/EN) ergaenzt: bis zu 4 Geraete,
Verwaltung unter Einrichtung/Setup -> "WoL-Geraete"/"WoL devices",
Seite "Geraete"/"Devices", Hinweise fuer einen neuen Windows-Laptop
(Listener installieren, WoL in BIOS+Netzwerkkarte aktivieren, Kabel
statt WLAN bevorzugen, feste IP im Router fuers Ausschalten).
Nichts committet/getaggt/gepusht.
- [ ] README/docs (DE/EN) Satz ergaenzen - noch offen.



Neuer Auftrag: auf dem Screen "Keine Verbindung zum Mac" soll zusätzlich
ein Button für den Acer (Wake-on-LAN/Shutdown) erscheinen - Alex will
morgens nur den Laptop starten, ohne den Mac zu wecken. NUR `cyd/`
betroffen, NICHT committen/taggen/pushen. Hauptscreen/Controls/
Bootscreen bleiben optisch unverändert.

### Schritt 1: Fundstellen (main.cpp)

- **Controls-Acer-Kachel**: Tap = `sendWolPacket()` (ueber
  `TouchCalib::Gesture::Tap`, Zeile ~2169), Halten `Config::
  FAN_LONGPRESS_MS`=500ms = `sendShutdownCommand()` (eigener
  Raw-Touch-Block mit `pressActive`/`maxMove`/`acerLongFired`, Zeile
  ~2216-2266, exakt das Muster der fruehren CPU-Temp-Kachel). Beide
  Wege teilen sich `acerTapLockedUntilMs`/`CTRL_TAP_LOCKOUT_MS`=3000ms.
  `acerLongFired` verhindert, dass das Loslassen nach einem Halten
  zusaetzlich noch als Tap (WoL) gewertet wird - `TouchCalib::
  pollGesture()` kennt selbst KEINE Haltedauer, sie unterscheidet nur
  beim Loslassen zwischen Tap/Swipe anhand der Bewegung, nicht der Zeit.
  Label `deviceCfg.labelLaptop`, bei fehlender `wolMacAddress` zeigt
  `drawCtrlAcerIdle()` stattdessen `PWR_NOT_AVAILABLE` (darkgrey).
- **"Keine Verbindung"-Screen**: gezeichnet in `drawNoConnectionScreen()`
  (~Zeile 659), Touch dafuer in `loop()`s `else if (showingNoConnection)`-
  Zweig (~Zeile 2543) - bisher nur `SwipeLeft` (-> Controls) und Tap auf
  `setupButtonRect` (Menue).
- **Beruehrung im Dunkeln**: der Block direkt davor (`macSleepState==
  Active && macSleepWokenAtMs==0`, ~Zeile 2529) verschluckt die ERSTE
  Beruehrung komplett (konsumiert die Geste via `pollGesture()`, setzt
  `macSleepWokenAtMs=millis()`, `return`). Danach lief der Dispatch
  normal weiter, ABER `macSleepWokenAtMs` wurde bisher NUR beim ersten
  Wecken gesetzt - `updateMacSleepState()` schaltet nach `TOUCH_WAKE_MS`
  =15000ms ab DIESEM einen Zeitpunkt ab, unabhaengig von weiteren
  Beruehrungen. Muss fuer Schritt 3 auf "15s ab der LETZTEN Beruehrung"
  umgestellt werden.

### Schritt 2+3: Umsetzung

- Neue Konstanten/Globals nahe `setupButtonRect`: `acerNoConnButtonRect`
  = {180,200,130,28} (spiegelbildlich: 320-10-130=180, gleiche
  y/Hoehe wie der Menue-Button), eigener Feedback-Zustand
  (`noConnAcerFbColor`/`noConnAcerFbUntilMs`, bewusst KEIN
  `CtrlFeedback` wiederverwendet - der Typ ist erst spaeter im File
  deklariert) und ein eigener `noConnAcerLongFired` (separat von
  Controls' `acerLongFired` - andere Beruehrungsepisode auf einem
  anderen Screen).
- `drawNoConnAcerButton()` (Label/Farbe wie `drawCtrlAcerIdle()`, aber
  als normaler `drawButton()`-Rahmen statt Kachel) wird aus
  `drawNoConnectionScreen()` aufgerufen UND bei abgelaufenem Feedback
  neu gezeichnet (in `loop()`).
- Touch-Logik im `showingNoConnection`-Zweig: Tap (ueber `pollGesture()`,
  derselbe Aufruf wie bisher fuer Swipe/Menue) -> `sendWolPacket()`,
  Halten (eigener Raw-Touch-Block, 1:1 das Controls-Muster, selbe
  Konstanten) -> `sendShutdownCommand()`. Beide teilen sich
  `acerTapLockedUntilMs` MIT der Controls-Kachel (Auftrag: "gemeinsame
  Sperre") - ein Tap auf Controls sperrt den Button hier fuer 3s und
  umgekehrt. Feedback = derselbe Flash wie auf Controls (Button kurz
  gruen/rot GEFUELLT statt Rahmen, `WOL_FEEDBACK_MS`=1500ms,
  `drawButton(..., filled=true)`) - bewusst KEINE neue "WoL gesendet"-
  Textmeldung/StringId, da die Farb-Flash-Rueckmeldung exakt das
  bestehende Controls-Muster ist (Auftrag: "wie auf Controls, falls
  vorhanden").
- Zeitregel: ganz oben im `showingNoConnection`-Zweig wird bei JEDER
  Beruehrung (`TouchCalib::rawPoint().touched`, nicht nur bei einer
  erkannten Geste - erfasst damit auch einen reinen Halte-Druck ohne
  Bewegung) `macSleepWokenAtMs=millis()` neu gesetzt, SOLANGE bereits
  geweckt (`macSleepWokenAtMs!=0`) - die allerERSTE Beruehrung bleibt
  unberuehrt davon verschluckt (der Code dafuer liegt weiter oben und
  endet mit `return`, dieser Block hier wird in der Weck-Runde gar
  nicht erreicht).

### Schritt 4+5: Prüfung, Bauen, Flashen

- [x] LAYOUT_DEBUG/SCREEN_TOUR-Probelauf: `runScreenTour()` um
      `labelLaptop`="Müller-Äußerst Laptop" + Test-`wolMacAddress`
      erweitert (sonst wäre nur der kurze PWR_NOT_AVAILABLE-Text
      durchlaufen worden, nie der lange konfigurierte Name). Mac-App
      kurz beendet (Port vollständig frei statt nur "USB freigeben"),
      Debug-Build geflasht, per `pyserial` 20s live mitgelesen: DE+EN
      je 1x "KeineVerbindung" durchlaufen, **0 LAYOUT_DEBUG-Verstöße**
      (keine Übersch./Überlapp/Zu-klein-Meldung). Danach Debug-Flags
      zurück auf AUS, finaler Stand neu gebaut+geflasht, Mac-App wieder
      gestartet.
- [x] `pio run` + Flashen (final, Debug AUS): Flash 75,5% (989437
      Bytes), RAM 14,8% (48400 Bytes). Verbindung nach ~1s wieder da
      (identisch zu frueheren Auftraegen).
- [x] Flightradar/Solar-Guardian unveraendert (`git status` leer) -
      Toolbox nicht erneut geprüft (bereits im Vorauftrag als "hat
      eigene, nicht von mir stammende offene Änderungen" vermerkt,
      hier nicht nochmal angefasst).
- [x] Test a) von Alex bestätigt (Foto): Button "Acer Predator" unten
      rechts neben "Menü" vollständig sichtbar, Größe/Position passend.
- [x] Tests b)+c) von Alex bestätigt: Tippen -> Acer startet (Button
      kurz grün), Halten -> Acer schaltet aus (Button kurz rot) -
      "funktioniert wie immer, perfekt".
- [x] Test d) von Alex bestätigt: Controls-Acer-Kachel geht weiterhin
      an/aus wie vorher, Rueckwischen zeigt wieder "Keine Verbindung"
      (unveraendert).
- [x] Test e) von Alex bestätigt: Mac-App neu gestartet -> Dashboard
      kommt sofort zurück.
- [x] Test f) von Alex bestätigt: erstes Antippen im Dunkeln loeste
      nichts aus, zweites Antippen auf den Acer-Button schaltete den
      Acer (an), Display blieb hell und ging danach wieder von selbst
      aus - genau wie gefordert.
- [x] Test g) von Alex bestätigt (Foto, Englisch): "No connection to
      Mac", Button "Acer Predator" vollständig sichtbar, alles
      funktioniert.

**Alle Live-Tests a)-g) von Alex bestätigt (09.11.) - Auftrag inhaltlich
abgeschlossen.** Noch offen: README/docs-Satz (DE/EN), Abschluss-
Zusammenfassung.

## Auftrag Ruhezustand-Fix: Fortschritt (09.11., laufend)

Bei Unterbrechung (Test lässt den Mac schlafen - die Sitzung kann dabei
abreißen): HIER zuerst weiterlesen, nichts bereits Abgehaktes
wiederholen. Schreibt Alex "weiter", an der Stelle "WIE WEITER" unten
ansetzen.

Beobachtung von Alex: Mac-Kachel antippen lässt den Mac schlafen, CYD
bleibt aber hell und zeigt "Keine Verbindung zum Mac" - geht nie aus.
Zusätzlich: CPU-Wert im Mac-Menü blieb zweimal bei 66% stehen.

### Schritt 1: Ursache (nur gelesen, bestätigt)

- **Mac-App sendet durchgehend weiter**: `_install_sleep_watcher()`
  (`mac/sender.py`, jetzt `_on_system_sleep`) reagierte NUR auf
  `NSWorkspaceWillSleepNotification` und beendete dort nur einen
  laufenden Lüfter-Boost - der `SenderThread` selbst kannte Schlaf/
  DarkWake überhaupt nicht und sendete im DarkWake (Mac kurz mit
  Netzwerk wach, Display bleibt aus, siehe `pmset -g log`) unverändert
  weiter. Genau das hat die alte CYD-Logik (10s ohne Daten = "Mac
  schläft") immer wieder abgebrochen.
- **CYD-Firmware**: `updateMacSleepState()` (`cyd/src/main.cpp`) brach
  `MacSleepState::Requested`/`Active` bisher bei JEDEM neuen Datenpaket
  sofort ab (`newDataSinceBaseline`) - ein einzelnes DarkWake-Paket
  reichte, um "Mac schläft" zu verwerfen. Die existierende SLEEP-
  Bestätigung (`sleep_ack`-Feld, `sleepCmdPending`/`sleepCmdBaselineAck`)
  bestätigte bisher nur "Befehl angekommen", wurde aber für den
  Beleuchtungs-Zustandsautomat gar nicht genutzt.
- **"CPU bleibt stehen"**: `MacmonReader._run()` liest den `macmon pipe`-
  Subprozess per blockierender Zeilen-Iteration, ohne jede Überwachung -
  hängt macmon (z.B. nach Schlaf/Aufwachen) fest, liefert `for line in
  proc.stdout` nie mehr EOF, der Prozess wird nie neu gestartet.
  `update_ui()` selbst blockiert nicht (liest nur einen Lock-geschützten
  Cache), die Ursache lag ausschließlich bei macmon selbst.

### Schritt 2: Mac-App - Sendepause (neu: `mac/wake_watch.py`)

`WakeWatcher` abonniert ZWEI unabhängige AppKit-Paare:
`NSWorkspaceWillSleep/DidWake` (echter Systemschlaf - `DidWake` feuert
NICHT bei reinem DarkWake) UND `NSWorkspaceScreensDidSleep/DidWake`
(Bildschirm aus/an - bleibt während eines DarkWake durchgehend "aus",
deckt genau die DarkWake-Lücke ab). `is_awake()` ist nur dann True,
wenn beide "wach" sagen. Sonderfall Mac ohne Bildschirm: `has_display`
wird einmalig über `NSScreen.screens()` geprüft - ohne Bildschirm (oder
ganz ohne PyObjC) zählt nur noch das Systemschlaf-Paar, keine
Sendepause durch die Display-Erkennung. `SenderThread.run()` prüft
`is_awake_fn()` vor jedem Tick; verneint, wird 0.3s gewartet und
`next_tick` auf "jetzt" zurückgesetzt, ohne zu messen/senden - nach dem
Aufwachen geht es dadurch binnen <1s weiter (gefordert: <2s).
`on_wake`-Callback (`WakeWatcher(on_wake=macmon.restart)`) startet
macmon sofort neu.
Bestätigung VOR dem Einschlafen sichergestellt: `_on_cyd_sleep()` zählt
jetzt zuerst `bump_sleep_ack()` hoch, sendet dann SOFORT per neuem
`SenderThread.send_now()` (außerhalb des normalen Takts, umgeht Pause/
Takt), und löst erst DANACH `pmset sleepnow` aus - vorher war die
Reihenfolge umgekehrt (Sleep zuerst), wodurch die Bestätigung den
nächsten regulären Tick nie unbedingt noch erreicht hätte.

### Schritt 3: Mac-App - hängende Werte selbst reparieren

`MacmonReader` hat jetzt einen Watchdog-Thread: kein neuer Wert seit
`MACMON_STALE_SEC`=5s -> `proc.kill()` (erzeugt EOF, die blockierende
Zeilen-Schleife in `_run()` kommt dadurch wieder frei und startet neu),
höchstens alle `MACMON_RESTART_MIN_GAP_SEC`=10s automatisch. Die neue
`restart()`-Methode (vom `WakeWatcher` beim Aufwachen aufgerufen)
umgeht diese Sperre bewusst ("nach JEDEM Aufwachen neu starten").
Jedes Ereignis (Neustart-Grund, Schlaf-/Aufwach-Übergänge) geht über
die neue `log_event()` in `~/Library/Application Support/
Eiswolfs iBoard/logs/events.log` (1 MB, `RotatingFileHandler` mit einer
rotierten Altdatei) UND weiterhin auf die Konsole - nie auf den
Bildschirm. `update_ui()` selbst blockiert nicht (siehe Schritt 1), kein
Fix dort nötig.

### Schritt 4: CYD-Firmware - Zustandsautomat auf Bestätigung umgestellt

`MacSleepState` um `Confirmed` erweitert: `Requested` (wartet auf
`sleep_ack`-Änderung, Abbruch nach `SLEEP_ACK_TIMEOUT_MS`=3000 ohne
Bestätigung) -> `Confirmed` (Bestätigung da, `SLEEP_DISPLAY_OFF_DELAY_MS`
=3000 Verzögerung) -> `Active` (Beleuchtung aus). In `Active` zählt
NICHT mehr jedes einzelne Paket als "Mac wach" - `WAKE_SUSTAIN_MS`=5000
verlangt einen ununterbrochenen Daten-Streak (Lücken über
`WAKE_GAP_RESET_MS`=2000 brechen den Streak ab, nicht benannt im
Auftrag, aber technisch nötig). Die generelle "Keine Verbindung"-
Umschaltung in `loop()` ist während `Active` bewusst gesperrt (sonst
hätte sie ein einzelnes DarkWake-Paket sofort gezeichnet, bevor die
Sustain-Prüfung überhaupt anlaufen konnte) - `updateMacSleepState()`
zeichnet beim bestätigten Aufwachen selbst (`drawStaticLayout()` vor
`applyBrightness()`, wie gefordert). `TOUCH_WAKE_MS`=15000 (Berührung im
Dunkeln) unverändert aus dem Vorauftrag übernommen. Log-Zeilen exakt
wie gefordert: "Bestätigung da" / "keine Bestätigung, abgebrochen" /
"Daten kurz, ignoriert" / "Daten anhaltend, Display an".
Gebaut (`pio run`): Flash 75,4%, RAM 14,8% (unverändert). NOCH NICHT
geflasht (folgt zusammen mit der Mac-App in Schritt 5).

### WIE WEITER (vor dem ersten echten Schlaf-Test geschrieben)

- [x] Schritt 1-4 Code fertig, Firmware-Build fehlerfrei
- [x] Mac-App gebaut (`pyinstaller`, Version 1.1.3), ad-hoc signiert
      ("valid on disk"), kurzer isolierter Testlauf fehlerfrei (lief
      >15s stabil, fand das CYD per USB, keine Fehler/Traceback), DMG
      neu gebaut (`dist/Eiswolfs-iBoard.dmg`, `mac/dmg_build/`
      aktualisiert), in `/Applications/Eiswolfs iBoard.app` installiert
      und neu gestartet (Autostart-LaunchAgent zeigte schon auf den
      richtigen Pfad, nichts daran geaendert) - `config.json` unbe-
      ruehrt.
- [x] CYD geflasht (`/dev/cu.wchusbserial31410`), Verbindung nach ~1s
      wieder da (identisch zu frueheren Auftraegen)
- [x] Flightradar/Solar-Guardian-CYD per `git status` als unveraendert
      (leer) bestaetigt. Toolbox (`eiswolfs-toolbox`) hat bereits
      VORHER bestehende, nicht von diesem Auftrag stammende offene
      Aenderungen (HackerNews/Mondphasen-Screens) - nicht angefasst,
      nur zur Kenntnis genommen.
- [x] Test a) von Alex live bestätigt (09.11.): Mac-Kachel antippen ->
      Mac schläft, CYD wird dunkel, NICHT "Keine Verbindung" gezeigt.
- [x] Test b) von Alex bestätigt (CYD blieb die ganze Zeit dunkel) UND
      per `pmset -g log` + `logs/events.log` selbst nachvollzogen:
      Schlaf 22:29:58 -> Wach 22:33:04 (~3:06 min), durchgehend
      "Systemschlaf"+"Bildschirm schlaeft", KEINE Zwischen-Aufwach-
      Meldung. In diesem Lauf trat zufaellig KEIN DarkWake auf (laut
      `pmset -g log` kein einziger DarkWake-Eintrag in diesem Fenster) -
      die Sendepause fuer einen durchgehenden echten Sleep ist damit
      bestaetigt, die DarkWake-spezifische Lücke wurde diesmal nicht
      exerziert (kein Fehlschlag, nur nicht aufgetreten). Beim
      tatsaechlichen Aufwachen (durch Bluetooth-HID-Aktivitaet, laut
      `pmset -g log`) hat `macmon neu: nach Aufwachen` sauber gegriffen.
      `logs/send_log.csv` ist eine STALE Datei von einem fruehen
      Diagnose-Auftrag (letzter Eintrag 03.11., `debug_log` aktuell
      `false`) - NICHT als Beleg verwendet, um keine falsche Sicherheit
      vorzutaeuschen.
- [x] Test c) von Alex bestätigt (09.11.): Berührung im Dunkeln -> 15s
      hell, zeigt "Keine Verbindung zum Mac" + Menü-Button, sonst nichts
      ausgelöst, danach wieder dunkel - genau wie beschrieben.
- [x] Test d) von Alex bestätigt (09.11., mehrfach gesehen): CYD wird
      von selbst hell, Dashboard ca. 2-3s nach dem Mac-Aufwachen da,
      Werte in der Mac-App bewegen sich wieder normal.
- [x] Test e) von Alex bestätigt: Mac-App beendet (ohne Kachel-Nutzung)
      -> CYD bleibt hell, zeigt "Keine Verbindung zum Mac" (kein
      Dunkel-Zustand, korrektes normales Verhalten).
- [x] Test f) von Alex bestätigt: Mac-App neu gestartet -> Dashboard
      kam sofort zurück.

**Alle Live-Tests a)-f) von Alex bestätigt (09.11.) - Auftrag inhaltlich
abgeschlossen.** Noch offen: README/docs-Satz (DE/EN), Abschluss-
Zusammenfassung.
- [ ] README/docs (DE/EN) um einen Satz zu Sendepause/Display-aus
      ergänzen
- [ ] Abschluss-Zusammenfassung an Alex (Ursache, Version, Tests mit
      Zeiten, Endzustand)

**Nicht committen/taggen/pushen (Auftrag).**

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
cd ~/Eiswolfs-iBoard/mac
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
`~/Library/LaunchAgents/com.eiswolf.iboard.app.plist` -
bewusst ein ANDERES Label als der Entwickler-LaunchAgent
(`com.eiswolf.iboard`, siehe `mac/install.sh`/
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

Projektordner heißt jetzt `~/Eiswolfs-iBoard` (vorher
`~/iBoard`) - Bindestrich statt Leerzeichen, damit
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
Label/Plist-Datei jetzt `com.eiswolf.iboard`
(`mac/com.eiswolf.iboard.plist`, vorher `...iboard.plist`).
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

Die Projekte **Flightradar** (`~/CYD Flightradar`),
**Toolbox/Satellitenradar** (`~/eiswolfs-toolbox`) und
**Solar Guardian** (`~/solar-guardian-cyd`) dürfen auf
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

## Hauptmenü (ÖFFNET HEUTE NUR über die Kachel "Menü" auf Seite 2/Controls bzw. den Knopf auf "Keine Verbindung"; das Antippen von "iBoard" in der Kopfzeile tut nichts mehr, siehe `runMainMenu()`)

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

- **Ort**: `~/Eiswolfs-iBoard/mac/sender.py` (läuft
  aus der venv in `mac/venv/`).
- **Von Hand starten**: Doppelklick auf
  `~/Eiswolfs-iBoard/Eiswolfs iBoard starten.command`
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
  (`com.eiswolf.iboard.plist`) hat bewusst **kein
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
   des aufrufenden Benutzers, Modus 0600 beschränkt - NIE auf dem Netz
   erreichbar).
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
     `/Library/LaunchDaemons/com.eiswolf.iboard.fanhelper.plist`).
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
  (`~/Library/LaunchAgents/com.eiswolf.iboard.app.plist`)
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
  (`~/.platformio/...`, durch Compiler-Debug-Infos
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
- [x] **Tray-Fassung des Windows-Listeners end-zu-Ende bestaetigt**:
      Alex hat zusaetzlich einen echten Neustart des Windows-Laptops
      getestet - der Listener startet zuverlaessig automatisch mit
      (Startmenue-Autostart-Verknuepfung funktioniert ueber einen
      Reboot hinweg, nicht nur beim einmaligen Installieren/manuellen
      Start). "funktioniert super!" (Mondsichel-Nebenthema separat
      geklaert) UND "Funktioniert! Listener wird mitgestartet" (nach
      echtem Neustart) - beides von Alex live bestaetigt.
- [x] **Committed und gepusht**: alles aus diesem Abschnitt (CYD-
      Aenderungen, `windows/`, `docs/TOKEN.md`, `tools/fan-helper/
      README.md`, README-Aenderungen, `.gitignore`) ist seit Commit
      `ce0a219` auf `main` sowie dem Release **v1.1** (Tag + GitHub
      Release mit DMG/Firmware/FLASHEN.md/Windows-Setup als Anhaenge,
      Web-Flasher-Seite auf v1.1 aktualisiert) oeffentlich auf GitHub.
      Auftrag vollstaendig abgeschlossen.

## Auftrag Sicherheit und Lizenzen: Fortschritt

Sicherheits-/Rechte-Durchgang vor weiterer Verbreitung, strikt in
Teil-Reihenfolge. NUR im Arbeitsordner aendern - NICHT committen,
taggen, pushen, kein Release, keine Aenderung am GitHub-Repo/an
GitHub Pages, das macht Alex selbst. Schutzregel wie immer:
Flightradar/Toolbox/Solarguardian unberuehrt, nur Eiswolfs-iBoard.
Apfel-Logo bleibt unveraendert (Alex' Entscheidung). Bei "weiter"
hier zuerst lesen, an der ersten unerledigten Stelle fortsetzen.

### Teil 1: Geheimnisse in Historie und Release-Dateien

- [x] 1a/1b abgeschlossen (siehe Funde unten). Umfang: alle 5 Commits
      (ein Branch `main`, Tags `v1.0`/`v1.1`, `git fsck --unreachable`
      leer - keine verworfenen/geloeschten Commits mit Altlasten).
      Token selbst (48 Zeichen, aus der lokalen, nicht versionierten
      `~/Library/Application Support/Eiswolfs iBoard/config.json`
      gelesen, nie ausgegeben) per `git log --all -S"<token>"` UND
      `git grep` ueber ALLE Commit-Blobs geprueft: **0 Treffer in der
      gesamten Historie**. WLAN-SSID/-Passwort: **kein Fund** - die
      CYD-WLAN-Zugangsdaten liegen ausschliesslich auf der SD-Karte des
      Geraets selbst, nie in diesem Repo (es gibt in `mac/config.json`
      ohnehin kein WLAN-Feld). Reale lokale IP-Adressen (z.B.
      `192.168.178.x`), E-Mail-Adressen (ausser Schriftlizenz-Copyright,
      unproblematisch), Rechnernamen aus dem lokalen Netz: **kein
      Fund**. Allgemeine Geheimnis-Muster (`password=`, `secret=`,
      `token=` mit echtem Wert): **kein Fund**.

  **Fund 1 (gering, aber real) - Benutzerpfad in `CLAUDE.md`**: Die
  Datei nennt an mehreren Stellen den vollen lokalen Pfad
  `/Users/al*********/Eiswolfs-iBoard` (und Nachbarprojekte) im
  Klartext, seit dem allerersten Commit `d5eae09` (also in JEDEM der
  5 Commits/beiden Tags enthalten). Verraet Alex' macOS-
  Benutzernamen, kein Sicherheitsrisiko fuer sich allein (kein Zugriff
  ohne physischen/Fernzugriff auf den Mac moeglich), aber oeffentlich
  einsehbare private Information.

  **Fund 2 (gering) - Bundle-Identifier mit Benutzernamen**: beide
  LaunchAgent/LaunchDaemon-Plist-Dateien
  (`mac/com.al*********.eiswolfsiboard.plist`,
  `tools/fan-helper/com.al*********.eiswolfsiboard.fanhelper.plist`)
  sowie Verweise darauf in `mac/eiswolfs_iboard.spec`,
  `mac/login_item.py`, `mac/install.sh`/`uninstall.sh`,
  `tools/fan-helper/install.sh`/`uninstall.sh` nutzen
  `com.al*********.eiswolfsiboard...` als feste Bundle-ID. Baut sich
  dadurch auch in jede gebaute `.app`/jeden installierten
  LaunchDaemon ein (im Info.plist der DMG bestaetigt). Kein
  Sicherheitsrisiko, aber unschoen fuer Fremdnutzer (deren
  LaunchAgent heisst dann trotzdem nach Alex) und verraet den Namen.

  **Fund 3 (gering, technischer Natur) - Build-Pfade in den
  veroeffentlichten Binaerdateien**: Sowohl `firmware.bin` (Repo-
  Wurzel, vom Web-Flasher ausgeliefert) als auch
  `dist/Eiswolfs-iBoard-CYD-merged.bin` (Release-Anhang) enthalten im
  Klartext u.a.
  `/Users/al*********/.platformio/packages/framework-arduinoespressif32/
  libraries/SPI/src/SPI.cpp` (und mehrere weitere, siehe `WiFiGeneric.cpp`/
  `WiFiSTA.cpp`/`WiFiUdp.cpp`/`Preferences.cpp`) - eingebettete
  `__FILE__`-Quellpfade des ESP32-Arduino-Core/PlatformIO-
  Build-Systems, NICHT aus eigenem Code. `bootloader.bin`/
  `partitions.bin` sind sauber. Die Windows-`setup.exe`
  (`windows/installer/EiswolfsIBoardListener-Setup.exe`, aktuell als
  Release-Anhang veroeffentlicht) enthaelt ebenso im Klartext mehrere
  Go-Modul-Pfade wie
  `/Users/al*********/go/pkg/mod/github.com/getlantern/context@.../context.go`
  (Standard-Verhalten eines `go build` OHNE `-trimpath`). Das Mac-
  App-Executable (in der DMG) wurde ebenfalls per `strings` geprueft -
  **kein Treffer**, PyInstaller baut offenbar ohne eingebettete
  absolute Quellpfade.

  **Fund 4 (keine Sicherheitsluecke, aber Haertungsempfehlung) -
  `.gitignore`-Deckung**: `cyd/src/secrets.h` und `mac/config.json`
  sind explizit erfasst; fuer `windows/`/`tools/` gibt es aber noch
  KEIN generisches Muster fuer `config.json`/`*.token`/`.env`, falls
  dort je eine lokale Testdatei entsteht (aktuell liegt keine einzige
  solche Datei im Arbeitsbaum, reiner Vorsorge-Punkt, kein aktueller
  Fund).

  **Plan (noch NICHT ausgefuehrt, wartet auf Alex' Entscheidung):**
  1. `CLAUDE.md` nachtraeglich pfad-neutral machen (z.B.
     `~/Eiswolfs-iBoard` statt des vollen `/Users/...`-Pfades) -
     einfache Textersetzung, kein Historie-Problem geloest, nur der
     KUeNFTIGE Stand.
  2. Bundle-IDs auf einen projektbezogenen, nutzerunabhaengigen
     Namensraum umstellen (z.B. `com.eiswolf-bg.eiswolfsiboard...`
     statt `com.al*********...`) - betrifft 7 Dateien, Mac-App muss
     danach neu gebaut und neu installiert werden (alter LaunchAgent-
     Eintrag muss vorher entfernt werden, sonst zwei parallele
     Eintraege).
  3. CYD-Firmware: `-fmacro-prefix-map=$PROJECT_PACKAGES_DIR=.`
     (bzw. aehnliches PlatformIO-`build_flags`-Flag) ergaenzen, damit
     `__FILE__` nicht mehr den vollen Pfad eintraegt - danach neu
     bauen, Web-Flasher-Dateien UND Release-Anhang ersetzen.
  4. Windows-Listener: `go build ... -trimpath ...` statt des
     bisherigen Befehls - danach `setup.exe` neu bauen, Release-Anhang
     ersetzen.
  5. `.gitignore` um `windows/**/config.json`, `**/*.token`, `**/.env`
     ergaenzen (reine Haertung).
  6. **Ob/wie die Git-HISTORIE selbst bereinigt werden soll** (die
     alten Commits/Tags `v1.0`/`v1.1` enthalten den Pfad weiterhin,
     auch nach einem neuen Commit) - das beruehrt Force-Push/
     Tag-Neuerstellung auf einem bereits oeffentlichen Repo (andere
     koennten schon geklont haben) und wird hier bewusst NICHT
     vorentschieden, reine Alex-Entscheidung.

  Punkt 1 ist der einzige mit None-Risiko (reiner Text in einer
  einzelnen Datei, kein Rebuild noetig) - alle anderen Punkte
  brauchen einen Neu-Build einer oder mehrerer Komponenten.

  **STOPP aufgehoben - Alex hat entschieden (siehe Auftrag
  "fortsetzen"): KEIN Force-Push/Historie-Umschreiben, stattdessen
  bereinigter Stand lokal als v1.1.1 vorbereiten.**

### Teil 1b: Bereinigung

- [x] **1b-1 Neutrale Kennung fertig**: `com.<Benutzername>.eiswolfsiboard`
      -> `com.eiswolf.iboard` (App/Dev-LaunchAgent),
      `com.<Benutzername>.eiswolfsiboard.app` -> `com.eiswolf.iboard.app`
      (installierter LaunchAgent, `mac/login_item.py`),
      `com.<Benutzername>.eiswolfsiboard.fanhelper` ->
      `com.eiswolf.iboard.fanhelper` (LaunchDaemon). Betroffen: beide
      Plist-Dateien (umbenannt, alte Datei lokal geloescht - NICHT per
      `git rm --cached`, nur im Arbeitsbaum, siehe `git status`),
      `mac/eiswolfs_iboard.spec` (`bundle_identifier`),
      `mac/install.sh`/`uninstall.sh`, `mac/login_item.py`,
      `tools/fan-helper/install.sh`/`uninstall.sh`/`README.md`. Danach
      Volltextsuche ueber das ganze Projekt (ohne `.git`/`.pio`/`venv`/
      `__pycache__`): **0 Treffer fuer den Benutzernamen** ausserhalb
      von `CLAUDE.md` (dort separat in 1b-2 behandelt). Socket-Pfad
      (`/var/run/eiswolfs-iboard-fan.sock`) enthielt nie den
      Benutzernamen, unveraendert gelassen. Mac-App-Version dabei
      gleich auf **1.1.1** gesetzt (`mac/eiswolfs_iboard.spec`, noch
      NICHT in `mac/sender.py` - folgt zusammen mit Teil 2 in einem
      Rutsch, um nicht zweimal neu zu bauen).
- [x] **1b-2 CLAUDE.md fertig**: alle `~/...`-Pfade (vorher voller
      `/Users/al*********/`-Pfad) und alle alten Bundle-IDs per
      gezieltem `sed` ersetzt - **0 Treffer fuer den Benutzernamen**
      in der ganzen Datei. Entscheidung/Vorschlag an Alex: `CLAUDE.md`
      bleibt NUR lokal (neue `.gitignore`-Regel ergaenzt, Datei selbst
      NICHT per `git rm --cached` entfernt - das macht Alex selbst,
      der Befehl steht unten im Abschluss). Neue, schlanke
      **`docs/DEVELOPMENT.md`** (zweisprachig DE/EN) angelegt als
      oeffentliche Bau-/Mitwirkungs-Referenz (Ordnerstruktur,
      Bau-Befehle fuer alle drei Komponenten inkl. `-trimpath`-Hinweis,
      Verweis auf `docs/TOKEN.md`, kurzer Mitwirken-Absatz) - KEINE
      privaten Pfade/Geheimnisse/internen Notizen. `README.md`s
      "Building from source"-Abschnitt verlinkt jetzt dorthin statt auf
      `CLAUDE.md`.
- [x] **1b-3 fertig** (Mac-App-DMG-Recheck bewusst auf NACH Teil 2e
      verschoben, siehe unten):
      - **Windows**: `go build -trimpath -buildvcs=false -ldflags
        "-H=windowsgui -s -w"` statt des bisherigen Befehls (ohne
        `-trimpath`). `strings` auf `EiswolfsIBoardListener.exe` UND
        die neu gebaute `EiswolfsIBoardListener-Setup.exe`: **0
        Treffer** fuer Benutzername/`/Users/`-Pfad (vorher mehrere
        `/Users/al*********/go/pkg/mod/...`-Pfade).
      - **Firmware**: neues `cyd/trim_paths.py` (PlatformIO-Pre-Skript,
        `extra_scripts = pre:trim_paths.py` in `platformio.ini`) - setzt
        `-ffile-prefix-map` fuer PlatformIO-Core-Verzeichnis,
        Projektverzeichnis UND Heimverzeichnis, alle drei zur BAUZEIT
        per `os.path.expanduser()`/PlatformIO-Umgebungsvariablen
        ermittelt - der echte Pfad steht dadurch NIRGENDS als Text im
        Repo, jeder baut automatisch mit seinem EIGENEN Pfad bereinigt.
        Sauberer Neu-Build (`.pio/build` geloescht) mit Wegwerf-Token:
        `strings` auf `firmware.bin`/`bootloader.bin`/`partitions.bin`/
        der gemergten Release-Datei: **0 Treffer fuer den
        Benutzernamen** (vorher mehrere `WiFiGeneric.cpp`/`SPI.cpp`-
        Pfade). **Rest gefunden, NICHT behebbar von uns aus**: ein
        anderer, FREMDER Pfad
        (`/Users/br*******/.gitlab-runner/builds/.../newlib/...`)
        steckt weiterhin in `firmware.bin` - das ist NICHT Alex'
        Konto, sondern der Build-Rechner von Espressifs eigener
        vorkompilierter newlib/libc-Bibliothek (Teil des ESP32-
        Arduino-Core-Toolchains, wir linken nur dagegen, kompilieren
        sie nicht selbst) - betrifft JEDES Projekt mit diesem
        Framework, kein Alex-spezifisches Datenschutzproblem, aber
        technisch nicht mit `-ffile-prefix-map` entfernbar (die Datei
        ist schon vorkompiliert). Danach echten Token wiederhergestellt,
        neu gebaut, Testgeraet neu geflasht (`/dev/cu.wchusbserial31410`,
        Upload erfolgreich).
      - **Metadaten/Autor-Felder**: Mac-`Info.plist` hat keine
        Autor-/Copyright-Eintraege mit Personenbezug (nie gesetzt).
        Windows-`.exe` hat gar keine eingebettete Versions-Ressource
        (keine CompanyName/Autor-Felder vorhanden). Git-Commit-Autor
        auf beiden Tags (`v1.0`/`v1.1`) bereits das pseudonyme
        `Eiswolf-BG <...@users.noreply.github.com>`, kein Klarname/
        keine private E-Mail.
- [x] **1b-4 fertig**: `local/migrate-to-neutral-id.sh` (kein sudo -
      beendet die laufende App, entfernt alte Dev-/App-LaunchAgents
      nach Rueckfrage, laesst `~/Library/Application Support/Eiswolfs
      iBoard/` unangetastet) + `local/migrate-fan-helper-sudo.sh`
      (sudo, separates Skript - entfernt den alten LaunchDaemon nach
      Rueckfrage, installiert danach `tools/fan-helper/install.sh`
      neu). Beide leiten den alten Benutzernamen zur Laufzeit her
      (`$(whoami)`/`$SUDO_USER`), NICHT fest im Text. Per `DRY_RUN=1`
      getestet (Karl, ohne sudo/ohne echte Aenderung) - Logik
      bestaetigt korrekt, u.a. den tatsaechlich vorhandenen alten
      App-LaunchAgent auf Alex' Mac gefunden. Echter Lauf (inkl. des
      sudo-Skripts) ist Alex' Aufgabe, siehe Abschluss unten.
- [x] **1b-5 .gitignore-Haertung fertig**: `**/secrets*`, `**/*.token`,
      `**/.env`, `**/config.json` (projektweit, alle Unterordner),
      `local/` und `CLAUDE.md` ergaenzt. Gegengeprueft per
      `git ls-files | grep -i config` - keine `config.example.json`
      o.ae. im Repo vorhanden, die dadurch versehentlich verdeckt
      wuerde. `git status` bestaetigt: bereits getrackte Dateien
      (allen voran `CLAUDE.md` selbst) bleiben trotz neuer
      `.gitignore`-Regel ganz normal als "modified" sichtbar (gitignore
      wirkt nur auf neue, bisher ungetrackte Dateien) - verhaelt sich
      wie erwartet.
- [x] **1b-6 fertig**: `local/BEREINIGUNG.md` angelegt (was neu gebaut
      wurde, was in der alten Historie sichtbar bleibt, Vorschlaege
      fuer Alex - alte Release-Anhaenge auf GitHub ersetzen, Web-
      Flasher-Dateien per normalem Commit aktualisieren, optionale
      spaetere Historie-Bereinigung nur als Hinweis ohne Empfehlung).
      Keine Geheimnisse darin.

**Teil 1b komplett abgeschlossen.**

### Teil 2: Schutz der Befehle gegen Wiederholung
- [x] **2a fertig**: Token stand BISHER im Klartext am Zeilenende jedes
      Befehls (`FAN BOOST/AUTO`, `SLEEP`, `SHUTDOWN` - per UDP-Unicast/
      Broadcast ODER USB-Zeile). Jeder, der das WLAN mitschneidet
      (oder eine USB-Debug-Ausgabe sieht), kann denselben Befehl
      beliebig oft wiederholen (Replay) - z.B. den Mac wiederholt
      schlafen schicken oder den Laptop wiederholt herunterfahren.
      Bindung: Mac (`fan_boost.py`) `sock.bind(("", 4211))` = alle
      Schnittstellen; Windows (`main.go`) `net.IPv4zero` auf Port 4212
      = ebenfalls alle Schnittstellen. Fuer macOS gibt es keine
      einfache "nur lokales Netz"-Bindung (mehrere Interfaces moeglich,
      kein Subnetz-Bind ohne feste IP) - die eigentliche Absicherung
      kommt hier bewusst von der neuen HMAC+Zaehler-Pruefung selbst,
      nicht von der Netzwerk-Bindung. Fuer Windows IST eine
      Firewall-Regel sinnvoll moeglich (Teil 2c).
- [x] **2b CYD-Seite fertig**, Entwurf + Umsetzung (Empfaenger folgen):
      Neues Format: `<BEFEHL optional-Argumente> <Zaehler>
      <16-Hex-Zeichen-HMAC>` statt `<BEFEHL optional-Argumente>
      <Token>`. HMAC = erste 8 Byte von HMAC-SHA256(Schluessel=Token,
      Nachricht=alles VOR dem HMAC inkl. Zaehler) - bewusst gekuerzt
      (volle 32 Byte waeren fuer eine Textzeile unhandlich), 64 Bit
      reichen fuer die Bedrohung "jemand im selben LAN" voellig.
      Neue Dateien `cyd/src/cmd_auth.h/.cpp` (mbedtls HMAC-SHA256, im
      ESP32-Arduino-Core bereits vorhanden, keine neue Abhaengigkeit).
      Neues additives Feld `Storage::DeviceConfig::cmdCounter`
      (`uint32_t`, NVS/SD) - steigt bei JEDEM gesendeten Befehl, wird
      SOFORT gespeichert (nicht gebuendelt), ueberlebt dadurch normale
      Neustarts. `sendFanCommand()`/`sendSleepCommand()`/
      `sendShutdownCommand()` nutzen jetzt `CmdAuth::signCommand()`
      statt des rohen Tokens. Neuer `sendResyncCommand()` (eigener
      "RESYNC"-Befehl, ganz normal signiert, an BEIDE Empfaenger
      gleichzeitig) + neuer Menuepunkt "Zaehler neu abgleichen" im
      Setup-Untermenue (mit "?"-Erklaerung, 5. Zeile - Zeilenhoehen
      dafuer weiter gestaucht, 28px statt 32px, passt noch auf den
      Bildschirm). Resync-SIcherheitslogik bewusst NUR empfaengerseitig
      (siehe Entwurfs-Begruendung im Code-Kommentar `cmd_auth.h`) - das
      Geraet kann nach einem Totalverlust seines eigenen Zaehlers
      unmoeglich selbst beweisen, dass sein neuer Vorschlag wirklich
      hoeher ist als vorher. Baut fehlerfrei (`pio run`,
      RAM 14.7%/Flash 71.7%) - **noch NICHT geflasht**, da die
      Empfaenger (Mac/Windows) das neue Format noch nicht verstehen -
      folgt in 2d nach deren Fertigstellung.
      **Abwaertskompatibilitaet bewusst NICHT auf CYD-Seite eingebaut**
      (sendet ab jetzt IMMER nur noch das neue Format) - die in 1b
      beschlossene gemeinsame Migration macht das unnoetig
      (CYD+Mac+Windows werden zusammen aktualisiert). Der optionale
      "alte Befehle noch annehmen"-Schalter (Standard aus) sitzt
      stattdessen EMPFAENGERSEITIG (Mac/Windows), siehe unten.
- [x] **2b Mac+Windows fertig**: `mac/fan_boost.py` (`_verify_signed()`,
      `CydCommandListener` komplett umgebaut - `_dispatch()` fuer
      erfolgreich geprueftes neues Format, `_handle_legacy()` fuer
      den optionalen alten Pfad, `allow_resync()`), `mac/config.py`
      (`last_cmd_counter`, `allow_legacy_commands`, beide additiv/
      Default aus bzw. 0). Neues Mac-Menue "Sicherheit" (`sender.py`):
      "Zaehler-Resync zulassen (60s)…" + "Alte, unsichere Befehle
      zulassen" (Checkbox, Warntext beim Einschalten). `windows/
      listener/main.go` komplett parallel umgebaut (`verifySigned()`,
      `handleLine()`, neue Config-Felder, zwei neue Tray-Menuepunkte).
      **Go/Python-HMAC-Kompatibilitaet bestaetigt**: identischer
      Testvektor (Token+Nachricht) ergibt in beiden Sprachen exakt
      denselben Hex-Wert - die drei Implementierungen (C++/mbedtls,
      Python/hmac, Go/crypto-hmac) sind nachweislich kompatibel.
- [x] **2c fertig**: Mac-Bindung bleibt `("", 4211)` (alle
      Schnittstellen) - fuer macOS gibt es keine praktikable "nur
      LAN"-Bindung ohne feste IP, die eigentliche Absicherung kommt
      von HMAC+Zaehler selbst (siehe 2a). Windows: KEINE Firewall-Regel
      per Installer (bräuchte Admin-Rechte, widerspricht dem Projekt-
      Grundsatz "kein erhöhter Install noetig") - stattdessen MessageBox
      im Installer UND `windows/README.md` raten jetzt ausdruecklich,
      beim Windows-eigenen Firewall-Prompt NUR "Private Netzwerke"
      zuzulassen. **Testliste fuer Alex** (Windows-Teil, da Karl das
      nicht selbst testen kann) steht unten im Abschluss.
- [x] **2d fertig (Mac-Teil)**: installierte App beendet, Entwicklungs-
      kopie mit dem neuen Code gestartet (sauber, keine Fehler im Log).
      Eigenes Testskript (`local/`-Scratch, nicht committed) simuliert
      signierte CYD-Befehle direkt per UDP an Port 4211, Zustand jeweils
      per `PING` am Luefter-Hilfsdienst-Socket live geprueft:
      - Gueltiger `FAN BOOST`-Befehl (Zaehler 1) -> Boost aktiviert
        (`mode: MANUAL`, `target_rpm: 2000`) - **bestanden**
      - Falscher HMAC (ein Hex-Zeichen veraendert, Zaehler 2) -> IGNORIERT,
        Zustand unveraendert - **bestanden**
      - EXAKTE Wiederholung des gueltigen Zaehler-1-Pakets -> IGNORIERT
        (Zaehler nicht > letzter akzeptierter) - **bestanden**
      - Gueltiger `FAN AUTO`-Befehl (Zaehler 3) -> zurueck auf Automatik
        - **bestanden**
      - Altes, unsigniertes Format (`FAN BOOST ... <Token>`) bei
        Legacy-Schalter AUS -> IGNORIERT - **bestanden**
      Luefter-Boost/Ruhemodus-Logik selbst unveraendert (nur die
      Signatur-/Zaehlerpruefung davor ist neu) - keine Verhaltens-
      aenderung fuer gueltige Befehle zu erwarten, durch obige Tests
      auch bestaetigt. Installierte App (noch ALTES Protokoll) danach
      wieder gestartet.
      **CYD noch NICHT geflasht** (macht sendResyncCommand() etc.
      sofort inkompatibel mit der noch installierten App) - folgt
      direkt im Anschluss an diesen Fortschritts-Eintrag, danach
      Installierte App via 2e aktualisieren, damit beide Seiten wieder
      zusammenpassen.
      **Nicht live getestet** (Zeit-/Umfangsgruende, aber per Code-
      Review/Logik-Pruefung abgedeckt): der Resync-Mechanismus selbst
      (braucht echte Geraete-Interaktion an BEIDEN Enden gleichzeitig)
      und SHUTDOWN/Windows-Listener direkt (braucht den Windows-Laptop,
      Alex' eigene Testliste unten deckt das ab).
- [x] **2e fertig**: CYD mit dem neuen Protokoll geflasht (echter
      Token, `/dev/cu.wchusbserial31410`, Upload erfolgreich). App neu
      gebaut (`APP_VERSION` bereits bei 1.1.1 aus 1b-1), signiert
      (gueltig), Bundle-ID bestaetigt `com.eiswolf.iboard.app`, **0
      Benutzername-Treffer** im Executable. DMG neu gebaut, Token- UND
      Username-Check: **0/0 Treffer**. Installierte App ersetzt und neu
      gestartet. Mac-seitigen `last_cmd_counter` auf 0 zurueckgesetzt
      (war durch Karls eigene Testbefehle auf 3 gestiegen, haette sonst
      Alex' erste paar echte Antipp-Versuche stillschweigend
      ignoriert, bis das CYD aufgeholt haette). **CYD und installierte
      App sprechen jetzt wieder dasselbe (neue) Protokoll.**
      **Noch nicht live am echten Geraet getestet** (Karl kann die
      Touchscreen-Taps nicht selbst ausloesen) - Alex-Testliste unten
      im Abschluss deckt das ab.

**Teil 2 komplett abgeschlossen.**

### Teil 3: Lizenzhinweise
- [x] **3a+3b fertig**: neue `licenses/`-Ordner (Repo-Wurzel) mit allen
      vollstaendigen Lizenztexten (macmon/MIT, rumps/BSD, psutil/
      BSD-3-Clause, pyobjc/MIT, PyInstaller-Hinweis zur Bootloader-
      Ausnahme, TFT_eSPI/FreeBSD, XPT2046_Touchscreen/MIT, ArduinoJson/
      MIT, ESP32-Arduino-Core-Hinweis/LGPL-2.1, MacFanControl/MIT,
      APACHE-2.0 (deckt alle getlantern-Pakete bis auf `hex` sowie
      `oxtoacart/bpool` ab), `go-stack/stack`/MIT,
      `getlantern/hex`/BSD-artig, `golang.org/x/sys`/BSD-3-Clause,
      pyserial-Hinweis - exakter Text lokal nicht gefunden, nur
      Verweis auf die Originalquelle). Neue `THIRD-PARTY-LICENSES.md`
      in der Repo-Wurzel (Tabellen je Projektteil: CYD/Mac/Windows/
      Web-Installer) ersetzt/erweitert die bisherige, engere
      `mac/dmg_staging/THIRD-PARTY-LICENSES.md` (wird jetzt 1:1 aus der
      neuen Datei kopiert). Alle Herkuenfte per lokalem `pip show`/
      Go-Modul-Cache/gecachten TFT_eSPI-/ArduinoJson-Lizenzdateien
      direkt geprueft (nicht geraten), nur `esp-web-tools` per
      Websuche verifiziert (Apache-2.0 laut oeffentlichem Paket-Index,
      im Dokument als "pruefen gegen die Originalquelle" markiert, da
      kein 100% sicherer Primaerbeleg gefunden wurde).
      **In DMG/Windows-Paket/README/Web-Installer verlinkt/gebuendelt**:
      `mac/dmg_staging/` UND `mac/dmg_build/` bekamen beide eine Kopie
      von `THIRD-PARTY-LICENSES.md` + `licenses/` (DMG neu gebaut, 0
      Token-/Username-Treffer bestaetigt). `windows/installer/
      installer.nsi` installiert jetzt ebenfalls beides mit (Installer
      neu gebaut, 0 Username-Treffer bestaetigt). `README.md` und
      `index.html` (Web-Installer-Seite) verlinken auf die neue Datei.
      Dabei gleich den fehlenden allgemeinen Haftungs-/Markenhinweis
      aus Teil 4 mit in `README.md` ergaenzt (Synergie, siehe dort).
- [x] **3c fertig (nur Pruefung + Vorschlag, NICHTS selbst geaendert)**:
      `LICENSE.md` sagt bereits klar und unmissverstaendlich, dass
      Fremdbestandteile ihre eigenen Lizenzen behalten ("under their
      own licenses") - das ist in Ordnung. ABER zwei Punkte fuer Alex:
      1. Der dortige Verweis zeigt noch auf den alten, engeren Pfad
         `mac/dmg_staging/THIRD-PARTY-LICENSES.md` statt auf die neue
         `THIRD-PARTY-LICENSES.md` in der Repo-Wurzel - Vorschlag:
         Zeile 29-31 ersetzen durch "...see [THIRD-PARTY-LICENSES.md]
         (THIRD-PARTY-LICENSES.md) for details."
      2. **Wichtigerer, neuer Fund** (nicht Teil des urspruenglichen
         Teil-1-Suchrasters, aber im selben Geist): Zeile 3 von
         `LICENSE.md` lautet woertlich "Copyright (c) Al** De*****.
         All rights reserved." - ein vollstaendiger Klarname (nicht
         nur ein Benutzername wie in Teil 1b), oeffentlich im Repo seit
         dem allerersten Commit. Das ist bei einer selbst verfassten
         Lizenz etwas GANZ ANDERES als die technischen Pfad-/ID-Funde
         aus Teil 1 - sehr wahrscheinlich bewusst/gewollt so
         eingetragen (uebliche Praxis fuer eine eigene Lizenz), aber
         KEIN automatischer technischer Zufallsfund, deshalb wird
         hier NICHTS vorgeschlagen oder geaendert, nur gemeldet: Alex
         entscheidet, ob der Klarname bleibt oder durch das
         ueberall sonst im Projekt genutzte Pseudonym "Eiswolf-BG"
         ersetzt werden soll.
- [x] **3d fertig**: eine Unklarheit gefunden (pyserial - kein lokal
      vorliegender exakter Lizenztext, nur die Gattungsbezeichnung
      "BSD" aus den Paketmetadaten; Hinweistext mit Verweis auf die
      Originalquelle statt erfundenem Text angelegt, siehe oben) -
      kein unvertraeglicher Fund (alle anderen Lizenzen - MIT/BSD/
      FreeBSD/Apache-2.0/OFL/LGPL-2.1-mit-korrektem-Quelloffenlegungs-
      Hinweis - sind alle untereinander und mit der eigenen
      "privat/nicht-kommerziell"-Lizenz problemlos vertraeglich, da
      Eiswolfs iBoard selbst nicht weiterverkauft/relizenziert wird).

### Teil 4: Warnhinweise in den Anleitungen
- [x] **Lueftersteuerung/Root-Hilfsdienst fertig**: `tools/fan-helper/
      README.md` um "If the fan seems stuck" (AUTO als der eigentliche
      Notbefehl - ein eigenes "RESET"-Kommando existiert in `fand.py`
      NICHT, nachgeprueft statt angenommen; Mac-Neustart als letzter,
      garantiert wirksamer Ausweg) und "Use at your own risk" (root-
      Rechte, Verweis auf LICENSE.md "as is", Verweis auf die MIT-
      lizenzierte MacFanControl-Herkunft) ergaenzt.
- [x] **Ruhemodus/Windows-Shutdown fertig**: bereits in 2b ausfuehrlich
      in `docs/TOKEN.md` behandelt ("Replay protection"-Abschnitt,
      inkl. ehrlicher Grenzen-Beschreibung "this is not encryption").
- [x] **Windows setup.exe fertig**: `windows/README.md` neuer Abschnitt
      "Before you run it: SmartScreen and antivirus" - SmartScreen-
      Klick-Anleitung, SHA-256-Pruefsumme vergleichen (PowerShell/
      shasum-Befehle fuer beide Plattformen), Verweis auf "selbst
      bauen" als Alternative. **Pruefsummen fuer den geplanten
      v1.1.1-Release notiert** (siehe Abschluss unten) - nicht fest in
      die Anleitung geschrieben (wuerde bei jedem neuen Build veralten),
      stattdessen Verweis auf die GitHub-Release-Seite als
      Autoritaetsquelle.
- [x] **Mac-App fertig**: Gatekeeper-Warnung UND "Lokales Netzwerk"-
      Abfrage waren schon in `mac/dmg_staging/INSTALLATION.md`
      abgedeckt (frueherer Auftrag) - nachgeprueft, nichts zu
      ergaenzen.
- [x] **Allgemeiner Haftungs-/Markenhinweis fertig**: neuer Abschnitt
      "Not affiliated with Apple, Microsoft, or anyone else named
      here" in `README.md` (zusammen mit 3b ergaenzt, selbe Stelle).
- [x] **SECURITY.md fertig**: neu angelegt (Repo-Wurzel) - "Report a
      vulnerability" ueber GitHubs privates Melde-Formular (Security-
      Tab), unterstuetzte Version = jeweils neuester Release, Hinweis
      "Token-Verdacht -> einfach neu erzeugen" statt eines Melde-
      Prozesses (der Token ist rein lokal, nichts zu koordinieren).
      **Kein Klarname, keine private E-Mail-Adresse** - nur Verweise
      auf GitHub-eigene Mechanismen.

### Teil 5: Abgleich der installierten Version (nur lesen)
- [x] **fertig**: drei verschiedene Staende, bewusst so (Auftrag:
      nichts pushen/veroeffentlichen):
      - **Installierte App** (`/Applications/Eiswolfs iBoard.app`):
        Version **1.1.1**, Bundle-ID `com.eiswolf.iboard.app` (neu/
        neutral) - enthaelt ALLES aus heute (Bereinigung, neues HMAC+
        Zaehler-Protokoll, Token-Teilen-Funktion, Sicherheits-Menue).
      - **Lokaler Git-Stand** (`a0d9877`, 39 ungetrackte/geaenderte
        Dateien): letzter COMMIT ist noch der alte v1.1-Stand - alle
        heutigen Aenderungen liegen nur im Arbeitsbaum, wie vom
        Auftrag verlangt (nichts committet).
      - **Oeffentlicher GitHub-Release**: `v1.1` (03.11., ohne die
        heutige Sicherheits-/Lizenz-Arbeit), davor `v1.0`.
      Installierte App ist damit WEITER als sowohl der letzte lokale
      Commit als auch der oeffentliche Release-Stand - korrekt und
      beabsichtigt fuer diesen Auftrag, aber wichtig fuer Alex zu
      wissen: die App auf diesem Mac ist aktuell NICHT das, was
      jemand sonst beim Herunterladen von GitHub bekommt.

### v1.1.1 Release-Vorbereitung (nur lokal, NICHTS veroeffentlicht)

- [x] **Windows-Listener live aktualisiert und getestet** (03.11., im
      Rahmen der Migration): neu gebauten `EiswolfsIBoardListener-
      Setup.exe` (mit dem neuen signierten Protokoll, s.o.) per
      USB-Stick ("EISWOLF") auf Alex' Windows-Laptop uebertragen -
      Pruefsumme vor UND nach dem Kopieren identisch bestaetigt. Alex
      hat den alten Listener selbst ueber die Taskleiste beendet, den
      neuen Installer ausgefuehrt (kein Admin/Passwort noetig,
      `RequestExecutionLevel user`) und direkt danach den Laptop
      erfolgreich ueber das CYD-Menue heruntergefahren - **funktioniert
      bestaetigt, live getestet**. Damit sprechen CYD, Mac-App und
      Windows-Listener jetzt wieder durchgehend dasselbe (neue,
      signierte) Protokoll.
- Versionsnummern bereits ueberall auf **1.1.1** gesetzt:
  `mac/eiswolfs_iboard.spec`/`mac/sender.py` (`APP_VERSION`),
  `manifest.json`, `index.html` (Versions-Badge).
- Web-Flasher-Dateien neu gebaut (`bootloader.bin`/`partitions.bin`/
  `firmware.bin` in der Repo-Wurzel, Wegwerf-Token-Verfahren wie
  gewohnt, 0/0 Token-/Username-Treffer bestaetigt, danach echter Token
  wiederhergestellt und Testgeraet neu geflasht).
- **Pruefsummen (SHA-256) der fuer den Release vorgesehenen Dateien**:
  ```
  be5cb25582b455e905811b2d9cd8146e54c71865b524b6905b8f26a449223ea2  dist/Eiswolfs-iBoard.dmg
  ffdaa774d3270efa963fa969e2f57200b457c2feadc3ac64fc5c332e7594ac14  dist/Eiswolfs-iBoard-CYD-merged.bin
  31412776ac94298f65040041d27681f1dc2e6accf716906f63790d0805196954  windows/installer/EiswolfsIBoardListener-Setup.exe
  ```
  (Diese Werte aendern sich bei jedem erneuten Build minimal, selbst
  bei identischem Quellcode - normales Verhalten bei einem Embedded-/
  PyInstaller-Build ohne "reproducible build"-Vorkehrungen, kein Fehler.
  Vor der tatsaechlichen Veroeffentlichung die Pruefsummen der dann
  WIRKLICH hochgeladenen Dateien neu berechnen.)

**Release-Notizen-Entwurf (zweisprachig DE/EN, wie vom Auftrag
verlangt - bisherige v1.0/v1.1-Releases waren nur englisch, das ist
also eine Umstellung, Alex entscheidet ob gewollt):**

> **Deutsch:**
> Dieses Release ist ein reines Sicherheits- und Aufraeum-Release,
> keine neuen sichtbaren Funktionen.
>
> - Befehle (Luefter-Boost, Ruhemodus, Windows-Shutdown) sind jetzt
>   gegen Wiederholung geschuetzt (steigender Zaehler + Pruefsumme
>   statt eines mitlesbaren Tokens am Zeilenende) - siehe docs/TOKEN.md
> - Alle veroeffentlichten Dateien (Firmware, Mac-App, Windows-
>   Listener) enthalten keine persoenlichen Pfade/Kennungen mehr
> - Vollstaendige Liste aller verwendeten Open-Source-Komponenten samt
>   Lizenzen: THIRD-PARTY-LICENSES.md
> - Neue SECURITY.md fuer Sicherheitsmeldungen
>
> Mac-App und Windows-Listener MUESSEN zusammen mit der CYD-Firmware
> aktualisiert werden (neues, nicht abwaertskompatibles Befehlsformat -
> ein Uebergangsschalter fuer das alte Format existiert, ist aber
> standardmaessig aus).
>
> **English:**
> This release is a security and cleanup release, no new user-facing
> features.
>
> - Commands (fan boost, sleep, Windows shutdown) are now protected
>   against replay (a rising counter + checksum instead of a readable
>   token at the end of the line) - see docs/TOKEN.md
> - All published files (firmware, Mac app, Windows listener) no
>   longer contain any personal paths/identifiers
> - Full list of every open-source component used, with licenses:
>   THIRD-PARTY-LICENSES.md
> - New SECURITY.md for security reports
>
> The Mac app and Windows listener MUST be updated together with the
> CYD firmware (new, non-backward-compatible command format - a
> transition switch for the old format exists but is off by default).

**Welche alten Dateien Alex auf GitHub ersetzen/loeschen sollte** (aus
`local/BEREINIGUNG.md`, hier nochmal kompakt): die Anhaenge an den
Releases `v1.0` und `v1.1` (DMG/Firmware/Windows-Setup) enthalten noch
den alten Benutzernamen in Build-Pfaden - beim v1.1.1-Release automatisch
durch die neuen, bereinigten Dateien ersetzt; die ALTEN Release-Anhaenge
bei `v1.0`/`v1.1` bleiben bestehen, bis Alex sie dort manuell entfernt
(optional, siehe `local/BEREINIGUNG.md`).

**Git-/gh-Befehle, die Alex selbst ausfuehren muesste (NICHTS davon
wurde ausgefuehrt)**:
```bash
cd "~/Eiswolfs-iBoard"
git add -A
git commit -m "v1.1.1: replay-protected commands, cleaned personal paths, full license audit"
git tag -a v1.1.1 -m "Eiswolfs iBoard v1.1.1 - security & cleanup release"
git push origin main
git push origin v1.1.1
gh release create v1.1.1 \
  dist/Eiswolfs-iBoard.dmg \
  dist/Eiswolfs-iBoard-CYD-merged.bin \
  dist/FLASHEN.md \
  windows/installer/EiswolfsIBoardListener-Setup.exe \
  --repo Eiswolf-BG/Eiswolfs-iBoard-CYD \
  --title "v1.1.1" \
  --notes "<Text oben einfuegen>"
```
Zusaetzlich fuer `CLAUDE.md` vorher pruefen: Datei bleibt lokal (neue
`.gitignore`-Regel) - falls sie schon mal committet war (ja, war sie),
bleibt sie im Staging-Index bis Alex explizit
`git rm --cached CLAUDE.md` ausfuehrt (siehe 1b-2) - sonst wird `git
add -A` sie trotz `.gitignore` weiterhin als "modified" einschliessen
(gitignore wirkt nur auf NEUE Dateien, nicht auf bereits getrackte).

### Abschluss
- [x] Deutsche Zusammenfassung erstellt (siehe Chat-Antwort am Ende
      dieser Session) - Auftrag "Sicherheit und Lizenzen" vollstaendig
      abgeschlossen (Teil 1, 1b, 2, 3, 4, 5), nichts committet/getaggt/
      gepusht.

## Nachtrag: Klarname ersetzen + gefuehrte Migration

Neuer Auftrag (Fortsetzung): A) Klarname in LICENSE.md durch
"Eiswolf-BG" ersetzen, B) Alex SCHRITT FUER SCHRITT (einfache
deutsche Saetze, ein Schritt nach dem anderen, auf Rueckmeldung
warten) durch die Migration auf die neue Kennung fuehren - sie kennt
sich mit Terminal/sudo nicht aus.

### Teil A: Klarname ersetzen
- [x] `LICENSE.md` Copyright-Zeile auf "Eiswolf-BG" geaendert (dabei
      gleich den veralteten Pfadverweis auf
      `mac/dmg_staging/THIRD-PARTY-LICENSES.md` auf die neue
      Wurzel-Datei korrigiert, war ohnehin schon als Vorschlag notiert).
- [x] Projektweite Suche (voller Name UND Nachname allein) in allen
      Textdateien (ohne `.git`/`.pio`/`venv`/`__pycache__`/`go`-Cache):
      **0 Treffer** ausser der bereits behobenen LICENSE.md-Zeile.
- [x] Gebaute Dateien geprueft (`mac/dist/.../Info.plist`,
      installierte App, DMG, Windows-`.exe`/`setup.exe`, Firmware/
      Web-Flasher-Bins): **0 Treffer ueberall** - `Info.plist` hatte nie
      ein `NSHumanReadableCopyright`-Feld gesetzt. **Kein Neubau
      irgendeiner Komponente noetig**, reine Textaenderung in einer
      einzigen Datei.
- [x] Git-Autor geprueft (maskiert): war schon immer das Pseudonym mit
      GitHub-No-Reply-Adresse (`Ei*** <37***@users.noreply.github.com>`),
      in JEDEM Commit der Historie - kein Klarname, keine private
      E-Mail, keine Aenderung an der Git-Konfiguration noetig.
- [x] `local/BEREINIGUNG.md` um eigenen Abschnitt "Klarname in
      LICENSE.md" ergaenzt - beide Wege fuer die alte Historie (neues
      Repo vs. `git filter-repo`/BFG + Force-Push) beschrieben, nichts
      davon ausgefuehrt, keine Empfehlung.

**Teil A komplett abgeschlossen.**

### Teil B: Gefuehrte Migration
- [x] Migrationsskripte (`local/migrate-to-neutral-id.sh`,
      `local/migrate-fan-helper-sudo.sh`) nochmal gruendlich gegengelesen
      + erneuter Trockenlauf (DRY_RUN=1) - keine Fehler gefunden.
- [x] B1: Luefter lief bereits im Automatik-Modus, kein Boost aktiv -
      keine Vorbereitungsaenderung noetig.
- [x] B2: `migrate-to-neutral-id.sh` selbst ausgefuehrt (kein sudo) -
      alter App-LaunchAgent entfernt, neue App (`com.eiswolf.iboard.app`)
      lief schon in /Applications. Alex hat die App neu gestartet,
      "Lokales Netzwerk" erlaubt und "Beim Login starten" wieder
      angehakt.
- [x] B3: sudo-Schritt (Luefter-Dienst) per `osascript -e 'do shell
      script ... with administrator privileges'` ausgeloest - Alex hat
      ihr Passwort nur im nativen macOS-Fenster eingegeben, Karl hat es
      nie gesehen. Technischer Haken dabei gefunden und geloest: bei
      `with administrator privileges` ist `SUDO_USER` leer (laeuft
      direkt als root, nicht ueber ein `sudo`-Login), das Skript
      erwartet es aber zur Ermittlung des Zielbenutzers - behoben durch
      explizites `SUDO_USER='<echter Name, zur Laufzeit per id -un
      ermittelt>'` in der osascript-Befehlszeile (nur als einmaliger
      Laufzeit-Parameter, nicht im Skript selbst hartkodiert). Alter
      LaunchDaemon entfernt, neuer (`com.eiswolf.iboard.fanhelper`)
      installiert und laeuft, Status ueber den Unix-Socket geprueft
      (`mode: AUTO, boost_active: false`).
- [x] Luefter-Boost auf dem echten CYD-Geraet getestet - funktioniert
      ("geht").

**Teil A und Teil B beide vollstaendig abgeschlossen.** Nichts
committet/getaggt/gepusht. Offen bleibt weiterhin nur die alte
Git-Historie/alte Release-Dateien (siehe `local/BEREINIGUNG.md`) -
Alex' Entscheidung, keine Eile.

## Auftrag Controls-Screen: Fortschritt

Neuer Auftrag: zweiter Bildschirm "Controls" (Wischen nach links vom
Hauptscreen), vier Kacheln (Acer Predator/Mac Mini M4/Luefter/Menue),
Hauptscreen wird reine Anzeige. NUR `cyd/`, NICHTS committen/taggen/
pushen. SCHUTZREGEL: Flightradar/Toolbox/Solarguardian nicht anfassen,
`mac/`/`windows/`/Befehlsprotokoll nicht aendern.

Codebasis-Ueberblick (vor Beginn gelesen): alles in `cyd/src/main.cpp`
(anonymer Namespace, keine Screen-Aufteilung in eigene Dateien - neue
Funktionen werden dort nach demselben Muster ergaenzt, kein neues
Modul). `TouchCalib::wasTapped()` feuert beim Loslassen, hat KEINE
eigene Wisch-Erkennung. Lange Haltegriffe (bisher FAN-Boost/Shutdown)
pollen `TouchCalib::rawPoint()` direkt mit eigener 150ms-Release-
Entprellung (Vorbild fuer den neuen Acer-Longpress). `Storage::
DeviceConfig` ist additiv (SD-JSON + NVS-Fallback, fehlende Felder
= Default). `i18n.h/.cpp` hat nur DE/EN (nicht 8 Sprachen wie die
Schwesterprojekte). Farben: TFT_CYAN=CPU, TFT_PURPLE=GPU,
TFT_ORANGE=RAM, TFT_GREEN=FAN, TFT_YELLOW=PWR (aus `TILES[]`/
`pwrStripeRect` im Hauptscreen) - fuer Controls wiederverwendet, keine
neuen Farbwerte. `Config::FAN_LONGPRESS_MS=500` (0,5s) wird fuer den
neuen Acer-Longpress wiederverwendet. ASCII-Hinweis: FreeSansBold-
Fonts stellen KEINE Umlaute dar (bestehender Code vermeidet sie
konsequent, z.B. "Luefter" nicht "Lüfter", "Menue" nicht "Menü") -
Controls-Kachel-Labels folgen demselben Muster, auch wenn der Auftrag
selbst "Lüfter"/"Menü" schreibt (Auftrag: "richte dich nach dem Code,
nicht nach dieser Beschreibung").

### Schritt 1: Seiten und Wischen
- [x] `Page`-Enum (Main/Controls), Wisch-Erkennung in `touch_calib.h/.cpp`
      (`TouchCalib::pollGesture()`, liefert Tap ODER SwipeLeft/Right,
      nie beides fuer dieselbe Beruehrung) - Entprellung 150ms beim
      Loslassen, exakt wie die bestehenden Lang-Antippen-Griffe.
- [x] Config-Konstanten: `SWIPE_MIN_DX_PX=80`/`SWIPE_MAX_DY_PX=40`/
      `SWIPE_MIN_HOLD_MS=80`/`TOUCH_RELEASE_DEBOUNCE_MS=150` in `config.h`.
- [x] Seitenpunkte unten NUR auf Controls (`drawPageDots()`, Main-Reihe2
      liess nur 3px statt der geforderten 8px, siehe Rechnung oben).
- [x] Wischen auch auf "Keine Verbindung"-Screen moeglich (eigener
      Zweig in `loop()`, "Menue"-Button bleibt zusaetzlich bestehen).

### Schritt 2: Controls-Layout
- [x] 4 Kacheln (146x80, Radius 7) an den 4 Positionen, Streifen
      (abgerundet oben)+Koerper (`#1A1A1A`), Titel "Controls" per
      `pickClearFont()` (automatische Breiten-/Groessenpruefung).
- [x] Icons per TFT_eSPI-Primitiven: Power = `drawArc()`+`drawWideLine()`
      (Luecke oben, Strich durch die Luecke), Luefterrad = Nabe
      (`fillCircle`) + 3 Fluegel (`fillEllipse`, 120 Grad versetzt,
      NICHT individuell rotiert - vereinfachte, aber eindeutig als
      Luefter erkennbare Variante), Menue = 3 Balken (`fillRect`).
- [x] `labelLaptop`/`labelMac` in `storage.h/.cpp` (additiv, SD+NVS,
      Standard "Acer Predator"/"Mac Mini M4") - siehe Abschluss unten,
      wie Alex sie aendern kann.

### Schritt 3: Kachel-Verhalten
- [x] Acer: Tap=WoL (gruen), Halten=Shutdown (rot, feuert SOFORT beim
      Erreichen der Haltezeit, zusaetzliche Absicherung: Finger muss
      beim Ausloesen noch auf der Kachel sein), "n/v" dauerhaft wenn
      `wolMacAddress` leer.
- [x] Mac Mini: Tap=Sleep (gruen), "n/v" ohne `sleep_ack`-Aenderung
      binnen 3s (`Config::FAN_CMD_CONFIRM_TIMEOUT_MS`).
- [x] Luefter: Tap=Boost an/aus (gruen beim Einschalten+3s-Bestaetigung
      wie bisher/rot beim Ausschalten), Zustand aus `fan_boost`/
      `fan_left`/`fan_ok` im Datenstrom, NICHT aus dem eigenen Befehl.
      "Bis Tippen"-Modus zeigt "AN"/"ON" statt Restzeit.
- [x] Menue: Tap=oeffnet bestehendes `runMainMenu()` unveraendert,
      kehrt zu Controls zurueck.
- [x] Alle drei interaktiven Kacheln: 3s-Sperre nach Befehl
      (`CTRL_TAP_LOCKOUT_MS`), Befehl erst beim Loslassen, Wischen
      loest nie eine Kachel aus (siehe `pollGesture()`).

### Schritt 4: Hauptscreen wird reine Anzeige
- [x] Alle Touch-Handler vom Hauptscreen entfernt (PWR-Tap/CPU-Temp-Tap+
      -Halten/FAN-Halten/Namens-Tap) - toter Code mit entfernt
      (`PwrFeedback`-Enum, `drawPwrFeedbackText()`,
      `showFanStripeNotAvailable()`, `pwrTileRect`/`headerLeftRect`,
      `wolFeedbackActive`/`cpuTempLongFired` + zugehoerige Konstanten).
      Drei nun ungenutzte i18n-Strings (`PWR_SLEEP_FEEDBACK`/
      `WOL_SENT_FEEDBACK`/`SHUTDOWN_SENT_FEEDBACK`) ebenfalls entfernt
      (DE+EN+Enum synchron).
- [x] Nur Wischen + Bootscreen-Skip + Notausgang bleiben - Anzeige
      selbst (Layout/Farben/Groessen) unveraendert, inkl. FAN-Streifen-
      BOOST-Anzeige (bleibt als reiner Spiegel stehen).

### Schritt 5: Verbindungsverhalten
- [x] Controls bleibt bei fehlenden Daten, "Keine Verbindung" nur auf
      Main (Zustand wird im Hintergrund weiter verfolgt, beim Wischen
      zurueck zu Main wird der dann aktuelle Zustand angezeigt).
- [x] Empfang/Timeouts laufen auf Controls normal weiter (KEINE eigene
      blockierende Schleife wie `runMainMenu()` - `handleControlsPage()`
      wird aus `loop()` heraus bei jedem Durchlauf aufgerufen, Luefter-
      Restzeit zaehlt dadurch live weiter).

### Schritt 6: Bauen, flashen, testen
- [x] `pio run` - Build erfolgreich im ersten Versuch (keine neuen
      Warnungen), Flash: 72,4% (948.465 Bytes), RAM 14,7%.
- [x] Upload auf `/dev/cu.wchusbserial31410` (Eiswolfs-iBoard-Testgeraet) -
      erfolgreich.
- [ ] Interaktive Tests a-k mit Alex (laufen jetzt im Chat).
- [ ] README/docs zweisprachig aktualisiert (nur Arbeitsordner).
- [x] Flightradar/Solarguardian unveraendert bestaetigt (`git status`
      beide leer). Toolbox zeigt Aenderungen, aber das sind Alex'
      eigene, bereits VOR dieser Sitzung vorhandene unfertige
      Aenderungen (von mir nicht angefasst, nur per `git status`
      gelesen) - siehe Hinweis an Alex im Chat.

**Code fertig, geflasht - interaktive Tests mit Alex laufen jetzt.**

## Auftrag Controls-Optik und USB-Sync: Fortschritt

Nachtrag (03.11., nach dem Controls-Screen-Auftrag): A) Optik-Bugs auf
dem Controls-Screen beheben, B) USB-Verbindung nach dem Flashen
selbststaendig wiederherstellen, C) Mac-App neu bauen + installierte
App aktualisieren. `mac/` durfte NUR fuer Teil B angefasst werden,
sonst nur `cyd/`. Umlaute (Luefter/Menue) bewusst NICHT angefasst
(Alex' ausdruecklicher Wunsch, kommt spaeter mit eigener Schrift).

### Teil A: Optik-Bugs - Ursachen (nicht geraten, nachgeprueft)

- [x] **Power-Symbol auf dem Kopf**: `TFT_eSPI::drawArc()` zaehlt den
      Winkel NICHT wie ein Zifferblatt von 12 Uhr, sondern von 6 Uhr
      (0 Grad = unten), im Uhrzeigersinn - direkt im mitgelieferten
      Beispiel-Sketch bestaetigt (`TFT_eSPI/examples/.../Draw_Arc/
      Draw_Arc.ino`: "0 degrees is at 6 o'clock position"). Die Luecke
      lag dadurch faelschlich unten statt oben. Fix: Luecke jetzt um
      180 Grad zentriert (215..145, das ist bei dieser Zaehlung oben),
      siehe `drawPowerIcon()`.
- [x] **Luefterrad ohne Nabe ("Klecks")**: Nabe wurde in DERSELBEN
      Farbe wie die Fluegel gezeichnet und dadurch komplett ueberdeckt.
      Zusaetzlich zeichnete `fillEllipse()` alle drei Fluegel IMMER
      gleich ausgerichtet (nur um die Nabe verschoben, nicht wirklich
      gedreht) - TFT_eSPI kann Ellipsen nicht rotieren. Fix: Fluegel
      jetzt als vorab am Rechner erzeugte, echte geglaettete Alpha-
      Bitmaske mit tatsaechlich gedrehten Fluegeln (`tools/
      generate_controls_icons.py` -> `src/icon_fan.h`, 32x32 Byte,
      zur Laufzeit per `drawAlphaIcon()` gegen die jeweils aktuelle
      Hintergrundfarbe geblendet), Nabe wird danach separat in der
      Hintergrundfarbe gezeichnet ("stanzt" ein Loch).
- [x] **Seitenpunkte hellblau statt dunkelgrau**: Der inaktive Punkt
      nutzte den Hex-Wert `0x5555` in der Annahme, das sei "neutrales
      Grau" - ist es bei RGB565 NICHT (5/6/5 Bit pro Kanal, `0x5555`
      ruft korrekt zurueckgerechnet Cyan/Tuerkis (82,170,172) hervor,
      nicht Grau). Fix: `ctrlColor(r,g,b)`-Hilfsfunktion (identische
      Formel wie `TFT_eSPI::color565()`) fuer ALLE Controls-Farben
      eingefuehrt, `CTRL_DOT_INACTIVE = ctrlColor(85,85,85)` = korrekt
      `0x52AA`. `CTRL_BODY_BG` (#1A1A1A) war beim Nachrechnen bereits
      korrekt (0x18C3), trotzdem jetzt ueber dieselbe Hilfsfunktion
      definiert.
- [x] **Streifen an allen 4 Ecken rund statt nur oben**: `fillRoundRect()`
      rundet IMMER alle vier Ecken. Fix: zwei neue Helfer
      `fillRoundRectTop()`/`fillRoundRectBottom()` (nutzen
      `TFT_eSPI::fillCircleHelper()` mit `cornername` 2 bzw. 1 direkt,
      derselbe Mechanismus, den `fillRoundRect()` selbst intern
      verwendet) - Streifen nur oben rund, Koerper nur unten rund,
      zusammen ein rund umrissenes 146x80-Rechteck
      (`drawCtrlTileShape()`).
- [x] **Farben blass/abweichend vom Entwurf**: alle Streifenfarben sind
      bereits 1:1 die Dashboard-Konstanten (`TFT_YELLOW`/`TFT_CYAN`/
      `TFT_GREEN`/`TFT_PURPLE`, keine neuen Werte). Abweichungen von
      den Referenzwerten im Auftrag:

      | Kachel | Konstante | TFT_eSPI-Wert | Referenz im Auftrag | Abweichung |
      |---|---|---|---|---|
      | Hintergrund | `TFT_BLACK` | (0,0,0) | (0,0,0) | keine |
      | Koerper | `CTRL_BODY_BG` | (24,24,24)* | (26,26,26) | minimal (RGB565-Rundung) |
      | Acer-Streifen | `TFT_YELLOW` | (255,255,0) | (245,230,0) | gering |
      | Mac-Streifen | `TFT_CYAN` | (0,255,255) | (46,230,240) | gering-mittel (kein Rot-Anteil) |
      | Fan-Streifen | `TFT_GREEN` | (0,255,0) | (60,224,60) | mittel (kein R/B-Anteil) |
      | Menu-Streifen | `TFT_PURPLE` | (128,0,128) | (138,75,255) | **deutlich** (Blau-Anteil nur 128 statt 255) |
      | Gruen-Feedback | `TFT_GREEN` | (0,255,0) | (60,224,60) | mittel (wie Fan-Streifen) |
      | Rot-Feedback | `TFT_RED` | (255,0,0) | (255,59,48) | gering |
      | Orange-Feedback/Boost | `TFT_ORANGE` | (255,180,0) | (255,159,28) | gering |
      | Seitenpunkt aktiv | `TFT_WHITE` | (255,255,255) | (255,255,255) | keine |
      | Seitenpunkt inaktiv | `CTRL_DOT_INACTIVE` | (82,170,172)* | (85,85,85) | war der eigentliche Bug, jetzt behoben |

      (* nach RGB565-Hin-und-Zurueckrechnung, siehe `tools/
      preview_controls.py::rgb565_roundtrip()`)

      Auftrag: "nimm die Dashboard-Konstante und melde es mir" - die
      deutlichste Abweichung ist **GPU/Menu-Lila** (TFT_PURPLE hat nur
      halbe Saettigung im Blaukanal gegenueber der Referenz). Keine
      Aenderung vorgenommen, nur gemeldet, wie verlangt.
- [x] **Vorschau-PNG erzeugt** (`tools/preview_controls.py`, nutzt
      dieselben Formeln/dieselbe Luefterrad-Maske wie main.cpp) -
      Alex gezeigt, liegt auch unter `~/Desktop/controls_preview.png`.

### Teil B: USB-Sync nach dem Flashen - Messung und Ursache

- [x] **Live nachgemessen statt vermutet** (Config::DEBUG_LOG kurz
      eingeschaltet, Mac-Entwicklungskopie mit Zeitstempel-Log laufen
      lassen, mehrere echte Flash-Durchlaeufe ausgeloest):
      - Eine bereits offene `serial.Serial`-Verbindung auf dem Mac
        bemerkt NICHT, dass das CYD gerade geflasht/neu gestartet wird -
        40 Schreibversuche im Sekundentakt liefen ueber einen
        kompletten Flash-Vorgang (Beginn bis Ende inkl. Neustart)
        hinweg durch, OHNE eine einzige Exception. macOS meldet den
        Konflikt auf dieser Ebene schlicht nicht.
      - Direkter Handshake-Test Minuten nach einem Flash (unabhaengig
        von der Mac-App): "IBOARD?" sofort nach `s.open()` gesendet ->
        KEINE Antwort. Erst mit 9s Wartezeit NACH `s.open()` kam
        "IBOARD_OK" sofort zurueck. Erklaerung: das blosse OEFFNEN des
        Ports loest selbst einen Reset des ESP32 aus (bekanntes
        Verhalten der CH340/CP2102-Bruecke auf diesen CYD-Boards) -
        jeder Verbindungsversuch startet also ungewollt einen neuen,
        7s langen Bootscreen, waehrend dessen "IBOARD?" bisher ins
        Leere lief (`pollUsbSerial()` lief nur im normalen Betrieb).
      - Simulierte Wiederverbindung (Oeffnen/Schliessen alle 5s wie
        die bestehende Scan-Schleife) hat sich nach GENAU EINEM
        gescheiterten Versuch selbst geheilt (Versuch 0 bei t=3,1s
        gescheitert, Versuch 1 bei t=9,1s erfolgreich und alle
        folgenden auch) - die Wiederverbindung an sich funktioniert
        also, nur fehlte der Mac App ein Grund, sie ueberhaupt zu
        starten.
      - **Eigentliche Ursache gefunden**: `UsbLink._scan_loop()`
        versucht NUR eine Neuverbindung, wenn `self.ser is None` ist.
        Weil ein Schreibfehler (siehe oben) so gut wie nie auftritt,
        bleibt `self.ser` nach dem Flashen auf dem ALTEN (toten)
        Verbindungsobjekt stehen - die App "glaubt" fuer immer, sie sei
        verbunden, obwohl sie es nicht mehr ist. Deckt sich mit Alex'
        Beobachtung: ohne App-Neustart kommt es nie von selbst zurueck.

### Teil B: Fix

- [x] **`mac/usb_link.py`**: `_maybe_check_liveness()` - bei
      bestehender Verbindung alle 8s erneut "IBOARD?" senden, bleibt
      die "IBOARD_OK"-Antwort laenger als 3s aus, gilt die Verbindung
      als tot (`self.ser = None`) - danach greift die (oben bestaetigt
      funktionierende) normale Wiederverbindung. `read_lines()` setzt
      den Antwort-Flag zurueck UND erkennt jetzt auch eigene Lesefehler
      (vorher nur `write_line()`).
- [x] **Kollision mit dem Flash-Werkzeug entdeckt und behoben** (beim
      ersten Testlauf dieses Fixes selbst live aufgetreten): schreibt
      die Mac-App waehrend des Flashens IRGENDETWAS auf den Port
      (z.B. die neue Lebenszeichen-Abfrage), bricht `pio run --target
      upload` mit "chip stopped responding" ab - zweimal reproduziert,
      beide Male verschwand der Fehler sofort nach Beenden des Mac-
      Prozesses. Fix: `_flash_tool_running()` (`pgrep -f esptool`) wird
      vor JEDER Port-Aktion geprueft; laeuft ein Flash-Werkzeug, gibt
      `_scan_loop()` den Port sofort frei und ruehrt ihn nicht an.
- [x] **`cyd/src/main.cpp`**: `pollUsbSerial()` wird jetzt zusaetzlich
      aus dem Bootscreen, der Tipp-Sperre danach, `runBrightnessMenu()`,
      `showHelpPopup()`, `runSetupMenu()`, `runFanSettingsMenu()` und
      `runMainMenu()` aufgerufen (vorher nur aus `loop()`/Hauptscreen/
      Controls) - "IBOARD?" wird jetzt in jedem dieser Zustaende
      beantwortet. `SetupFlow`-Bildschirme (Ersteinrichtung, WLAN-
      Auswahl, Kalibrierung) bewusst NICHT angefasst - seltene,
      lang dauernde, von Alex selbst ausgeloeste Vorgaenge, bei denen
      ein zeitgleicher Flash unrealistisch ist, und `pollUsbSerial()`
      ist dort (andere Uebersetzungseinheit) nicht ohne Weiteres
      erreichbar.
      Zusaetzlich: `setup()` sendet direkt nach `Serial.begin()`
      EINMAL unaufgefordert "IBOARD_OK", bevor der Bootscreen ueberhaupt
      beginnt.
- [x] **End-zu-Ende bestaetigt** (drei echte Flash-Durchlaeufe mit
      laufender Mac-Entwicklungskopie, OHNE sie zu beenden):
      1. Flash waehrend bestehender Verbindung -> Flash-Werkzeug
         erkannt, Port freigegeben, Flash erfolgreich, automatische
         Wiederverbindung **ca. 2,1 Sekunden nach Flash-Ende** (17,2s
         nach Flash-Start).
      2. Zweiter Durchlauf: Lebenszeichen-Abfrage griff zusaetzlich
         (Verbindung wurde als tot erkannt, kurz bevor die Flash-
         Erkennung selbst zuschlug) - beide Mechanismen greifen
         unabhaengig voneinander, wie beabsichtigt.
      3. App-Start waehrend das CYD bereits laeuft (nicht exakt
         waehrend des Bootscreens treffbar, aber: Verbindung kam
         binnen 1,5s zustande).
      **Ziel "spaetestens ca. 15s nach Neustart" klar erreicht/
      uebertroffen** (tatsaechlich ca. 2s nach Flash-Ende).

### Teil C: Mac-App neu bauen, testen

- [x] Versionsnummer vereinheitlicht: `mac/sender.py` hatte noch "1.1"
      (Darstellung im Menue), `mac/eiswolfs_iboard.spec` "1.1.1" (Bundle-
      Version) - beide inkonsistent zueinander UND zum Auftrag. Jetzt
      beide auf **1.1.2** (einzige inhaltliche Ursache fuer die
      bisherige Abweichung: zwei verschiedene Stellen pflegen die
      Versionsnummer, nicht nur eine - fuer kuenftige Releases im Blick
      behalten).
- [x] `pyinstaller` neu gebaut, ad-hoc signiert
      (`com.eiswolf.iboard.app`, v1.1.2), `codesign --verify --deep
      --strict` bestanden.
- [x] Token-Check: der Mac-App-Quellcode liest den Token ausschliesslich
      zur LAUFZEIT aus `config.json` (`config.get_fan_boost_token()`) -
      er steht nirgends im Quellcode/Build, anders als beim CYD-
      Wegwerf-Token-Verfahren ist hier technisch gar kein Leck moeglich.
      Trotzdem Textsuche wiederholt: App-Bundle, DMG (gemountet) und
      Firmware je **0 Treffer** fuer Benutzername/Klarname (die
      anfaengliche Meldung "12 Treffer" im App-Bundle war ein falsch-
      positiver, zu unscharfer Such-Regex-Fehler meinerseits, mit dem
      exakten Suchbegriff direkt auf 0 korrigiert).
- [x] DMG neu gebaut (`mac/dmg_build/` aktualisiert, `hdiutil create`),
      enthaelt weiterhin `THIRD-PARTY-LICENSES.md`/`licenses/`/
      `INSTALLATION.md`/`macmon_LICENSE.txt`/`Programme`-Symlink.
- [x] Installierte App ersetzt (`/Applications/Eiswolfs iBoard.app`,
      ueber Menue beendet -> ersetzt -> neu gestartet). Geprueft:
      "Beim Login starten" (LaunchAgent) laedt weiterhin denselben
      Pfad, `config.json`-Einstellungen unveraendert erhalten
      (`debug_log` nach dem Test wieder auf `false` zurueckgesetzt),
      Luefter-Hilfsdienst antwortet (`mode: AUTO, boost_active: false`).
- [x] Flightradar/Toolbox/Solarguardian erneut nur gelesen, unveraendert
      (Toolbox weiterhin mit Alex' eigenen, vorbestehenden Aenderungen -
      nicht von mir angefasst).

**Teil A, B und C umgesetzt und mehrfach live verifiziert.** Offen:
interaktive Sichtpruefung durch Alex am echten Geraet (Punkt a-d der
Testliste) und die destruktive Windows-Shutdown-Testzeile - beides nur
von ihr pruefbar. Nichts committet, getaggt oder gepusht.

### Nachtrag (Alex' Korrektur anhand des Vorschau-Vergleichs)

Alex hat die Vorschau mit dem urspruenglich bestaetigten Entwurf
verglichen: der Kachelkoerper im Entwurf ist reines Schwarz, nicht
`#1A1A1A` (der Soll-Wert im Auftragstext selbst war also schon
ungenau/veraltet gegenueber dem tatsaechlich bestaetigten Entwurf).
`CTRL_BODY_BG` jetzt `TFT_BLACK` statt `ctrlColor(26,26,26)` - der
Kachelkoerper verschmilzt dadurch optisch mit dem Bildschirm-
hintergrund, nur der farbige Streifen oben bleibt als Kachel-Umriss
sichtbar (siehe `controls_preview.png` auf dem Schreibtisch, neu
erzeugt). Gebaut und erneut aufs Testgeraet geflasht, WAEHREND die
installierte Mac-App lief (bestaetigt: die Flash-Werkzeug-Erkennung
aus Teil B funktioniert auch mit der echten installierten App, nicht
nur der Entwicklungskopie).

## Auftrag Umlaute und Umrandung: Fortschritt

Neuer Auftrag: A) echte Umlaute (ä ö ü Ä Ö Ü ß) auf CYD UND Mac-App-
Menue, Schrift dafuer aus dem Flightradar-Projekt (nur LESEN, Dateien
KOPIEREN, Flightradar selbst nicht veraendern), B) Controls-Kacheln
bekommen eine feine weisse 1px-Umrandung + Lueftersymbol-Kachel heisst
jetzt "Luefter Mac"/"Mac Fan" (ASCII bis die neue Schrift da ist).
Nur bauen/flashen, NICHT committen/taggen/pushen. SCHUTZREGEL:
Flightradar/Toolbox/Solarguardian nur lesen, IBOARD?/IBOARD_OK/
config.json-Schluessel/Dateinamen/launchd-Labels/Befehlsnamen bleiben
ASCII und unveraendert, SD-Ordner /iBoard bleibt.

### Teil A, Schritt A0: Umrandung + Label (zuerst, klein, ohne neue Schrift)
- [x] `Config::CTRL_OUTLINE_PX=1`/`CTRL_OUTLINE_COLOR=0xFFFF` in config.h
- [x] Weisse 1px-Umrandung um alle 4 Controls-Kacheln (`drawCtrlOutline()`,
      zeichnet mehrere nach INNEN gestapelte Ringe falls PX>1) - als
      LETZTES aufgerufen in `drawCtrlAcerIdle()`/`drawCtrlMacIdle()`/
      `drawCtrlMenuIdle()`/beiden Zweigen von `drawCtrlFanIdle()`/
      `ctrlFlashTile()` (deckt alle Feedback-Zustaende + Boost ab).
- [x] `labelFan`/`label_fan` in storage.h/.cpp (additiv, SD+NVS), aktueller
      Standard "Luefter Mac" (ASCII-Platzhalter bis Schritt A3). NEU:
      `fanTileLabel()` macht das Label sprachabhaengig wie von Test h)
      verlangt (EN zeigt "Mac Fan"), ABER nur solange Alex es nicht
      selbst geaendert hat (Vergleich gegen den bekannten DE-
      Standardwert) - **Stolperfalle fuer Schritt A3**: der Vergleichs-
      String `DE_DEFAULT` in `fanTileLabel()` muss dort mit auf den
      echten Umlaut umgestellt werden, sonst erkennt die Sprachumschaltung
      den (dann geaenderten) Standardwert nicht mehr.
- [x] Label-Breite automatisch pruefen/verkleinern bereits strukturell
      vorhanden (alle Controls-Labels laufen durch dieselbe
      `pickClearFont(label, r.w-6, STRIPE_H-4)`-Messung in
      `drawCtrlTileBase()`, keine Sonderbehandlung noetig).
- [x] Gebaut (Flash 72,5%, +~1KB ggue. vorher) + geflasht
      (`/dev/cu.wchusbserial31410`), automatische Mac-App-Portfreigabe
      hat wie erwartet funktioniert.

### Teil A, Schritt A1: Flightradar-Schrift ansehen (nur lesen) - ABGESCHLOSSEN
- [x] **Quellschrift/Format**: `CYD Flightradar/src/ui_font.h`, Header-
      Kommentar: "Automatisch generierter Font (DejaVu Sans Mono, 11px)
      fuer TFT_eSPI GFXFF... Deckt Basis-ASCII + Umlaute/Akzente fuer
      DE/TR/FR/ES/IT ab (U+0020-U+015F)". Format: klassisches Adafruit-
      GFXfont-Struct (`{bitmapOffset,width,height,xAdvance,xOffset,
      yOffset}` je Glyph + `{bitmap*,glyph*,first,last,yAdvance}` fuer
      den Font selbst) - EXAKT dasselbe Struct-Layout, das iBoard fuer
      `FreeSansBold9/18/24pt7b` bereits via `LOAD_GFXFF`/`setFreeFont()`
      nutzt (in `main.cpp::applyFont()`), also direkt kompatibel.
- [x] **Erzeugungs-Skript NICHT vorhanden**: im Flightradar-Projekt
      existiert kein `.py`/Shell-Skript und keine Lizenzdatei fuer
      diese Schrift (weder im Projekt noch in CLAUDE.md/README dort
      dokumentiert) - nur der eine Kommentarsatz in `ui_font.h` selbst.
      Kann also nicht 1:1 wiederverwendet werden, nur das ERGEBNIS
      (Quellschrift-Name + Struct-Format) ist bekannt. **Hinweis fuer
      Alex**: Dem Flightradar-Projekt fehlt damit selbst noch die
      Lizenzdokumentation fuer diese Schrift - das wird hier NICHT
      behoben (nur lesen erlaubt), aber als Beobachtung festgehalten,
      falls das dort mal nachgezogen werden soll.
- [x] **Lizenz gepruefte, frei nutzbar/weitergebbar**: DejaVu Fonts
      stehen unter der DejaVu-Lizenz (Weiterentwicklung der Bitstream-
      Vera-Lizenz) - freie Nutzung, Modifikation UND Weitergabe
      ausdruecklich erlaubt, auch eingebettet/kommerziell, einzige
      Einschraenkung: der Name "DejaVu" darf bei einer *wesentlich*
      veraenderten Fassung nicht unveraendert weitergefuehrt werden
      (hier irrelevant, da nur eine Teilmenge der Zeichen als Bitmap
      exportiert wird, keine Schriftdatei selbst weitergegeben wird).
      Volltext kommt in `licenses/` (Schritt A2). **Kein Stopp noetig,
      nichts bei Alex nachgefragt.**
- [x] **Schriftschnitt-Vergleich**: Flightradar nutzt "DejaVu Sans
      MONO" (schmale Laufweite, regulaeres/nicht fettes Gewicht) - passt
      NICHT zu iBoards aktueller fetter, proportionaler
      `FreeSansBold`-Anmutung. **Entscheidung**: eigene iBoard-Schrift
      aus **DejaVu Sans Bold** (fett, proportional, nicht Mono - lokal
      vorhanden ueber die matplotlib-Python-Installation, gleiche
      DejaVu-Lizenz) statt der duenneren Mono-Variante erzeugen, wie im
      Auftrag vorgesehen.
- [x] **Werkzeug**: Adafruit GFX Library's `fontconvert.c` ist das
      Standard-Werkzeug fuer genau dieses Struct-Format (hoechst-
      wahrscheinlich auch fuer Flightradars `ui_font.h` genutzt) -
      braucht freetype zum Kompilieren, das auf diesem Mac nicht fertig
      vorlag (Freetype-Quellcode zwar lokal im Espressif-Cache
      gefunden, Kompilieren ueber dessen `configure`-Skript schlug mit
      einem Build-System-Fehler fehl, nicht weiter verfolgt). Stattdessen
      **eigenes Python-Skript** (`cyd/tools/generate_ui_font.py`, siehe
      Schritt A2), das Pillow (bringt sein eigenes, bereits
      funktionierendes Freetype mit) zum Rastern jedes Zeichens nutzt
      und das Adafruit-GFXfont-Bit-Packing von Hand nachbildet - exakte
      Bit-Reihenfolge (1bpp, MSB zuerst, Zeilen ohne Auffuellung direkt
      hintereinander, jede Glyph beginnt bei einem neuen Byte) direkt
      aus `TFT_eSPI.cpp::drawChar()` (GFXFF-Zweig) abgelesen, nicht
      geraten.

**WICHTIGER ZWISCHENSTAND / GROESSENEINSCHAETZUNG** (vor Schritt A2
festgehalten, damit eine kuenftige "weiter"-Sitzung nicht neu anfangen
muss): Beim genaueren Hinsehen ist der Umfang von Schritt A3 ("ALLE
sichtbaren deutschen Texte") GROESSER als die Kopfzeile/das Dashboard/
Controls: Nur Kopfzeilen-Name, Dashboard-Werte/Streifen, Controls-
Labels und der Bootscreen-Titel laufen ueber die drei GFXFF-"Clear"-
Kandidaten (`FreeSansBold9/18/24pt7b`, ersetzt durch die neue Schrift).
**Saemtliche Menues, die Einrichtung, die Bildschirmtastatur, Hilfe-
Popups und Knopf-Beschriftungen** (`runMainMenu()`, `runSetupMenu()`,
`runFanSettingsMenu()`, `runBrightnessMenu()`, `showHelpPopup()`,
`drawNoConnectionScreen()`, `drawButton()`, alles in `setup_flow.cpp`)
laufen stattdessen ueber `useBuiltinFont()` (TFT_eSPIs eingebauten,
rein ASCII-faehigen GLCD-Font, `tft.setTextFont(1)`) - das sind 17
Aufrufstellen allein in `main.cpp`, dazu die komplette
`setup_flow.cpp` (nutzt implizit denselben Font-Zustand, da sie selbst
nie umschaltet). Fuer echte Umlaute MUESSEN all diese Stellen
zusaetzlich auf die neue GFXFF-Schrift umgestellt werden (eigene,
kleinere Groesse noetig, da Font 1 kompakter ist) - das ist ein
deutlich groesserer Eingriff als nur drei Schriftgroessen
auszutauschen, mit entsprechend mehr Mess-/Layout-Pruefarbeit
(Schritt A2 letzter Punkt) und mehr Testflaeche (Schritt A4). Wird
trotzdem wie beauftragt umgesetzt, nur hier als realistische
Grosseneinschaetzung vermerkt.

### Teil A, Schritt A2: Schrift einbauen - TEILWEISE (siehe Luecke unten)
- [x] Schriftdateien nach `cyd/src/fonts/ui_bold_font.h` (automatisch
      erzeugt, siehe `cyd/tools/generate_ui_font.py`), Lizenz nach
      `licenses/DEJAVU-FONTS-LICENSE.txt` (1:1 von der lokalen
      matplotlib-DejaVu-Kopie uebernommen, authentischer Text).
- [x] Zeichenumfang: luecklos 0x20-0xFF (TFT_eSPI/Adafruit-GFXfont
      indiziert das Glyphen-Array DIREKT ueber Codepunkt-first, OHNE
      Nachschlagetabelle - ein luecklückiger Bereich wie urspruenglich
      geplant [0x20-0x7E + einzelne Latin-1-Zeichen] haette zu
      Pufferueberlauf/Absturz gefuehrt, siehe Testlauf-Korrektur in
      `generate_ui_font.py`). Nur die tatsaechlich gebrauchten
      Codepunkte (ASCII + ÄÖÜäöüß + Gradzeichen) bekommen ein echtes
      Bitmap, der Rest leere Platzhalter-Glyphen (kein Bitmap-Speicher).
- [x] 4 Groessen erzeugt: `UiBold24`/`UiBold18`/`UiBold9` (ersetzen
      `FreeSansBold24/18/9pt7b` 1:1, Pixelgroessen empirisch auf
      identische 'M'-Hoehe 34/26/13px kalibriert - optische Groesse
      des Hauptscreens dadurch unveraendert) + `UiBoldSmall` (neu, 13px
      Quellgroesse, ersetzt den eingebauten GLCD-Font ueberall dort, wo
      `useBuiltinFont()` aufgerufen wird).
- [x] Ladeweg: identisch zum Bestehenden (`tft.setFreeFont()` via
      `applyFont()`/`CLEAR_CANDIDATES[]`), keine neue Mechanik noetig.
- [x] Flash-Auslastung: **72,5% -> 73,9%** (950.753 -> 968.253 Bytes,
      +17.500 Bytes fuer 4 komplette Schriftgroessen) - deutlich unter
      der 88%-Grenze, keine Reduktion noetig.
- [x] Hauptscreen-Werte ("100%","4900","99.9 W","--") unveraendert, da
      `CLEAR_CANDIDATES` nur die FONT-DATEN tauscht, nicht die
      Mess-/Verkleinerungs-Logik selbst (`pickFont()`/`fitAllStrings()`
      unveraendert).
- [ ] **LUECKE, bewusst offen gelassen (Alex' Entscheidung nach
      Rueckfrage)**: `setup_flow.cpp` (Ersteinrichtung, WLAN-Auswahl,
      Touch-Kalibrierung, Bildschirmtastatur) ruft `useBuiltinFont()`
      NIE auf und schaltet selbst NIE die Schrift um - haengt also am
      TFT_eSPI-eigenen Standardfont (Font 1, ASCII), unabhaengig von
      `useBuiltinFont()`s Swap in `main.cpp`. Braucht eine EIGENE
      Schrift-Anbindung (+ eigene Pruefung auf setCursor()+print()-
      Stellen dort) - noch nicht angefasst.

### Teil A, Schritt A3: Texte umstellen - TEILWEISE (siehe Luecke)
- [x] **Kritischer Fund vor der Textumstellung**: `setCursor()+print()`
      verankert bei der neuen GFXFF-Schrift an der GRUNDLINIE statt wie
      beim alten Font 1 an der Oberkante (derselbe Stolperstein wie im
      Flightradar-Projekt dokumentiert) - `drawString()+setTextDatum()`
      ist davon NICHT betroffen. Alle sechs betroffenen Live-Stellen in
      `main.cpp` gefunden und auf `drawString()+TL_DATUM` umgestellt:
      `drawNoConnectionScreen()` (4 Zeilen), `runMainMenu()`-Header (2
      Zeilen), `showHelpPopup()`s Textschleife, die "Keine SD-Karte"-
      Meldung. Zwei weitere `setCursor()`-Fundstellen sind TOTER CODE
      (`Config::USE_OLD_HEADER==false`, nie erreicht) - unangetastet
      gelassen. `drawButton()` nutzte bereits vorher `drawString()`,
      war also schon sicher (betrifft ALLE ueber `drawButton()`
      gezeichneten Knopf-Beschriftungen im ganzen Programm).
- [x] Texte umgestellt (nur dort, wo die Anzeige bereits auf der neuen
      Schrift laeuft - siehe Luecke unten fuer den Rest), alt -> neu:
      - `MENU_BUTTON`: "Menue" -> "Menü"
      - `MENU_HEADER`: "Eiswolfs iBoard Menue" -> "Eiswolfs iBoard Menü"
      - `MENU_FAN_BUTTON`: "Luefter" -> "Lüfter"
      - `MENU_BACK`: "Zurueck" -> "Zurück"
      - `SETUP_WIFI_ONLY`: "Nur WLAN aendern" -> "Nur WLAN ändern"
      - `SETUP_WIFI_HELP`: "...unveraendert erhalten." -> "...unverändert erhalten." (+ "Aendert" -> "Ändert")
      - `SETUP_CAL_HELP`: "...unveraendert." -> "...unverändert."
      - `SETUP_WOL_HELP`: "...faehrt ihn runter..." -> "...fährt ihn runter..."
      - `SETUP_RESYNC`: "Zaehler neu abgleichen" -> "Zähler neu abgleichen"
      - `SETUP_RESYNC_HELP`: "noetig"->"nötig", "Luefter-Boost"->"Lüfter-Boost", "muessen"->"müssen", "dafuer"->"dafür", "Menue"->"Menü"
      - `labelFan`-Standardwert (Storage, nicht i18n): "Luefter Mac" -> "Lüfter Mac" (Schritt-A0-ASCII-Platzhalter abgeloest; `fanTileLabel()` erkennt beide alten/neuen Default-Werte als "noch nicht von Alex angepasst", damit ein Geraet mit dem alten Platzhalter in `config.json` die Sprachumschaltung trotzdem korrekt macht)
      - EN-Strings unveraendert gelassen (kein Umlaut-Bedarf dort)
- [x] Grossschreibung (`langCode.toUpperCase()` fuer "DE"/"EN") hat
      keine Umlaut-Beruehrungspunkte - ASCII-Kuerzel, nicht betroffen,
      keine Sonderfunktion noetig.
- [ ] **LUECKE (hoert mit der A2-Luecke zusammen)**: `CALIB_*`/`KB_*`/
      `WIFI_*`-Texte (Kalibrierung, Bildschirmtastatur, WLAN-Scan/
      -Verbindung) sind bewusst NICHT umgestellt, weil sie exklusiv in
      `setup_flow.cpp` gerendert werden (noch auf Font 1/ASCII, siehe
      A2-Luecke) - ein Umstellen der TEXTE allein haette dort nur
      fehlende/falsche Glyphen gezeigt. Bildschirmtastatur hat
      dementsprechend auch noch KEINE Umlaut-/ß-Sondertaste.

### Teil A, Schritt A4: Vorschau, Bauen, Flashen, Tests a-j - TEILWEISE
- [x] Gebaut + geflasht (`/dev/cu.wchusbserial31410`), automatische
      Mac-App-Portfreigabe hat funktioniert.
- [ ] Vorschau-PNGs fuer diesen Zwischenstand NICHT erzeugt (Zeitdruck,
      Auftrag pausiert fuer den neuen Controls-Bedienung-Auftrag, siehe
      unten) - beim Fortsetzen nachholen.
- [ ] Interaktive Tests a-j mit Alex noch NICHT durchgefuehrt - verschoben,
      Alex hat bestaetigt, dass der Controls-Bedienung-Auftrag zuerst
      drankommt.

**Status, per Alex' ausdruecklicher Entscheidung (Rueckfrage gestellt
und beantwortet): Umlaute-Auftrag wird HIER pausiert, nicht
abgebrochen.** Der aktuell geflashte Stand ist vollstaendig
durchgebaut und in sich konsistent (keine halb umgestellten Screens -
die noch-ASCII-Screens [Einrichtung/Tastatur] sehen exakt so aus wie
vor diesem Auftrag, nichts ist dort kaputt, nur noch nicht umgestellt).
Fortsetzen mit "weiter": zuerst Schritt A2/A3-Luecke (setup_flow.cpp
Schrift-Anbindung + Tastatur-Umlauttaste), dann A4 (Vorschau+Tests),
dann Teil B (Mac-App-Menue).

### Teil B: Mac-App-Menue (NOCH NICHT BEGONNEN, nach Teil A)
- [ ] Alle deutschen Mac-App-Texte auf echte Umlaute umgestellt
- [ ] Info.plist-Texte UTF-8 geprueft
- [ ] Version 1.1.3, DMG neu gebaut, Token-/Namens-Suche wiederholt,
      installierte App aktualisiert, Autostart/Einstellungen/
      Hilfsdienst geprueft

### Dokumentation
- [x] THIRD-PARTY-LICENSES.md + licenses/DEJAVU-FONTS-LICENSE.txt ergaenzt
- [ ] README-Hinweis auf neue Schrift (noch offen, mit dem Rest von A2/A3/A4/Teil B nachholen)
- [ ] docs/DEVELOPMENT.md (zweisprachig) auf "Lüfter Mac"/`label_fan`
      aktualisiert, falls "Luefter"/"Menue" dort vorkommt (noch offen)
- [x] CLAUDE.md fortlaufend aktualisiert (siehe Eintraege oben)
- [x] Flightradar/Toolbox/Solarguardian bislang nur-gelesen bestaetigt
      (git status aller drei unberuehrt durch diese Sitzung)

**Pausiert nach Schritt A0-A3 (teilweise) - siehe "Status"-Absatz
weiter oben. Mit "weiter" an der dort beschriebenen Luecke
fortsetzen.**

## Auftrag Controls-Bedienung: Fortschritt

Neuer Auftrag (baut auf dem oben pausierten Umlaute-Stand auf, OHNE
Umrandung/Label/Schrift/Texte zu aendern): 1) Zurueck-Button in der
Controls-Kopfzeile, 2) strengere Antippen-Erkennung (kein
versehentliches Ausloesen durch missglueckte Wischgesten - Alex hat
dadurch bereits 3x ungewollt den Mac schlafen geschickt/Luefter-Boost
gestartet), 3) Wischen leichter machen. Nur `cyd/` betroffen, NICHT
committen/taggen/pushen.

### Schritt 1: Zurueck-Button
- [ ] Weisser Pfeil "‹" aus 2 Linien gezeichnet (nicht als Schriftzeichen),
      Umrandung mit denselben Konstanten wie die Kacheln
- [ ] Sichtbar ca. 44x28px bei x=8,y=6; Tipp-Bereich ca. 80x38px
      (0..80, 0..38), beruehrt Kachelreihe (y=40) nicht, ueberlappt
      "Controls"-Titel nicht
- [ ] Kurzes Antippen = wie Wischen nach rechts (zurueck zu Main)
- [ ] Rueckmeldung: 150ms weiss gefuellt, Pfeil schwarz

### Schritt 2: Strengere Kachel-/Button-Ausloesung (Hauptfix)
- [ ] Ursache geprueft (nicht geraten) - TouchCalib::pollGesture() + Kachel-Code gelesen
- [ ] `TAP_MAX_MOVE_PX`-Konstante (~10px, groesste Bewegung waehrend der GANZEN Beruehrung, nicht nur Start/Ende)
- [ ] Aufsetzpunkt = Mittelwert der ersten 2-3 gueltigen Messwerte
- [ ] Prell-Entprellung (150ms) bleibt, weit entfernter Neuaufsatz = neue Beruehrung
- [ ] Dauergrenzen fuer Mac/Luefter/Menue/Zurueck (~40ms-1,5s)
- [ ] Acer-Halten: Bewegung bis zur Haltezeit (0,5s) unter TAP_MAX_MOVE_PX

### Schritt 3: Wischen leichter
- [ ] `SWIPE_MIN_DX_PX=50`, `SWIPE_MAX_DY_PX=60`, zusaetzlich dx >= 1,5*dy, `SWIPE_MIN_HOLD_MS=80` bleibt
- [ ] Rechts-Wisch auf Controls darf ueberall beginnen (auch auf Kachel)
- [ ] Als Wisch erkannte Beruehrung loest nie eine Kachel aus

### Schritt 4: Testfassung ohne echte Befehle + Messung
- [ ] `TOUCH_TEST_MODE`-Schalter (Normalbetrieb aus): Rueckmeldefarben wie sonst, aber KEIN Befehl gesendet, stattdessen Serial-Log pro Beruehrung
- [ ] Mac-App-Port freigeben (Menue/"USB freigeben"), Serial-Log selbst mitlesen
- [ ] Tests a-f mit Alex, Zielwerte dokumentieren, bei Abweichung Konstanten nachjustieren
- [ ] Normalfassung (TOUCH_TEST_MODE aus) gebaut+geflasht, App neu gestartet
- [ ] Je 1x jeder echte Befehl mit Alex geprueft, Luefter am Ende Automatik
- [ ] Notausgang + Bootscreen-Skip unveraendert funktionsfaehig

### Dokumentation
- [ ] CLAUDE.md: Ursache, neue Konstanten, Zurueck-Button-Aufbau, Testfassung+Messergebnisse, Stolperfallen
- [ ] README/docs (DE/EN): Zurueck-Pfeil + Wischen beschrieben
- [ ] Flightradar/Toolbox/Solarguardian nur gelesen bestaetigt

### Nachtrag: drei Korrekturen anhand Alex' Live-Feedback am echten Geraet

Bevor Schritt 1 begonnen wurde, hat Alex direkt am Hauptscreen-Foto drei
Bugs aus dem VORHERIGEN Umlaute-Auftrag gemeldet - hier sofort behoben,
da sie denselben Code betreffen:

- [x] **"Zahlen zu klein"**: Ursache gefunden (nicht geraten) - die
      urspruenglich verwendete Quellschrift DejaVu Sans Bold ist bei
      gleicher 'M'-Hoehe deutlich BREITER als FreeSansBold. "100%" war
      beim alten 18pt-Ersatz (142px bzw. 107px bei den beiden
      kalibrierten Groessen) breiter als die Kachel (97px), wodurch
      `fitAllStrings()` staendig auf die kleinste Kandidatengroesse
      zurueckfiel. **Fix**: Quellschrift gewechselt auf **Liberation
      Sans Bold** (freier Arial-Ersatz, SIL Open Font License 1.1,
      schmaler als DejaVu) - Groessen jetzt anhand der TATSAECHLICHEN
      Breite der kritischen Strings ("100%"/"99.9 W"/"110C"/"4900")
      kalibriert (mit ca. 5px Sicherheitsabstand), nicht mehr nur
      anhand der 'M'-Hoehe. `licenses/DEJAVU-FONTS-LICENSE.txt` entfernt,
      `licenses/LIBERATION-FONTS-LICENSE.txt` ergaenzt,
      THIRD-PARTY-LICENSES.md aktualisiert.
- [x] **"USB oben rechts" zeigte Buchstabenmatsch**: Ursache -
      `drawHeaderSourceNew()` nutzte `useBuiltinFont()`, das seit diesem
      Auftrag die deutlich hoehere neue Schrift anwendet - der Loesch-
      bereich dort (`headerSourceRect`, 10px hoch) war aber noch auf die
      winzige ALTE eingebaute Schrift zugeschnitten, die neue Schrift
      ragte heraus und wurde bei jedem Update nur teilweise geloescht
      ("Geisterpixel" mehrerer Frames uebereinander). **Fix**: "USB"/
      "WLAN" brauchen keine Umlaute (reines ASCII) - bleiben deshalb
      bewusst auf dem echten, winzigen `tft.setTextFont(1)` statt
      `useBuiltinFont()` (unveraenderte `headerSourceRect`-Groesse).
- [x] **"Luefter Mac" statt "Lüfter Mac" auf Alex' Geraet**: Ursache -
      ihr Geraet hatte den ASCII-Platzhalter aus Schritt A0 bereits in
      `config.json` gespeichert, die additive Regel ("bestehende Werte
      nie ueberschreiben") verhinderte die automatische Umstellung auf
      den neuen Standardwert mit echtem Umlaut. **Fix**: gezielte,
      einmalige Migration in `storage.cpp::migrateLabelFan()` - ersetzt
      beim Laden AUSSCHLIESSLICH exakt den alten Platzhalter-Text
      "Luefter Mac" durch "Lüfter Mac". Jeder andere (selbst
      eingetragene) Name bleibt unberuehrt - keine Verletzung der
      additiven Regel, da es sich nachweislich nicht um eine eigene
      Eingabe handelt, sondern um den damaligen Code-Standardwert.
- [x] Gebaut (Flash 73,8%) + geflasht (`/dev/cu.wchusbserial31410`).
      "Menü" hat laut Alex schon vorher korrekt funktioniert (keine
      Aenderung dort noetig).

**Noch nichts umgesetzt fuer Schritt 1-4 selbst - startet jetzt mit
Schritt 2 (Code lesen/Ursache pruefen). Hinweis: `TouchCalib::
pollGesture()` wurde dabei VORAB schon umgebaut (TAP_MAX_MOVE_PX ueber
die gesamte Beruehrung, Aufsetzpunkt-Mittelwert aus bis zu 3 Messwerten,
Dauergrenzen, lockerere Wisch-Werte inkl. Richtungsverhaeltnis) - siehe
Config-Konstanten weiter oben in dieser Datei (bereits ergaenzt) und
`touch_calib.cpp`.**

### Schritt 1+2+3: umgesetzt und geflasht
- [x] Zurueck-Button: `drawCtrlBackButton()` (Pfeil aus 2 Linien,
      `drawWideLine()`, Staerke 3px, dieselbe Umrandung wie die
      Kacheln), sichtbar 44x28px bei (8,6), Tipp-Bereich `ctrlBackTapRect`
      80x38px (0,0)-(80,38) - beruehrt weder Kachelreihe (y=40) noch
      "Controls"-Titel (Titel beginnt horizontal erst ab ca. x=110).
      Kurzes Antippen = wie Wischen nach rechts (gleicher Code-Pfad),
      150ms weisse Rueckmeldung (blockierendes `delay(150)` bewusst
      statt eines eigenen Timer-Zustands, da der Screen direkt danach
      ohnehin komplett neu aufgebaut wird).
- [x] `TouchCalib::pollGesture()` komplett neu: `TAP_MAX_MOVE_PX`
      (Config, 10px) gilt fuer die GROESSTE Bewegung waehrend der
      GANZEN Beruehrung (eigene `maxMove`-Verfolgung bei jedem
      Messwert, nicht nur Start/Ende) - vorher wurde NUR die
      senkrechte Abweichung verfolgt, eine missglueckte, zu kurze
      Wischgeste fiel dadurch IMMER automatisch auf "Antippen" zurueck
      (das war die tatsaechliche Ursache fuer Alex' ungewollte
      Ausloesungen - nachgeprueft im Code, nicht geraten: die alte
      Funktion kannte gar keinen dritten Zustand "verworfen"). Aufsetz-
      punkt = Mittelwert der ersten bis zu 3 Messwerte. Dauergrenzen
      `TAP_MIN_HOLD_MS`(40)/`TAP_MAX_HOLD_MS`(1500) gelten fuer JEDES
      Antippen (gilt automatisch fuer alle Kacheln + Zurueck-Button, da
      alle denselben `pollGesture()`-Aufruf nutzen).
- [x] Acer-Halten: eigene `maxMove`-Verfolgung waehrend der Haltezeit
      ergaenzt (vorher nur Positions-Check beim Erreichen der 0,5s,
      keine durchgehende Bewegungsgrenze) - bewegt sich der Finger
      vorher weiter als `TAP_MAX_MOVE_PX`, verfaellt das Halten.
- [x] Wisch-Konstanten gelockert: `SWIPE_MIN_DX_PX=50` (vorher 80),
      `SWIPE_MAX_DY_PX=60` (vorher 40), neu `SWIPE_MIN_HORIZONTAL_RATIO=1.5`
      (waagerecht muss mind. das 1,5-fache der groessten senkrechten
      Abweichung sein). "Wisch rechts darf ueberall auf Controls
      beginnen" war schon strukturell erfuellt (Wisch-Erkennung kennt
      keine Kachel-Geometrie) - keine Aenderung noetig.
- [x] Bekannte, bewusst nicht behobene Randschaerfe (klein, dokumentiert
      statt repariert): `pollGesture()` prueft Schritt 2a ("Aufsetzen
      UND Loslassen in DERSELBEN Kachel") nur indirekt ueber die
      TAP_MAX_MOVE_PX-Grenze (10px) - bei einem Tipp, der GENAU auf
      einer Kachel-Kante beginnt (Kachelabstand nur 8px), koennte der
      Loslasspunkt theoretisch in der Nachbarkachel liegen. Sehr
      seltener Grenzfall, kein durch Alex gemeldetes Problem - bei
      Bedarf spaeter nachruestbar (zweiter exponierter Punkt aus
      `pollGesture()` + eigener Gleichheitscheck im Aufrufer).
- [x] Gebaut (Flash 73,8%) + geflasht (`/dev/cu.wchusbserial31410`).

**Alex' Entscheidung nach Rueckfrage: KEINE formale TOUCH_TEST_MODE-
Messung (Schritt 4) - stattdessen direkt die normale Fassung live
ausprobieren. Schritt 4 (Testfassung+Messwerte a-f) bleibt damit
bewusst ausgelassen, nicht vergessen. Noch OFFEN: Dokumentation
(README/docs-Bedienbeschreibung), finale Live-Bestaetigung aller
echten Befehle mit Alex, Luefter-Automatik-Endzustand pruefen.**

## Auftrag Ueberschriften und WLAN-Zurueck: Fortschritt

Neuer Auftrag: abgeschnittene/ueberdeckte Texte auf vielen Bildschirmen
seit der neuen Schrift beheben, WLAN-Liste bekommt einen Zurueck-Pfeil.
Nur `cyd/` betroffen (inkl. `tools`/Doku), NICHT committen/taggen/pushen.
Hauptscreen/Controls/Bootscreen bleiben optisch unveraendert.

### Ursache (geprueft, nicht geraten)

Hypothese (a) bestaetigt: die neue, umlautfaehige Schrift (`UiBoldSmall`,
siehe oben "Schriften") hat andere Massverhaeltnisse als die beiden
alten ASCII-Schriften, auf die Layouts/Y-Positionen/Abstaende noch
abgestimmt waren:
- **"Keine Verbindung zum Mac"**: Die zweizeilige rote Ueberschrift
  wurde per `tft.setTextSize(2)` auf der bereits 24px hohen
  `UiBoldSmall` hochskaliert (im Code gefunden, nicht vermutet) - das
  ist genau die im Auftrag verbotene Skalierung (Hypothese b) und
  erklaerte sowohl das Ueberlaufen (effektiv ~48px Zeilenhoehe bei
  festen Y=40/64) als auch die "gezackten" Kanten (Pixel-Verdopplung
  einer Bitmap-Schrift sieht grob aus, anders als normales GFXFF-
  Rendering).
- **Hauptmenue**: Titel (`drawString()+TL_DATUM`, Box y=4..28) und Label
  "Verbindung:" (eigener `drawString()`-Aufruf bei y=20, Box y=20..44)
  ueberlappten sich tatsaechlich - beide nutzen `UiBoldSmall` (24px
  Zeilenhoehe), die Y-Werte stammten noch von der alten, viel
  kompakteren eingebauten Schrift. Gleiches bei "Schrift:" (y=78..102)
  gegen die Tasten darunter (y=90..130).
- **WLAN-Liste/Tastatur/Kalibrierung/Verbindungstest** (`setup_flow.cpp`):
  dieses ganze Modul haengte seit dem Umlaute-Auftrag noch komplett am
  alten, eingebauten ASCII-Font (nie umgestellt, siehe "LUECKE" oben) -
  daher keine Umlaute dort UND mehrere ungeprueft breite Texte
  (`setCursor()+print()` ohne Breitenmessung fuer den Tastatur-
  Hinweistext, `drawButton()` ohne jede Breitenpruefung fuer die
  WLAN-Zeilen "Name + Signalstaerke" als EIN zentrierter String).
- Hypothese (c) "Glaettung gegen falsche Hintergrundfarbe" **nicht**
  bestaetigt als eigenstaendige Ursache: alle betroffenen Bildschirme
  nutzen `tft.fillScreen(TFT_BLACK)` (reines Schwarz), es gibt KEIN
  "dunkelblau" im Code - das im Auftrag beschriebene "wirkt dunkelblau"
  ist mit hoher Wahrscheinlichkeit ein Foto-/Kamera-Weissabgleich-
  Artefakt eines TFT-Panels mit schwarzem Hintergrund, kein
  Render-Unterschied (alle Bildschirme in diesem Auftrag: reines
  `#000000`, identisch zu Hauptscreen/Controls, siehe Hintergrundfarben-
  Tabelle unten).
- Hypothese (d)/(e): kein spaeter gezeichnetes Feld ueberdeckt einen
  vorher gezeichneten Text absichtlich falsch - die Ueberdeckung kam
  ausschliesslich aus zu knapp bemessenen Y-Abstaenden fuer die neue
  Schrifthoehe (siehe oben).
- Punkt 5 ("ausgewaehlte Tasten wirken duenner"): im Code identisch
  (dieselbe Schrift/Groesse fuer gefuellte UND ungefuellte Tasten,
  `drawButton()` nutzt in beiden Zweigen denselben `UiText::draw()`-
  Aufruf) - sehr wahrscheinlich die bekannte optische Taeuschung
  "dunkle Schrift auf hellem Grund wirkt duenner als helle Schrift auf
  dunklem Grund" (kein Code-Bug, nicht behoben, da nichts zu beheben
  ist - hier dokumentiert statt stillschweigend ignoriert).

### Zentrale Textfunktion (Schritt 2): `src/ui_text.h`/`.cpp`

Neues, gemeinsames Modul (von `main.cpp` UND `setup_flow.cpp` genutzt -
bewusst NICHT dupliziert wie das kleine Rect/drawButton-Muster, da
sonst dieselben Schriftobjekte doppelt im Flash laegen):
- `UiText::draw(tft, box, text, fg, bg, align, paddingX)`: loescht NUR
  die eigene Box mit `bg` (gleiche Farbe fuer Text-Hintergrund -> keine
  Kantenglaettung gegen eine falsche Farbe), probiert automatisch die
  groesstmoegliche ERZEUGTE Schriftgroesse (`UiBoldSmall` 24px ->
  `UiBoldS2` 20px -> `UiBoldS3` 15px, zwei NEUE Schrumpf-Stufen, siehe
  unten), kuerzt mit "..." falls selbst `UiBoldS3` nicht passt. Nutzt
  TFT_eSPIs eigene `ML/MC/MR_DATUM`-Vertikalzentrierung (laut
  Flightradar-Projekt bereits korrekt fuer GFXFF-Schriften, unabhaengig
  vom Baseline-Anker - nachgeprueft, nicht blind uebernommen) statt
  eigener Ascent/Descent-Mathematik.
- `UiText::drawHeader(tft, title, showBack, backFlashed)`: einheitliche
  34px-Kopfzeile (`HEADER_H`), Titel in der Groesse, die Controls
  TATSAECHLICH nutzt (`UiBold9`, nicht `UiBold18` - siehe Code-
  Kommentar in `ui_text.cpp`: `pickClearFont()` mit `maxH=20` lehnt bei
  Controls alle drei Kandidaten wegen der Hoehe ab und faellt immer auf
  die letzte, kleinste Kandidatin `UiBold9` zurueck, nachgeprueft ueber
  `TFT_eSPI.cpp::fontHeight()` - `maxH` dort war schon immer zu knapp
  fuer `UiBold18`). `CONTENT_Y`=38 (Konstante) ist die Y-Position, ab
  der restlicher Bildschirminhalt beginnen soll. `showBack=true`
  zeichnet denselben Zurueck-Pfeil (zwei Linien, 44x28px sichtbar,
  Tipp-Bereich `UiText::BACK_TAP_RECT` 80x38px links) wie Controls -
  eine EIGENE Funktion, Controls' eigener Code
  (`drawCtrlBackButton()`/`ctrlBackTapRect`) ist UNVERAENDERT, damit
  Controls optisch garantiert gleich bleibt.
- Zwei neue Schriftgroessen `UiBoldS2`(17px Quellgroesse)/`UiBoldS3`
  (13px) in `tools/generate_ui_font.py`/`src/fonts/ui_bold_font.h`
  ergaenzt, ausschliesslich fuer die Schrumpf-Stufen von
  `UiText::draw()`. `UiBoldS3` (Gesamthoehe 15px) ist die kleinste
  erzeugte Groesse - klar ueber der geforderten 11px-Untergrenze.
  Flash-Kosten: ca. +2,3KB fuer beide zusammen (unwesentlich).
- `drawButton()` (main.cpp UND setup_flow.cpp, beide Module) laeuft
  jetzt ueber `UiText::draw()` statt einer festen, ungeprueften
  `useBuiltinFont()`-Groesse - jede Tastenbeschriftung bekommt dadurch
  automatisch Breitenmessung+Schrumpf/Kuerzung.
- `LAYOUT_DEBUG` (siehe Schritt 6 unten) ist direkt in `UiText::draw()`/
  `drawHeader()` eingebaut, kein separater Mechanismus.

### Behobene Bildschirme (Schritt 3)

- **"Keine Verbindung zum Mac"**: `setTextSize(2)` entfernt, Titel ueber
  zwei `UiText::draw()`-Boxen (je 26px hoch, 8px Rand links/rechts,
  rot, zentriert - waehlt automatisch bis zu `UiBoldSmall`=24px, genau
  die im Auftrag genannte Obergrenze). WLAN/IP/Status jetzt in eigenen
  17px-Boxen (erzwingt `UiBoldS3`, ~15px - nahe an den geforderten
  "ca. 14px"), nichts mehr angeschnitten. EN-Text `NO_CONNECTION_LINE2`
  von "to the Mac" auf "to Mac" korrigiert (Auftragswortlaut).
- **Hauptmenue**: einheitliche 34px-Kopfzeile, beide Labels
  ("Verbindung:"/"Schrift:") jetzt in eigenen 15px-Boxen mit
  garantiertem Abstand zu Titel/Tasten (keine Ueberlappung mehr
  moeglich, da jede Box ihre eigene Flaeche loescht). Tastenhoehen
  36px (>= geforderte 34px), alle Funktionen/Reihenfolge unveraendert.
- **WLAN-Liste**: Kopfzeile mit Titel "WLAN waehlen"/"Choose WiFi"
  (34px, Pfeil links wenn `allowBack`), Zeilen jetzt 28px mit Name
  LINKS (eigene Box, kuerzt mit "..." statt zu ueberlaufen) und
  Signalstaerke RECHTS in fester 64px-Spalte (vorher: ein einziger
  zentrierter String "Name  -82dBm", konnte bei langem Namen rechts
  rauslaufen). 34+6*28+34=236px, passt in 240px Hoehe. Bodenleiste:
  "Neu suchen"/"Manuell eingeben" als zwei gleich breite, eigene
  Tasten (vorher ebenfalls ein nicht breitengeprueftes `drawButton()`
  ueber die gesamte Zeile).
- **Tastatur**: Hinweistext ueber dem Feld + Eingabefeld-Inhalt laufen
  jetzt ueber `UiText::draw()` (kuerzt automatisch statt abzuschneiden).
  Taste "Zurueck" zeigt jetzt "Zurück" (i18n-Korrektur, siehe Schritt 5).
  Neue Umlaut-Ebene: Reihe 1 (10 Plaetze) zeigt bei aktivem "#+-"/"123"-
  Umschalter (`symbols==true`) jetzt ä ö ü Ä Ö Ü ß (7 Tasten + 3 leere
  Plaetze) statt wie vorher unveraendert dieselben Buchstaben wie sonst
  (die Reihe reagierte auf den Umschalter bisher ueberhaupt nicht -
  geprueft im Code, nicht geraten). UTF-8-Bytes landen unveraendert in
  `String text` und werden so an `WiFi.begin()` uebergeben - Arduino
  `String`/`WiFi.begin()` behandeln das als reine Byte-Sequenz, keine
  eigene Kodierung noetig.
- **Kalibrierung/Sprachauswahl/Verbindungstest** (`setup_flow.cpp`):
  komplett von `setCursor()+print()`/ungeprueftem `drawString()` auf
  `UiText::draw()` umgestellt - schliesst zugleich die Luecke aus dem
  Umlaute-Auftrag (dieses Modul haengte bis hierher noch am alten,
  reinen ASCII-Font).
- **Einrichtung-Untermenue/Luefter-Boost-Menue/Helligkeitsregler/
  Hilfe-Popup** (main.cpp): Titel jeweils auf `UiText::drawHeader()`
  umgestellt (einheitliche 34px-Kopfzeile, Schritt 2). Luefter-Boost-
  Menue: erste Tastenreihe von y=30 auf `UiText::CONTENT_Y`=38
  verschoben (vorher 4px Ueberlappung mit der neuen Kopfzeile moeglich).
  Hilfe-Popup-Textzeilen laufen jetzt ebenfalls durch `UiText::draw()`
  (Sicherheitsnetz gegen zu lange Zeilen, auch wenn die bisherigen
  manuellen "\n"-Umbrueche schon passten).

### Zurueck-Pfeil WLAN-Liste (Schritt 4)

- Sichtbar NUR, wenn `allowBack=true`: das ist der Fall bei
  `SetupFlow::runWifiOnly()` (aufgerufen ueber Menue > Einrichtung >
  "Nur WLAN aendern" - eine bereits gueltige Einrichtung existiert).
  Antippen bricht sofort ab (`runWifiFlow()` gibt `false` zurueck,
  `cfg` wird dabei an KEINER Stelle veraendert - `cfg.wifiSsid`/
  `wifiPassword` werden nur bei erfolgreichem Verbindungstest gesetzt,
  der bei einem Abbruch nie erreicht wird) - die bestehenden WLAN-Daten
  bleiben dadurch nachweislich unangetastet, nicht nur "wahrscheinlich".
  `runWifiOnly()` ruft danach `Storage::save()` GAR NICHT auf, wenn
  `runWifiFlow()` `false` meldet (frueher Return).
- Bei `SetupFlow::runFullSetup()` (allererster Start ohne gueltige
  Einrichtung) ist `allowBack` bewusst `false` - nach der Kalibrierung
  gibt es keinen sinnvollen Rueckweg mehr (ein "Kalibrierung rueckgaengig
  machen"-Fluss existiert im Code nicht und wurde hier nicht neu
  gebaut) - der Pfeil erscheint dort deshalb gar nicht, exakt wie im
  Auftrag als Fallback vorgesehen ("sonst wird der Pfeil nicht
  angezeigt").
- Empfang (UDP/USB) und eine ggf. bestehende Mac-Verbindung sind davon
  nie betroffen - `runWifiOnly()` wird aus `runSetupMenu()` aufgerufen,
  das selbst aus dem laufenden Hauptmenue heraus gestartet wird, kein
  Neustart beteiligt.
- Andere Einrichtungsseiten auf fehlenden Rueckweg geprueft: manuelle
  SSID-Eingabe und Verbindungstest-Fehler haben bereits einen
  funktionierenden Rueckweg (Tastatur-"Zurueck" bricht zurueck zur
  Liste, ein fehlgeschlagener Verbindungstest fuehrt zurueck zur
  Passwort-Eingabe fuer dieselbe SSID) - **nicht ergaenzt, da schon
  vorhanden**. `runCalibrationOnly()` (Menue > Einrichtung > "Touch neu
  kalibrieren") hat KEINEN Zurueck-Pfeil - absichtlich nicht ergaenzt
  (ausserhalb des Auftragsumfangs "WLAN-Liste", haette eine groessere
  eigene Kalibrierungsablauf-Aenderung gebraucht) - **an Alex
  zurueckgemeldet, nicht stillschweigend ausgelassen**.

### Schritt 5: weitere Umlaute

Komplette Durchsicht aller DE-UI-Strings (i18n.cpp) auf `ae/oe/ue/ss`-
Reste (Code-Kommentare bleiben bewusst ASCII, siehe "Code-Stil"-
Konvention der Schwesterprojekte) - fuenf gefunden und korrigiert:
`CALIB_TOUCH_TEST_POINT` ("Testpunkt beruehren" -> "berühren"),
`KB_BACK`/`WIFI_BACK_TO_INPUT` ("Zurueck..." -> "Zurück..."),
`WIFI_CHOOSE` ("WLAN waehlen" -> "WLAN wählen"), `FAN_BOOST_TITLE`
("Luefter-Boost" -> "Lüfter-Boost"). `storage.cpp`/`main.cpp`s
`DE_DEFAULT_OLD_ASCII = "Luefter Mac"` bleibt ABSICHTLICH ASCII (dient
der Migrations-Erkennung alter Geraete, siehe Umlaute-Auftrag oben) -
kein Fund, sondern bewusst unveraendert.

### Schritt 6: Pruefung ohne Alex

- `Config::LAYOUT_DEBUG`/`Config::SCREEN_TOUR` als auskommentierte
  `#define`-Zeilen in `config.h` (Normalbetrieb: beide AUS).
  `LAYOUT_DEBUG` ist in `UiText::draw()`/`drawHeader()` eingebaut
  (Bildschirmrand/eigene Box/Ueberlappung mit anderen Boxen derselben
  Seite/Schrift unter 11px, `UiText::beginScreen()` leert die Liste pro
  Bildschirm) - bisher nur auf den vier am staerksten betroffenen
  Bildschirmen verdrahtet (`beginScreen()`-Aufruf ergaenzt):
  "KeineVerbindung", "Hauptmenue", "WlanListe", "Tastatur". Die
  restlichen Bildschirme (Einrichtung-Untermenue, Luefter-Boost-Menue,
  Helligkeit, Hilfe-Popup, Kalibrierung, Sprachauswahl,
  Verbindungstest) nutzen zwar bereits `UiText::draw()`/`drawHeader()`
  (profitieren also vom Schrumpf-/Kuerz-Schutz), sind aber NICHT in die
  `LAYOUT_DEBUG`-Ueberlappungspruefung eingetragen - aus Zeitgruenden
  nicht nachgeholt, **ehrlich als Luecke vermerkt statt verschwiegen**.
- `SCREEN_TOUR` (`main.cpp::runScreenTour()`) zeigt automatisch NUR
  "Keine Verbindung zum Mac" (DE dann EN, WLAN-Name durch den
  Extremtext "Müller-Straße-Äußerst 5GHz" ersetzt, 4s je Sprache,
  Name im Serial-Log) - **ehrliche Einschraenkung**: Hauptmenue/WLAN-
  Liste/Tastatur/Kalibrierung sind als BLOCKIERENDE Eingabeschleifen
  gebaut (warten auf echten Touch), ein automatischer Durchlauf dieser
  Seiten haette eine groessere Umstrukturierung gebraucht (Render von
  Eingabeschleife trennen) und wurde hier NICHT umgesetzt - diese
  Seiten werden stattdessen manuell mit Alex getestet (Schritt 7).
- **Live-Selbsttest NICHT moeglich**: mit beiden Flags AN gebaut und
  geflasht, aber die Mac-App reconnectet den seriellen Port innerhalb
  von ca. 1s von selbst wieder (automatische Port-Erkennung alle 5s,
  siehe "USB-Protokoll" oben) und haelt ihn dann exklusiv - ein
  eigenstaendiges mitlesendes Skript kommt ohne Klick auf "USB
  freigeben" im Mac-App-Menue nicht mehr dazwischen. Der Build mit
  beiden Flags kompilierte fehlerfrei; eine tatsaechliche Pruefung der
  Serial-Ausgabe (0 Verstoesse vorher/nachher) steht noch aus. Beide
  Flags sind im ausgelieferten, geflashten Stand wieder AUS.

### Schritt 7: Gebaut, geflasht

- `pio run` fehlerfrei (Flash 75,3%, RAM 14,7%). Geflasht auf
  `/dev/cu.wchusbserial31410` (automatische Mac-App-Portfreigabe hat
  wie erwartet funktioniert). Verbindung kam innerhalb von **ca. 1
  Sekunde** von selbst wieder (gemessen: `lsof` zeigte die Mac-App
  schon 1s nach Upload-Ende wieder aktiv Daten empfangend).
  `config.json` auf der SD-Karte nicht angefasst (kein Storage-Format
  geaendert).
- Flightradar/Solar-Guardian-CYD per `git status` als unveraendert
  bestaetigt (beide sauber). `eiswolfs-toolbox` zeigt zwar
  uncommittete Aenderungen/neue Dateien (Hacker-News-/Mondphasen-
  Bildschirme) - das ist NACHWEISLICH NICHT diese Sitzung (in dieser
  Sitzung wurde `eiswolfs-toolbox` kein einziges Mal geoeffnet/
  gelesen/geschrieben), sondern bereits bestehender, unversionierter
  Arbeitsstand aus anderer Arbeit - nur zur Vollstaendigkeit vermerkt.
- **Interaktive Live-Tests mit Alex noch OFFEN** (Schritt 7 selbst):
  Menue/Controls/Hauptscreen/"Keine Verbindung"/WLAN-Liste+Pfeil/
  Tastatur+Umlaute/Englisch/Wischen - siehe Abschluss-Zusammenfassung
  fuer die genaue Testliste.

### Hintergrundfarben (nur genannt, NICHT geaendert)

Alle Bildschirme dieses Auftrags nutzen `tft.fillScreen(TFT_BLACK)` -
reines Schwarz (`#000000`), IDENTISCH zu Hauptscreen/Controls/
Bootscreen. Kein Bildschirm im Projekt verwendet eine andere
Hintergrundfarbe (siehe Ursachen-Abschnitt oben zum vermuteten
"dunkelblau").

### Dokumentation

- [ ] README/docs (DE/EN): Zurueck-Pfeil in der WLAN-Liste beschreiben
      (noch offen, siehe TODO unten)
- [x] CLAUDE.md: dieser Abschnitt

**Noch offen:** README/docs-Ergaenzung (Zurueck-Pfeil), die vier
restlichen `LAYOUT_DEBUG`-`beginScreen()`-Eintraege (Einrichtung-
Untermenue/Luefter-Boost/Helligkeit/Hilfe-Popup/Kalibrierung/
Sprachauswahl/Verbindungstest), ein erweiterter `SCREEN_TOUR` fuer die
interaktiven Bildschirme (braucht eine Render/Eingabe-Trennung), die
interaktiven Live-Tests mit Alex (Schritt 7).

**Von Alex live bestaetigt (09.11.):** Zurueck-Pfeil in der WLAN-Liste
funktioniert wie vorgesehen (war zunaechst nur uebersehen worden).

## Auftrag Display aus im Ruhezustand: Fortschritt

Neuer Auftrag: tippt Alex auf Controls die Kachel "Mac Mini M4" und der
Mac geht in den Ruhezustand, soll die Hintergrundbeleuchtung ausgehen -
und beim Aufwecken des Mac von selbst wieder angehen. Nur `cyd/`
betroffen, NICHT committen/taggen/pushen. Hauptscreen/Controls/
Bootscreen bleiben optisch unveraendert, `config.json`/Helligkeits-
Einstellung bleiben unberuehrt (Zustand nur im RAM).

### Schritt 1: Fundstellen (main.cpp)

- **Backlight**: `applyBrightness()` (Zeile ~597) - `analogWrite(TFT_BL,
  map(pct,0,100,0,255))`, `pct` aus `deviceCfg.brightness` (10-100%,
  Menue). `TFT_BL` kommt aus der TFT_eSPI-Pin-Konfiguration
  (`platformio.ini`/`User_Setup`).
- **SLEEP-Befehl**: Mac-Kachel-Tap in `handleControlsPage()` ruft
  `sendSleepCommand()` auf und setzt `sleepCmdPending`/
  `sleepCmdBaselineAck` - das ist aber nur die Pruefung, ob der Befehl
  BESTAETIGT wurde (Feld `sleep_ack` im naechsten Datenpaket), NICHT ob
  der Mac wirklich eingeschlafen ist (ein abgelehnter Befehl kann
  trotzdem bestaetigt werden, der Mac schickt danach aber weiter Daten).
- **Datenempfang**: `lastUdpPacketMs`/`lastUsbPacketMs`, in
  `parsePacket()` nur bei erfolgreichem JSON-Parse aktualisiert (das ist
  die "gueltige Pruefsumme" aus dem Auftrag - es gibt keine eigene
  Checksumme, nur die JSON-Gueltigkeit). Neue Hilfsfunktion
  `computeLastAnyPacketMs()` (ersetzt ein Code-Duplikat in `loop()`) -
  modusabhaengig (wifi/usb/auto wie bisher).
- **"Keine Verbindung"-Ausloeser**: `loop()`, `bool noData = (millis() -
  lastAnyPacketMs > Config::NO_DATA_TIMEOUT_MS) || lastAnyPacketMs==0`
  (Timeout 6s), toggelt `showingNoConnection` und zeichnet bei Main
  entsprechend um.

### Schritt 2+3: Zustandsautomat `MacSleepState` (main.cpp)

Drei Zustaende (`enum class MacSleepState { Off, Requested, Active }`,
nur im RAM, kein SD-/NVS-Zugriff):

1. **Requested** (gesetzt beim Mac-Kachel-Tap, zusaetzlich zum
   bestehenden `sleepCmdPending`): `macSleepRequestedAtMs`=Sendezeit,
   `macSleepBaselineLastPacketMs`=`computeLastAnyPacketMs()` beim
   Senden. Jede Runde prueft `updateMacSleepState()`:
   - Kommt in der Zwischenzeit ein NEUES Paket (Wert weicht von der
     Momentaufnahme ab) -> zurueck zu `Off`, Log "abgebrochen (Daten
     kommen weiter)".
   - Nach `MAC_SLEEP_CONFIRM_MS`=10000ms ohne neue Daten -> `Active`,
     `currentPage=Main` (erzwungen, egal wo Alex gerade war),
     `showingNoConnection=true` (sachlich korrekt, wird aber NICHT
     gezeichnet), `analogWrite(TFT_BL,0)`, Log "bestaetigt, Display
     aus".
2. **Active, dunkel** (`macSleepWokenAtMs==0`): jede Beruehrung wird in
   `loop()` VOR dem normalen Dispatch abgefangen und verschluckt (siehe
   Schritt 4), sonst passiert nichts von selbst.
3. **Zurueck zu Off**: sobald `computeLastAnyPacketMs()` vom
   Momentaufnahme-Wert abweicht (neue echte Daten), egal ob dunkel oder
   gerade per Beruehrung wach - `applyBrightness()` (liest die AKTUELLE
   `deviceCfg.brightness`, siehe Auftrag Punkt g), Log "Daten da,
   Display an". WICHTIG: `updateMacSleepState()` wird in `loop()` ERST
   NACH der bestehenden `showingNoConnection`-Umschaltung aufgerufen -
   die zeichnet bei neuen Daten (da `currentPage==Main` erzwungen war)
   bereits synchron `drawStaticLayout()`, BEVOR die Beleuchtung wieder
   angeht (Auftrag: "Beleuchtung geht erst an, wenn das Dashboard fertig
   gezeichnet ist").

### Schritt 4: Beruehrung weckt kurz auf

In `loop()`, direkt vor dem normalen `currentPage`-Dispatch: ist
`MacSleepState::Active` und `macSleepWokenAtMs==0`, wird EINMAL
`TouchCalib::pollGesture()` aufgerufen - liefert sie irgendeine Geste
(`!= Gesture::None`), gilt sie als "verschluckt" (der nachfolgende
Dispatch bekommt in dieser Runde keine Geste mehr, da `pollGesture()`
sie bereits konsumiert hat - kein Sonderfall noetig, dieselbe
Antippen-Erkennung wie ueberall sonst): `macSleepWokenAtMs=millis()`,
`applyBrightness()`, `drawNoConnectionScreen()`, Log "Beruehrung, 15s
an", dann `return;` (ueberspringt den Dispatch komplett fuer diese
Runde). Nach `MAC_SLEEP_TOUCH_WAKE_MS`=15000ms ohne echte neue Daten
setzt `updateMacSleepState()` `macSleepWokenAtMs` zurueck auf 0 und
schaltet die Beleuchtung wieder aus - zurueck in den dunklen
Unterzustand. WAEHREND der 15s laeuft der normale Dispatch (Main +
`showingNoConnection`) ganz normal weiter (Menue-Taste, Wischen
funktionieren) - nur die EINE weckende Beruehrung selbst loest nichts
aus, danach bedient sich alles wie beim normalen "Keine Verbindung"-
Bildschirm.

### Schritt 5: Sonderfaelle

- Mac-Absturz/WLAN-Ausfall ohne Kachel-Tap: `macSleepState` bleibt
  `Off`, `updateMacSleepState()` kehrt sofort zurueck (kein Eingriff) -
  die bestehende "Keine Verbindung"-Logik laeuft unveraendert.
- Kachel waehrend `Active`+dunkel nochmal erreichbar? Nein -
  `currentPage` ist auf `Main` erzwungen UND jede Beruehrung wird vor
  dem Dispatch abgefangen - `handleControlsPage()` (und damit die
  Kachel) wird in diesem Zustand nie aufgerufen.
  Formal vom Auftrag verlangtes Verhalten ("passiert nichts") damit
  strukturell garantiert, nicht nur per Sonderfallpruefung.
- Menue/Einrichtung/Helligkeit/Sprache: laufen innerhalb der 15s-
  Weckzeit ueber den UNVERAENDERTEN bestehenden Code-Pfad (Taste
  "Menue" auf dem "Keine Verbindung"-Bildschirm) - keine eigene Logik
  noetig.
- Helligkeitsaenderung im Menue gilt beim naechsten Aufwecken, weil
  `applyBrightness()` immer `deviceCfg.brightness` frisch liest, nie
  einen beim Einschlafen zwischengespeicherten Wert.
- `millis()`-Ueberlauf (~49 Tage): alle Vergleiche nutzen das Projekt-
  weit uebliche Subtraktions-Muster (`millis() - zeitpunkt >= dauer`),
  das ist bei vorzeichenlosen `uint32_t` automatisch ueberlaufsicher -
  keine neue Absicherung noetig, nur konsequent dasselbe Muster wie
  ueberall sonst im Code verwendet.
- Log-Zeilen exakt wie im Auftrag gefordert: "Mac-Schlaf: angefordert" /
  "...bestaetigt, Display aus" / "...abgebrochen (Daten kommen weiter)"
  / "...Daten da, Display an" / "...Beruehrung, 15s an".

### Schritt 6: Gebaut, geflasht

- `pio run` fehlerfrei (Flash 75,4%, RAM 14,8% - kaum Aenderung
  gegenueber dem vorherigen Stand). Geflasht auf
  `/dev/cu.wchusbserial31410`, automatische Mac-App-Portfreigabe hat
  funktioniert. Verbindung war laut `lsof`-Pruefung bereits **1 Sekunde**
  nach Upload-Ende wieder aktiv (identisch zum vorherigen Auftrag).
  `config.json` nicht angefasst.
- Flightradar/Solar-Guardian-CYD per `git status` als unveraendert
  bestaetigt.
- **Interaktive Live-Tests a)-g) mit Alex noch OFFEN** (brauchen ein
  echtes Einschlafen/Aufwecken des Mac mini, siehe Auftrag Schritt 6 -
  kann nicht ohne Alex' Mac durchgefuehrt werden).

### Dokumentation

- [x] CLAUDE.md: dieser Abschnitt
- [ ] README/docs (DE/EN) Satz ergaenzen - noch offen

**Noch offen:** README/docs-Satz, die Live-Tests a)-g) mit Alex.

## Datenschutz-Regel (gilt für ALLES, ohne Ausnahme)

Der echte Name, die private E-Mail-Adresse, Mac-Benutzernamen/-Pfade (`/Users/...`) und echte Schlüssel
dürfen NIE in Dateien, Commits, Tags, Release-Texten oder Doku stehen. Öffentlich gibt es nur das
Pseudonym "Eiswolf-BG" mit der GitHub-noreply-Adresse. Pfade immer als `~/...` schreiben. Wenn der echte
Name erklärt werden muss, heißt er hier nur "der echte Name". Vor jedem Commit kurz suchen (z. B. mit
`git ls-files -co --exclude-standard -z | xargs -0 grep -a -i -n` nach Namen, Benutzername, `/Users/`,
privater Adresse). Lokaler Git-Name/-Mail dieses Repos: "Eiswolf-BG" + noreply-Adresse (Kontrolle:
`git config user.name`, `git config user.email`).

## Auftrag Verlauf bereinigen: Fortschritt

- [x] Sicherungen vor Beginn: FULL_BACKUP (tar.gz) und history_backup (git bundle, enthält v1.0/v1.1) im
      Home-Ordner, bleiben unangetastet.
- [x] Der echte Name, der Mac-Benutzername und die privaten Pfade sind im GANZEN Verlauf ersetzt (Dateien in
      allen Commits und Tags; Autoren, Committer und Tagger waren schon nur "Eiswolf-BG" mit noreply-Adresse).
      Regeln: Name -> "Eiswolf-BG", `/Users/<Benutzername>` -> `~`, Kennung `com.<Benutzername>.eiswolfsiboard`
      -> `com.eiswolf.iboard` (zwei Plist-Dateien umbenannt), Vorname bleibt (Entscheidung des Besitzers),
      private Netzwerk-Beispieladressen bleiben (Entscheidung des Besitzers).
- [x] Die alten `firmware.bin` in allen 5 Commits enthielten den Benutzernamen als Text in eingebetteten
      Compile-Pfaden (16 Stellen). Er wurde durch einen gleich langen neutralen Text ersetzt, die Prüfsumme und
      der SHA-256 des ESP-Images wurden neu berechnet (`esptool image_info`: Checksum und Validation hash valid).
      NICHT auf einem echten Gerät getestet. `bootloader.bin`, `partitions.bin`, `index.html`, `manifest.json`
      sind byte-identisch.
- [x] Prüfung der bereinigten Kopie: im gesamten Verlauf, in allen Tags und Dateien (auch binär) 0 Treffer für
      Name, Benutzername, private E-Mail; Dateibäume vorher/nachher gleich bis auf die ersetzten Dateien.
- [x] Arbeitsordner an die neue Historie angeglichen (`git reset --mixed`, Dateien unangetastet: alle 6060
      Dateien byte-identisch zu vorher). `git status` zeigt statt 55 jetzt 47 Einträge: 8 Einträge waren reine
      Namens-/Kennungs-Änderungen (zwei Plist-Umbenennungen und vier Skripte), die jetzt im Verlauf stehen.
- [x] Lokaler Git-Name "Eiswolf-BG" + noreply-Adresse gesetzt. Die globale Git-Konfiguration war schon
      "Eiswolf-BG" + noreply-Adresse, unverändert.
- [x] Force-Push erledigt (15.11.2026): `main` (5 Commits, jetzt `caff9bb`) mit `--force-with-lease` und NUR die
      bestehenden Tags `v1.0`/`v1.1` mit Force auf die umgeschriebenen Commits gesetzt, keine neuen Tags, kein
      Release, kein `--mirror`/`--all`. Auf GitHub gab es vorher nur `main` und diese zwei Tags.
- [x] GitHub kontrolliert: alle 5 Commits nur "Eiswolf-BG", `LICENSE.md` Copyright "Eiswolf-BG", 0 Treffer in
      den geprüften Dateien, Web-Installer-Dateien (`firmware.bin`, `bootloader.bin`, `partitions.bin`,
      `manifest.json`, `index.html`) identisch zum geprüften Stand, Installer-Seite antwortet mit 200. NICHT
      prüfbar: die Commit-Seiten im Browser (nur über die API gesehen). Der alte Commit `a0d9877` ist über die
      API noch abrufbar (Zwischenspeicher, siehe unten).
- [x] Alle 7 Anhänge der Releases v1.0 und v1.1 sind von GitHub entfernt (15.11.2026; Releases und Tags unverändert, Beschreibungstexte
      waren sauber); für den nächsten Release müssen saubere Dateien gebaut werden: Firmware aus neutralem Pfad bzw. mit
      Pfad-Maskierung, Listener mit `-trimpath`, Mac-App geprüft, und ALLE Release-Dateien vor dem Hochladen (auch Binärdateien)
      auf den echten Namen, den Benutzernamen und die private E-Mail durchsuchen. Bis dahin liefern die Download-Knöpfe auf der
      Installer-Seite (`releases/latest/download/...`) 404.
- [ ] Löschung der alten Commit-IDs aus dem Zwischenspeicher beim GitHub-Support beantragen (Antragstext hat Alex bekommen).
