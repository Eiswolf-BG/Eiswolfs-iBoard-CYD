# Eiswolfs iBoard – Installation

Eiswolfs iBoard schickt die Systemwerte deines Mac (CPU, GPU,
Temperaturen, Lüfter, Akku/Leistung) an ein kleines Display (CYD), das
sie live anzeigt. Diese Anleitung ist für Macs mit **Apple Silicon**
(M1 oder neuer). Version 1.1.

## 1. App installieren

1. Doppelklick auf **Eiswolfs-iBoard.dmg**, falls noch nicht geöffnet.
2. Im sich öffnenden Fenster die **App "Eiswolfs iBoard"** auf das
   Symbol **"Programme"** ziehen.
3. Das DMG-Fenster kann danach geschlossen werden (die DMG-Datei
   selbst kannst du behalten oder löschen – installiert ist die App
   jetzt in "Programme").

## 2. Ersten Start erlauben (Gatekeeper-Warnung)

Da die App nicht über ein kostenpflichtiges Apple-Entwicklerkonto
signiert ist, warnt macOS beim allerersten Start. Das ist normal:

1. **Nicht doppelklicken.** Stattdessen im Finder (Ordner "Programme")
   mit der **rechten Maustaste** (oder zwei Finger auf dem Trackpad)
   auf "Eiswolfs iBoard" klicken und **"Öffnen"** wählen.
2. Im erscheinenden Dialog nochmal auf **"Öffnen"** klicken.
3. Falls stattdessen nur eine Meldung ohne "Öffnen"-Knopf erscheint:
   **Systemeinstellungen → Datenschutz & Sicherheit** öffnen, dort
   ganz unten steht ein Hinweis auf "Eiswolfs iBoard" mit einem Knopf
   **"Trotzdem öffnen"**.
4. Das ist nur beim allerersten Start nötig – danach startet die App
   ganz normal per Doppelklick oder automatisch beim Login.

## 3. Netzwerk-Berechtigung erlauben

Beim ersten Start fragt macOS, ob Eiswolfs iBoard im **lokalen
Netzwerk** senden darf. Bitte **erlauben** – ohne diese Berechtigung
kommen keine Werte beim Display an.

## 4. Beim Login automatisch starten

Auf das Menüleisten-Symbol (zeigt standardmäßig die CPU-Last in
Prozent) klicken und **"Beim Login starten"** anhaken. Die App läuft
dann ab dem nächsten Login automatisch im Hintergrund – ganz ohne
Autostart-Ordner oder Skript von Hand einrichten.

### Anderen Wert in der Menüleiste anzeigen

Im Menü unter **"In Menüleiste anzeigen"** lässt sich auswählen, welcher
Wert im Symbol oben erscheint – zur Auswahl stehen CPU-Last, RAM,
Festplatte, CPU-Temperatur, GPU-Temperatur, GPU-Last, Leistung und
Lüfter. Die Auswahl bleibt auch nach einem Neustart der App erhalten.

### Lüfter-Boost (nur mit Zusatzprogramm)

Im Menü gibt es außerdem einen Punkt **"Lüfter-Boost"**, mit dem sich
der Lüfter des Mac kurzzeitig auf eine höhere Drehzahl bringen lässt.
Das ist ein Zusatz für fortgeschrittene Nutzer und braucht ein
separates, selbst installiertes Hilfsprogramm mit Administratorrechten
– ohne dieses Hilfsprogramm zeigt das Menü "nicht verfügbar" an, alle
anderen Funktionen der App sind davon nicht betroffen.

## 5. Das CYD-Display einrichten

Das CYD-Gerät braucht eine **microSD-Karte** (FAT32-formatiert) und
richtet sich beim allerersten Einschalten komplett selbst am Display
ein:

1. Touch-Kalibrierung: die vier angezeigten Kreuze antippen, danach
   den Testpunkt in der Mitte.
2. WLAN auswählen: bitte das **normale Heimnetz** nehmen, **nicht**
   ein Gastnetz (Gastnetze trennen Geräte oft voneinander ab, dann
   findet das Display den Mac nicht). Nur **2,4-GHz-Netze** werden
   unterstützt, kein 5-GHz.
3. WLAN-Passwort über die Bildschirmtastatur eingeben.

Mac und CYD müssen im **selben Netzwerk** sein.

## 6. Fehlerhilfe: "Keine Verbindung zum Mac"

Zeigt das Display dauerhaft "Keine Verbindung zum Mac", bitte der
Reihe nach prüfen:

- Läuft **Eiswolfs iBoard** auf dem Mac? (Menüleisten-Symbol sichtbar?)
- Sind Mac und CYD wirklich im **gleichen** WLAN (nicht Gastnetz,
  nicht ein zweites Netzwerk)?
- Wurde die Abfrage zur **Netzwerk-Berechtigung** (Schritt 3) erlaubt?
  Nachträglich prüfen/ändern: Systemeinstellungen → Datenschutz &
  Sicherheit → Lokales Netzwerk.
- Steckt das CYD per USB am Mac? Dann im Menü der App unter "Weg"
  nachsehen, ob "USB" erkannt wurde.

## Hinweis zu den angezeigten Werten

Nicht jeder Mac hat dieselben Sensoren. Auf Macs **ohne Lüfter** (z. B.
MacBook Air) zeigt die Lüfter-Kachel am Display dauerhaft "--" an –
das ist kein Fehler. Welche Werte genau verfügbar sind, hängt vom
Mac-Modell ab.

## App aktualisieren

Bei einer neuen Version:

1. Eiswolfs iBoard über das Menüleisten-Symbol **"Beenden"**.
2. Die neue App aus der DMG-Datei auf das **"Programme"**-Symbol ziehen
   und beim Nachfragen **ersetzen** bestätigen.
3. Eiswolfs iBoard aus "Programme" neu starten (beim ersten Start nach
   dem Update ggf. wieder Schritt 2 "Ersten Start erlauben" nötig, falls
   macOS die neue Version erneut als unbekannt einstuft).

Die gespeicherten Einstellungen (gewählte Menüleisten-Anzeige, "Beim
Login starten" usw.) bleiben beim Aktualisieren erhalten – sie liegen
in `~/Library/Application Support/Eiswolfs iBoard` und werden vom
Installieren/Ersetzen der App nicht berührt.

## App wieder entfernen

1. Im Menüleisten-Symbol **"Beim Login starten"** abhaken (falls
   gesetzt).
2. Im Menü **"Beenden"** wählen.
3. Die App aus dem Ordner "Programme" in den Papierkorb ziehen.
4. Optional, falls auch die gespeicherten Einstellungen gelöscht
   werden sollen: den Ordner
   `~/Library/Application Support/Eiswolfs iBoard` löschen (im Finder
   mit ⌘⇧G den Pfad eingeben).
