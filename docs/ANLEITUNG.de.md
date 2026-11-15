# Eiswolfs iBoard – Anleitung (Deutsch)

> **Das ist die einzige Quelle der Anleitung.** Aus dieser Datei (und der englischen Schwesterdatei
> `ANLEITUNG.en.md`) entstehen später alle PDFs und Kopien. Änderungen bitte nur hier machen.
> Stellen mit `[Foto: …]` sind Platzhalter für Bilder, `[Generation]` und `[OFFEN: …]` werden noch ergänzt.

Eiswolfs iBoard zeigt die Werte deines Mac (CPU, GPU, Temperaturen, Lüfter, Leistung) live auf einem
kleinen Display, dem **CYD** (ESP32-2432S028, „Cheap Yellow Display“). Zusätzlich kann das CYD einen
Windows-Laptop **einschalten** (Wake-on-LAN) und **ausschalten**, den Monitor umschalten und auf
**Alexa** hören.

Du musst nicht alles einrichten. Fang mit den Kapiteln 1 bis 4 an, dann hast du schon das Display mit den
Mac-Werten. Alles Weitere (Laptop wecken, Alexa …) ist freiwillig.

**Inhalt**

1. [Was du brauchst](#1-was-du-brauchst)
2. [CYD flashen](#2-cyd-flashen)
3. [WLAN und Einrichtung am CYD](#3-wlan-und-einrichtung-am-cyd)
4. [Mac-App installieren](#4-mac-app-installieren)
5. [Schlüssel erzeugen und eintragen](#5-schlüssel-erzeugen-und-eintragen)
6. [Feste IP im Router](#6-feste-ip-im-router-beispiel-fritzbox)
7. [Wake-on-LAN am Laptop](#7-wake-on-lan-am-laptop)
8. [Windows-Listener](#8-windows-listener)
9. [Monitor-Umschaltung](#9-monitor-umschaltung)
10. [Alexa](#10-alexa)
11. [Fehlersuche](#11-fehlersuche)
12. [Sicherheit](#12-sicherheit)

---

## 1. Was du brauchst

**Für das Display (Pflicht):**

- Ein **CYD**, und zwar die klassische Variante **ESP32-2432S028** mit Touchscreen, der auf Druck reagiert
  (resistiv). Eine Variante mit kapazitivem Touch ist **nicht** geeignet.
- Eine **microSD-Karte**, mit **FAT32** formatiert. Das CYD speichert darauf seine Einstellungen.
- Ein **USB-Kabel, das Daten überträgt** (nicht nur Strom). Wenn der Computer das CYD nicht erkennt, ist
  meist das Kabel schuld.
- Einen Computer mit **Chrome, Edge oder Opera** (zum Flashen im Browser).
- Einen **Mac mit Apple Silicon** (M1 oder neuer) für die Mac-App.
- Ein **normales Heim-WLAN mit 2,4 GHz**. Ein Gastnetz funktioniert meist nicht, weil Gastnetze die Geräte
  voneinander trennen. 5-GHz-Netze kann das CYD nicht.

**Nur für die Extras (freiwillig):**

- Einen **Windows-Laptop**, wenn das CYD ihn ein- und ausschalten soll.
- Einen **Monitor mit mehreren Eingängen** (HDMI/DisplayPort), wenn er beim Einschalten umschalten soll.
- Ein **Alexa-Gerät** (zum Beispiel einen Echo Dot) und die Alexa-App.
- Zugang zu den **Einstellungen deines Routers** (Beispiel in Kapitel 6: FritzBox).

[Foto: Alle Teile auf einem Tisch – CYD, microSD-Karte, USB-Kabel]

---

## 2. CYD flashen

„Flashen“ heißt: die Firmware (das Programm des Displays) auf das CYD übertragen. Das musst du nur einmal
machen und bei neuen Versionen wiederholen. Die Einstellungen auf der SD-Karte bleiben dabei erhalten.

### Weg A: Web-Installer (empfohlen)

1. Stecke die **microSD-Karte** in das CYD.
2. Verbinde das CYD per **USB** mit dem Computer.
3. Öffne im Browser (**Chrome, Edge oder Opera**) die Seite
   **https://eiswolf-bg.github.io/Eiswolfs-iBoard-CYD/**.
4. Klicke auf **„Connect“** und wähle in der Liste das CYD aus.
5. Bestätige die Installation. Der Rest läuft von allein. Das dauert ein bis zwei Minuten, bitte das Kabel
   nicht abziehen.

[Foto: Web-Installer-Seite mit dem Knopf „Connect“]

Wenn in der Liste nichts erscheint, fehlt meist der **Treiber für den USB-Chip (CH340)**:
[Windows](https://www.wch-ic.com/downloads/CH341SER_EXE.html) ·
[macOS](https://www.wch-ic.com/downloads/CH341SER_MAC_ZIP.html). Unter Linux ist er meist schon dabei.
Danach Kabel neu stecken und es noch einmal versuchen.

### Weg B: esptool (Kommandozeile)

Für alle, die den Web-Installer nicht nutzen können. Du brauchst die Datei
**`Eiswolfs-iBoard-CYD-merged.bin`** von der Releases-Seite des Projekts (sie enthält Bootloader,
Partitionstabelle und Programm in einer Datei) und Python.

```
pip install esptool
esptool.py --chip esp32 --port <dein-Port> write_flash 0x0 Eiswolfs-iBoard-CYD-merged.bin
```

Den Port findest du auf dem Mac mit `ls /dev/cu.*`, unter Windows im Geräte-Manager (meist `COM3` o. Ä.).

---

## 3. WLAN und Einrichtung am CYD

Beim allerersten Einschalten richtet sich das CYD selbst am Display ein:

1. **Touch kalibrieren:** Tippe nacheinander die vier angezeigten Kreuze an (oben links, oben rechts,
   unten rechts, unten links). Danach tippst du den **Testpunkt** in der Mitte an.
2. **WLAN wählen:** Wähle dein **Heim-WLAN** (2,4 GHz, kein Gastnetz) aus der Liste. Ist es nicht
   sichtbar, tippe auf „Neu suchen“ oder „Manuell eingeben“.
3. **Passwort eingeben:** Mit der Bildschirmtastatur. Das Passwort steht gleich im Klartext, „Verbergen“ schaltet auf Sterne um. Der erste Buchstabe ist groß, danach schreibt die Tastatur klein weiter. Mit der Taste links unten schaltest du die Großschreibung von Hand um.
4. Wenn „Verbunden!“ erscheint, ist das CYD im WLAN.
5. Zuletzt fragt es nach der **Sprache** (Deutsch oder English).

[Foto: Kalibrierung mit den vier Kreuzen]
[Foto: WLAN-Liste am CYD]

Das CYD und der Mac müssen im **selben WLAN** sein.

**Später etwas ändern:** **Wische auf dem Hauptbildschirm nach links** zum Controls-Bildschirm (Seite 2) und tippe auf die Kachel **„Menü“**. (Auf dem Bildschirm „Keine Verbindung zum Mac“ heißt der Knopf ebenfalls **„Menü“**.) Tippe dort auf **„Einrichtung“**. Dort gibt es:

- **Nur WLAN ändern** – anderes WLAN wählen, ohne alles neu einzurichten.
- **Touch neu kalibrieren**
- **Helligkeit**
- **WoL-Geräte** – Laptops einrichten, die das CYD wecken und ausschalten soll (Kapitel 7 und 8).
- **Zähler neu abgleichen** – nur nötig, wenn Befehle nach einem Zurücksetzen nicht mehr ankommen (Kapitel 5).
- **Alexa** – Sprachsteuerung (Kapitel 10).

Hinter jedem Menüpunkt steht ein kleines **„?“**. Tippst du darauf, bekommst du eine kurze Erklärung.

[Foto: Menü „Einrichtung“ am CYD]

---

## 4. Mac-App installieren

Die Mac-App sitzt in der Menüleiste und schickt die Werte an das CYD.

1. Lade von der Releases-Seite die Datei **`Eiswolfs-iBoard.dmg`** herunter und öffne sie per Doppelklick.
2. Ziehe die App **„Eiswolfs iBoard“** auf das Symbol **„Programme“**.
3. **Erster Start (Gatekeeper):** Die App ist nicht über ein kostenpflichtiges Apple-Entwicklerkonto
   signiert, deshalb warnt macOS. Das ist normal.
   - **Nicht doppelklicken.** Öffne den Ordner „Programme“, klicke mit der **rechten Maustaste** (oder
     mit zwei Fingern) auf „Eiswolfs iBoard“ und wähle **„Öffnen“**. Bestätige im Dialog noch einmal mit
     **„Öffnen“**.
   - Erscheint nur eine Meldung ohne „Öffnen“: **Systemeinstellungen → Datenschutz & Sicherheit**, ganz
     unten bei „Eiswolfs iBoard“ auf **„Trotzdem öffnen“** klicken.
   - Das ist nur beim ersten Start nötig.
4. **Netzwerk erlauben:** Beim ersten Start fragt macOS, ob die App im **lokalen Netzwerk** senden darf.
   Antworte mit **„Erlauben“**. Ohne diese Erlaubnis kommen keine Werte am CYD an. Nachträglich ändern:
   Systemeinstellungen → Datenschutz & Sicherheit → Lokales Netzwerk.
5. **Automatisch starten:** Klicke auf das Symbol in der Menüleiste und hake **„Beim Login starten“** an.

[Foto: DMG-Fenster mit App und „Programme“]
[Foto: Rechtsklick → „Öffnen“]
[Foto: Menüleisten-Symbol mit geöffnetem Menü]

Nach wenigen Sekunden zeigt das CYD die Werte deines Macs. Zeigt es „Keine Verbindung zum Mac“, schau in
Kapitel 11.

**Hinweis:** Nicht jeder Mac hat alle Sensoren. Bei Macs ohne Lüfter (zum Beispiel MacBook Air) zeigt die
Lüfter-Kachel dauerhaft „--“. Das ist kein Fehler.

---

## 5. Schlüssel erzeugen und eintragen

Fürs **Ein- und Ausschalten** und einige Extras (Lüfter-Boost, Ruhezustand, Windows herunterfahren) benutzt
das System einen **gemeinsamen Schlüssel**. Das ist ein Geheimtext, den du dir selbst ausdenkst. Er muss an
allen Stellen **Zeichen für Zeichen gleich** sein. Er schützt dich davor, dass ein fremdes Gerät in deinem
WLAN deinem Rechner Befehle schickt.

Der Schlüssel verlässt dein Heimnetz nie. Aufwecken per Wake-on-LAN und die Anzeige der Werte
brauchen **keinen** Schlüssel.

**Wichtig:** Schreibe deinen Schlüssel nie in ein Foto, einen Chat, ein Forum oder eine Datei, die du
weitergibst. In dieser Anleitung steht immer nur der Platzhalter `<DEIN-SCHLUESSEL>`.

### Schlüssel erzeugen

Öffne auf dem Mac das **Terminal** und gib ein:

```
openssl rand -hex 24
```

Die Ausgabe (48 Zeichen aus Ziffern und den Buchstaben a–f) ist dein Schlüssel. Kopiere sie. Alternativ
reicht jeder zufällige Text mit mindestens 30 Zeichen.

### Schlüssel eintragen

**a) Mac-App.** Starte die App einmal und beende sie wieder (dann legt sie ihre Einstellungsdatei an).
Öffne die Datei `~/Library/Application Support/Eiswolfs iBoard/config.json` mit einem Texteditor und trage
ein:

```json
{
  "fan_boost_token": "<DEIN-SCHLUESSEL>"
}
```

Die anderen Einträge in der Datei bleiben stehen. Dann die App wieder starten.

**b) Windows-Listener** (nur, wenn das CYD den Laptop herunterfahren soll): am einfachsten geht es mit
dem Mac-Menü **„Token an Windows senden…“**. Es zeigt eine Adresse, die **60 Sekunden** gilt. Öffne sie im
Browser des Windows-Laptops. Es lädt eine `config.json` herunter, die **nur den Schlüssel** enthält.
**Überschreibe nicht die Datei des Listeners damit**, denn dort stehen auch Gerätename, Port und die
Monitor-Einstellungen, die sonst verloren gehen. Mache es so: Rechtsklick auf das Listener-Symbol →
**„Einstellungen…“** → beim Schlüssel auf **„Aus Datei wählen…“** und die heruntergeladene Datei auswählen.
Dann wird nur der Schlüssel übernommen. Danach im Symbolmenü auf **„Token neu laden“** klicken. Oder du
tippst den Schlüssel direkt im Einrichtungsfenster ein (Kapitel 8).

**c) CYD-Firmware.** Der Schlüssel steckt im Programm des CYD (Datei `cyd/src/secrets.h`) und wird beim
Bauen der Firmware eingebaut:

```cpp
#pragma once
static const char* const FAN_BOOST_TOKEN = "<DEIN-SCHLUESSEL>";
```

Danach die Firmware selbst bauen und flashen (`cd cyd && pio run --target upload`, siehe
`docs/DEVELOPMENT.md`).

> **[OFFEN: Schlüssel bei der fertigen Firmware.]** Die fertige Firmware aus dem Web-Installer enthält
> bewusst **keinen** echten Schlüssel. Damit sind Lüfter-Boost, Ruhezustand und das Herunterfahren des
> Laptops mit dieser Datei nicht nutzbar. Aufwecken, Display, Menü und Alexa-Einschalten gehen trotzdem. Ob
> und wie man den Schlüssel am Gerät selbst eintragen kann, wird noch entschieden und hier ergänzt.

### Wenn Befehle plötzlich nicht mehr ankommen („Resync“)

Jeder Befehl trägt einen Zähler, der nur steigen darf. Verliert das CYD seinen Zähler (zum Beispiel nach
einem Zurücksetzen), werden seine Befehle abgelehnt. Dann: Am Empfänger den Zähler-Abgleich für **60
Sekunden** freigeben (Mac-Menü → Sicherheit → „Zähler-Resync zulassen (60s)…“, Windows-Taskleistensymbol →
„Zähler-Resync zulassen (60s)“) und gleich danach am CYD im Einrichtungsmenü auf **„Zähler neu abgleichen“** tippen.

---

## 6. Feste IP im Router (Beispiel FritzBox)

Aufwecken braucht keine IP-Adresse. Das **Herunterfahren** schickt das CYD aber gezielt an eine
IP-Adresse. Deshalb muss der Laptop **immer dieselbe IP** haben. Sonst hört das Ausschalten irgendwann
einfach auf zu funktionieren.

**Am besten im Router festlegen (FritzBox):**

1. Öffne die Oberfläche deiner FritzBox (meist `http://fritz.box`) und melde dich an.
2. **Heimnetz → Netzwerk** und klicke beim Laptop auf das **Stift-Symbol (bearbeiten)**.
3. Hake **„Diesem Netzwerkgerät immer die gleiche IPv4-Adresse zuweisen“** an und speichere.

[Foto: FritzBox, Heimnetz → Netzwerk, Gerät bearbeiten]

Andere Router haben dafür eine Einstellung wie „DHCP-Reservierung“ oder „Feste IP-Vergabe“.

**Wenn dein Router das nicht kann**, stelle am Gerät selbst eine feste IP ein:

- **Windows:** Einstellungen → Netzwerk und Internet → deine Verbindung → bei „IP-Zuweisung“ auf
  **Bearbeiten** → **Manuell**. Trage eine IP **außerhalb** des DHCP-Bereichs deines Routers ein, dazu
  Subnetzmaske, Gateway und DNS (meist beides die IP des Routers).
- **macOS:** Systemeinstellungen → Netzwerk → deine Verbindung → Details → TCP/IP → „IPv4 konfigurieren:
  Manuell“.

### IP und MAC-Adresse finden

Du brauchst beides für die Einrichtung am CYD.

- **Windows:** Öffne die **Eingabeaufforderung** und gib `ipconfig /all` ein. Suche den aktiven Adapter
  (bei Kabel meist „Ethernet-Adapter Ethernet“). **„Physische Adresse“** ist die MAC (sechs Paare wie
  `AA-BB-CC-DD-EE-FF`), **„IPv4-Adresse“** ist die IP.
- **macOS:** Systemeinstellungen → Netzwerk → deine Verbindung → Details. Reiter **Hardware** zeigt die MAC,
  Reiter **Netzwerk** die IP.

[Foto: Ausgabe von ipconfig /all mit markierter MAC und IP]

**Tipp zum Mac:** Schalte für dein WLAN die **„Private WLAN-Adresse“** aus (Systemeinstellungen → WLAN →
dein Netzwerk → Details). Eine zufällige Adresse ändert sich regelmäßig und würde auch eine feste IP im
Router aushebeln.

---

## 7. Wake-on-LAN am Laptop

Wake-on-LAN („WoL“) weckt den Laptop mit einem Netzwerk-Signal. Es muss an **zwei** Stellen
eingeschaltet sein.

1. **BIOS/UEFI:** Starte den Laptop neu und rufe das BIOS auf (je nach Hersteller die Taste F2, F10, Entf
   oder Esc direkt nach dem Einschalten). Suche unter den Energie-Einstellungen nach **„Wake on LAN“** oder
   **„Power On by PCI-E/PCIe“** und schalte es **ein**.
2. **Netzwerkkarte in Windows:** Geräte-Manager → **Netzwerkadapter** → dein Adapter → **Eigenschaften**.
   - Reiter **Energieverwaltung:** **„Gerät kann den Computer aus dem Ruhezustand aktivieren“** anhaken.
   - Reiter **Erweitert:** **„Wake on Magic Packet“** auf **Aktiviert**, falls vorhanden.
3. **Schnellstart ausschalten:** Systemsteuerung → Energieoptionen → „Auswählen, was beim Drücken von
   Netzschaltern geschehen soll“ → „Einige Einstellungen sind momentan nicht verfügbar“ anklicken →
   **„Schnellstart aktivieren“ abhaken**. Der Schnellstart legt den Laptop beim Ausschalten in einen
   Zustand, aus dem er sich oft nicht mehr wecken lässt.

[Foto: BIOS mit „Wake on LAN“]
[Foto: Geräte-Manager, Reiter „Energieverwaltung“]

**Verbinde den Laptop per Netzwerkkabel**, wenn es geht. Über WLAN lässt sich ein Laptop in der Regel nicht
aus dem vollständig ausgeschalteten Zustand wecken, höchstens aus dem Ruhezustand.

### Den Laptop am CYD anlegen

1. Am CYD: **Menü (Seite 2, Kachel „Menü“) → Einrichtung → „WoL-Geräte“ → „Gerät hinzufügen“**.
2. **Name** eingeben (frei wählbar, steht auf der Kachel).
3. **MAC-Adresse** eingeben, Format `AA:BB:CC:DD:EE:FF` (aus Kapitel 6).
4. **IP fürs Ausschalten** eingeben (empfohlen, siehe Kapitel 6). Mit „Zurück“ überspringst du es, dann
   kann das CYD den Laptop nur wecken, nicht ausschalten.
5. Optional: **Monitor beim Einschalten** wählen (Kapitel 9).

Es sind bis zu **4 Geräte** möglich. Das erste Gerät liegt oben links auf dem **Controls**-Bildschirm (nach
links wischen). Ab dem zweiten Gerät gibt es eine zweite Seite „Geräte“.

**Bedienung:** **Kurz tippen** auf die Kachel weckt das Gerät. **Lange halten** schaltet es aus (nur mit
eingetragener IP, installiertem Windows-Listener und Schlüssel).

---

## 8. Windows-Listener

Der Listener ist ein kleines Programm auf dem Windows-Laptop. Es nimmt den Ausschalt-Befehl vom CYD
entgegen. **Zum Aufwecken brauchst du ihn nicht.**

1. Lade **`EiswolfsIBoardListener-Setup.exe`** von der Releases-Seite herunter und starte sie.
2. **SmartScreen:** Windows zeigt „Der Computer wurde durch Windows geschützt“, weil das Programm nicht mit
   einem kostenpflichtigen Zertifikat signiert ist. Das ist normal. Klicke auf **„Weitere Informationen“**
   und dann auf **„Trotzdem ausführen“**. Wer sicher gehen will, vergleicht die SHA-256-Prüfsumme mit der
   auf der Releases-Seite: `Get-FileHash EiswolfsIBoardListener-Setup.exe -Algorithm SHA256` (PowerShell).
3. **Administrator-Frage (UAC):** Einmal mit **„Ja“** bestätigen. Das Programm legt damit die
   **Firewall-Regel** an, die den Befehl durchlässt.
4. Das Einrichtungsfenster fragt:
   - **Gerätename:** frei wählbar, nur zur Übersicht.
   - **Port:** **4212** so lassen. Das CYD benutzt diesen Port.
   - **Schlüssel:** derselbe wie überall (Kapitel 5), eintippen oder über „Aus Datei wählen…“ die vom Mac
     erzeugte Datei nehmen.
   - **Beim Anmelden automatisch starten (Autostart):** angehakt lassen.
5. Zweite Seite **„Monitor-Umschaltung“:** Siehe Kapitel 9. Ohne Monitor-Umschaltung einfach überspringen.

Danach erscheint ein Symbol unten rechts neben der Uhr (eventuell hinter dem Pfeil „Ausgeblendete Symbole
einblenden“). Das Symbol zeigt den Status („Token gesetzt, bereit auf Port 4212“) und hat ein Menü mit
„Einstellungen…“, „Token neu laden“, „Protokoll öffnen“ und „Beenden“. Das Menü ist nur auf Deutsch.

**Firewall:** Hat die automatische Regel nicht geklappt, erscheint beim ersten Befehl ein Windows-Fenster
„Defender-Firewall hat Features blockiert“. Erlaube dort **nur „Private Netzwerke“**.

**Wichtig:** Das Netzwerk des Laptops muss als **„Privat“** eingestuft sein. Windows stuft es nach einem
Neustart manchmal auf „Öffentlich“ zurück, dann blockiert die Firewall still. Prüfen mit PowerShell:
`Get-NetConnectionProfile`.

**Aktualisieren:** Einfach die neue Setup-Datei noch einmal starten. Alle bisherigen Einstellungen werden
vorausgefüllt. **Deinstallieren:** Startmenü → „Eiswolfs iBoard“ → „Deinstallieren“.

[Foto: SmartScreen-Warnung mit „Weitere Informationen“]
[Foto: Einrichtungsfenster des Listeners]
[Foto: Taskleistensymbol mit Menü]

---

## 9. Monitor-Umschaltung

Teilen sich Mac und Laptop einen Monitor, kann das System beim Einschalten des Laptops den Monitor auf
den richtigen **Eingang** umstellen (HDMI 1, HDMI 2, DisplayPort 1 oder 2) und beim Ausschalten wieder
zurück. Das ist freiwillig.

**Voraussetzung:** Am **Monitor** muss **DDC/CI** eingeschaltet sein (steht im Bildschirmmenü des Monitors,
oft unter „System“ oder „Sonstiges“). Manche Monitore, Adapter oder Docks lassen das Umschalten nicht zu.
Das lässt sich nur ausprobieren.

**So funktioniert es:**

- **Einschalten:** Das CYD weckt den Laptop und sagt gleichzeitig der **Mac-App** Bescheid. Die Mac-App stellt
  über den Monitor-Anschluss des Mac den gewünschten Eingang ein.
- **Ausschalten:** Der **Windows-Listener** schaltet den Monitor zurück, **bevor** er den Laptop
  herunterfährt.

**Einrichten:**

1. **Mac-App:** Menü **„Monitor-Umschaltung…“** → **„Ein“**. Unter **„Bildschirm“** den Monitor wählen
   (steht nichts da: „Bildschirme neu suchen“). Mit **„Test: auf HDMI 2 schalten“** kannst du prüfen, ob
   das Umschalten klappt. Der Monitor springt kurz um und nach 15 Sekunden von selbst zurück. Bei manchen
   Monitoren sind die Zahlen für die Eingänge anders. Unter „Eingangswerte (VCP 0x60)“ lassen sie sich
   anpassen.
2. **CYD:** Beim Laptop in **WoL-Geräte** (bearbeiten) im letzten Schritt **„Monitor beim Einschalten“**
   wählen: Aus, HDMI 1, HDMI 2, DisplayPort 1 oder DisplayPort 2.
3. **Windows-Listener:** Auf der Seite **„Monitor-Umschaltung“** des Einrichtungsfensters einschalten, den
   Monitor und den Eingangswert wählen, auf den **zurückgeschaltet** werden soll.

[Foto: Mac-Menü „Monitor-Umschaltung…“]
[Foto: Monitor-Bildschirmmenü mit DDC/CI]

---

## 10. Alexa

Mit Alexa kannst du sagen: **„Alexa, schalte Laptop an.“** Das CYD gibt sich im Heimnetz als
Lampe aus (Philips Hue, so sieht es Alexa). Alexa findet sie und löst das Wake-on-LAN aus. In der
Alexa-App taucht „Laptop“ deshalb als **Lampe** auf, „an“ und „aus“ funktionieren wie bei einer Lampe.

> **Getestet mit:** Echo Dot [Generation]

**Wichtig:** Alexa ist im Auslieferungszustand **aus**. Solange sie aus ist, läuft auf dem CYD kein
zusätzlicher Dienst, und es ist kein Port offen.

### Einschalten

1. Am CYD: **Menü (Seite 2, Kachel „Menü“) → Einrichtung → „Alexa“**.
2. Tippe auf **„Alexa einschalten“**. Die Taste ist gefüllt, wenn Alexa an ist.
3. **Name:** Tippe auf **„Name: …“**. Vorgabe ist **Laptop**. Erlaubt sind Buchstaben (ohne Umlaute),
   Ziffern, Leerzeichen und Bindestrich. Andere Zeichen werden entfernt.
4. Tippe auf **„Zurück“**. Hast du etwas geändert, **startet das CYD neu**. Das gehört dazu.
5. Der Name gehört zum **ersten Gerät** in „WoL-Geräte“. Das muss eingerichtet sein.

Die Hinweise im Menü (das kleine „?“ neben jedem Punkt) erklären alles kurz.

[Foto: Alexa-Menü am CYD]

### Geräte suchen

- Sage **„Alexa, suche neue Geräte.“** Das dauert etwa 20 Sekunden.
- Der Weg über die App (**Geräte → + → Gerät hinzufügen**) führt zu einer **Herstellerliste**. Dort findest du
  das iBoard nicht, nimm den Sprachbefehl. Klappt es nicht beim ersten Mal, sage ihn noch einmal und warte
  bis zu eine Minute, die Suche des Echo Dot braucht manchmal zwei Anläufe.
- Alexa meldet, dass sie das Gerät **„Laptop“** (oder deinen Namen) gefunden hat.

**Nach einer Namensänderung** musst du die Geräte **neu suchen**. Lösche vorher das alte Gerät in der
Alexa-App, sonst bleibt es doppelt stehen.

### Befehle

- **„Alexa, schalte Laptop an.“** weckt den Laptop (Wake-on-LAN, bei Bedarf mit Monitor-Umschaltung).
- **„Alexa, schalte Laptop aus.“** fährt den Laptop **nur dann** herunter, wenn du im Alexa-Menü
  **„Alexa: Ausschalten“** eingeschaltet hast (Vorgabe: aus). Dafür brauchst du eine eingetragene IP, den
  Windows-Listener und den Schlüssel (Kapitel 5 bis 8).

In der Alexa-App springt der Schalter nach einigen Sekunden von selbst wieder auf „aus“. Das ist Absicht:
So funktioniert „an“ jedes Mal, auch wenn der Laptop schon einmal an war.

### Wenn es nicht klappt

Siehe die Tabelle in Kapitel 11. Die häufigsten Gründe: Alexa-Gerät und CYD sind nicht im selben WLAN, die
Funktion ist am CYD noch aus, oder das CYD wurde nach der Änderung nicht neu gestartet.

---

## 11. Fehlersuche

| Symptom | Mögliche Ursache | Lösung |
|---|---|---|
| Das CYD wird vom Computer nicht erkannt | Kabel überträgt nur Strom, oder der USB-Treiber fehlt | Ein anderes (Daten-)Kabel nehmen. Den CH340-Treiber installieren (Kapitel 2) |
| „Keine Verbindung zum Mac“ | Die Mac-App läuft nicht | Prüfe, ob das Symbol in der Menüleiste da ist, und starte die App |
| „Keine Verbindung zum Mac“ | Mac und CYD sind in verschiedenen WLANs, oder ein Gastnetz | Beide ins selbe normale Heim-WLAN (2,4 GHz) |
| „Keine Verbindung zum Mac“ | Die Netzwerk-Erlaubnis in macOS fehlt | Systemeinstellungen → Datenschutz & Sicherheit → Lokales Netzwerk → „Eiswolfs iBoard“ erlauben |
| Das CYD verbindet sich nicht mit dem WLAN | Falsches Passwort, oder ein 5-GHz-Netz | Das Passwort steht im Klartext, genau prüfen (Groß- und Kleinschreibung). Ein 2,4-GHz-Netz benutzen |
| Das CYD zeigt „Keine SD-Karte“ | Karte fehlt oder ist nicht FAT32 | Eine FAT32-Karte einstecken |
| Lüfter-Kachel zeigt „--“ | Der Mac hat keinen Lüfter (zum Beispiel MacBook Air) | Kein Fehler |
| Aufwecken funktioniert nicht | Wake-on-LAN im BIOS oder in der Netzwerkkarte aus | Kapitel 7 noch einmal prüfen, besonders den Schnellstart |
| Aufwecken funktioniert nicht | Der Laptop hängt am WLAN | Per Netzwerkkabel verbinden |
| Aufwecken geht nur aus dem Ruhezustand | Manche Laptops wecken nur daraus, nicht aus dem Ausschalten | Mit vollständig ausgeschaltetem Laptop testen, wenn möglich Kabel nutzen |
| Ausschalten geht nicht, Aufwecken geht | Keine IP am CYD eingetragen (dann zeigt das Halten „Ausschalten nicht eingerichtet“) | IP bei „WoL-Geräte“ eintragen (Kapitel 6, 7) |
| Ausschalten ging, jetzt nicht mehr | Die IP des Laptops hat sich geändert | Feste IP im Router vergeben (Kapitel 6), am CYD korrigieren |
| Ausschalten, Lüfter-Boost oder Ruhezustand ohne Reaktion | Der Schlüssel ist nicht überall gleich | Zeichen für Zeichen vergleichen (Kapitel 5) |
| Das Protokoll des Listeners zeigt keine neue Zeile nach dem Halten | Der Befehl erreicht den Laptop nicht (Firewall, Netzwerk) | Firewall-Regel „Eiswolfs iBoard Listener“ prüfen, Netzwerkprofil „Privat“ (Kapitel 8) |
| Das Protokoll zeigt „ungültige Signatur“ | Schlüssel stimmt nicht überein, oder die Zähler sind aus dem Takt | Schlüssel prüfen. Ist er richtig, den Zähler-Abgleich (Resync) machen |
| Alle Befehle werden nach einem Zurücksetzen des CYD ignoriert | Der Zähler des CYD ist zurückgesetzt | Resync (Kapitel 5) |
| Alexa findet das Gerät nicht | Alexa ist am CYD noch aus | Menü → Einrichtung → Alexa → „Alexa einschalten“, zurück, das CYD startet neu |
| Alexa findet das Gerät nicht | Alexa-Gerät und CYD sind in verschiedenen WLANs | Beide ins selbe Heim-WLAN, kein Gastnetz |
| Alexa findet das Gerät nicht | Der Router blockiert die Suche im Heimnetz (Multicast) | Die Router-Einstellung für „Geräte im WLAN dürfen untereinander kommunizieren“ prüfen. Gerät neu suchen |
| Alexa findet nach der Namensänderung nichts Neues | Der alte Eintrag stört, oder es wurde nicht neu gesucht | Altes Gerät in der Alexa-App löschen, „Alexa, suche neue Geräte“ |
| „Alexa, schalte Laptop aus“ tut nichts | „Alexa: Ausschalten“ ist aus (Vorgabe) | Im Alexa-Menü einschalten, außerdem IP, Listener und Schlüssel nötig |
| Alexa sagt „Das Gerät reagiert nicht“ | Das CYD antwortet nur kurz, der Schalter springt zurück | Meist trotzdem erfolgreich. Prüfen, ob der Laptop aufwacht. Sonst neu suchen |
| Monitor schaltet nicht um | DDC/CI am Monitor aus, oder der Monitor/Adapter erlaubt es nicht | DDC/CI im Monitormenü einschalten. Mit „Test: auf HDMI 2 schalten“ in der Mac-App prüfen |
| Monitor schaltet auf den falschen Eingang | Der Zahlenwert des Eingangs weicht beim Monitor ab | In der Mac-App unter „Eingangswerte (VCP 0x60)“ anpassen |

---

## 12. Sicherheit

- **Das ist ein Heimnetz-Spielzeug, kein Sicherheitssystem.** Alles bleibt in deinem WLAN. Nichts geht
  ins Internet, die Daten kommen nur von deinen eigenen Geräten.
- **Der Schlüssel ist keine Verschlüsselung.** Befehle laufen als lesbarer Text durch dein WLAN. Der
  Schlüssel schützt davor, dass jemand Befehle **fälschen** oder einen alten Befehl **erneut abspielen**
  kann. Wer in deinem Netz mitliest, sieht aber, **dass** ein Befehl geschickt wird.
- **Aufwecken (Wake-on-LAN) hat keinen Schutz.** Es ist ein Standard-Netzwerksignal ohne Schlüssel. Jedes Gerät
  in deinem Heimnetz kann deinen Laptop wecken.
- **Alexa:**
  - Das Gerät, das das CYD vortäuscht, hat **keinen Schutz**. Es nutzt das einfache Philips-Hue-Verfahren ohne
    Anmeldung. Ist Alexa am CYD eingeschaltet, kann **jedes Gerät in deinem Heimnetz** „an“ und „aus“
    schicken.
  - Ist zusätzlich **„Alexa: Ausschalten“** an, kann damit **jedes Gerät in deinem Heimnetz** den Laptop
    **herunterfahren**, ohne den Schlüssel zu kennen. Schalte es nur ein, wenn du deinem Heimnetz
    vertraust.
  - Schalte Alexa aus, wenn du sie nicht brauchst. Dann läuft kein Dienst und es ist kein Port offen.
- **Dein Heim-WLAN braucht ein gutes Passwort.** Gäste gehören ins Gastnetz.
- **Gib deinen Schlüssel nicht weiter** und zeige ihn nicht auf Fotos. Hast du ihn versehentlich
  veröffentlicht, erzeuge einen neuen (Kapitel 5) und trage ihn überall ein.
- **Lüfter-Boost** braucht ein kleines Hilfsprogramm mit Administratorrechten (siehe
  `tools/fan-helper/README.md`). Das ist freiwillig und nur etwas für Fortgeschrittene.
- **Der Windows-Listener** fährt den Laptop herunter, wenn ein gültiger, signierter Befehl kommt. Er
  speichert den Schlüssel nur in deinem Benutzerordner (`%APPDATA%\EiswolfsIBoard\config.json`) und schreibt
  ihn nie ins Protokoll.
- Sicherheitslücken meldest du bitte privat, siehe `SECURITY.md`.

Mehr dazu: `docs/TOKEN.md` erklärt den Schlüssel und den Wiederholungsschutz genauer.
