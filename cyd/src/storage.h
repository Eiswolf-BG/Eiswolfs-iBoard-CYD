#pragma once
#include <Arduino.h>
#include "touch_calib.h"
#include "config.h"

namespace Storage {

    // Ein WoL-Geraet (Auftrag "WoL-Geraete") - bis zu Config::MAX_WOL_
    // DEVICES Stueck, Reihenfolge = Anzeigereihenfolge (Index 0 =
    // Controls-Kachel, 1-3 = Seite "Geraete"/"Keine Verbindung"-Buttons
    // in umgekehrter Reihenfolge, siehe main.cpp).
    struct WolDevice {
        String name = "";
        String mac = "";
        uint16_t color = 0; // RGB565, bei Anlage einmalig zufaellig vergeben
        // Optional (Auftrag Schritt 6) - nur gesetzt, wenn Alex beim
        // Hinzufuegen/Bearbeiten eine IP eingetragen hat. Leer = Halten
        // zeigt "Ausschalten nicht eingerichtet", sendet nichts - AUSSER
        // legacyBroadcastShutdown ist gesetzt (siehe unten).
        String shutdownIp = "";
        // NUR bei der einmaligen Migration des alten Acer-Eintrags
        // (wolMacAddress/labelLaptop) gesetzt - erhaelt dessen heutiges
        // Verhalten (Shutdown-Befehl als Broadcast, keine IP noetig)
        // unveraendert. Ueber die normale "Hinzufuegen"-Seite angelegte
        // Geraete bekommen dieses Flag NIE automatisch (auch nicht, wenn
        // der Acer geloescht und mit derselben MAC neu angelegt wird -
        // das ist dann bewusst ein ganz normales neues Geraet).
        bool legacyBroadcastShutdown = false;
        // Auftrag "Monitor-Umschaltung" - additiv wie shutdownIp oben.
        // 0 = Aus (Standard, bisheriges Verhalten unveraendert), 1-4 =
        // logischer Eingang (1=HDMI1, 2=HDMI2, 3=DisplayPort1,
        // 4=DisplayPort2) - die Zuordnung zu den echten VCP-0x60-
        // Rohwerten liegt NUR in der Mac-App-Konfiguration, siehe
        // CLAUDE.md.
        uint8_t monitorInputOnPowerOn = 0;
    };

    struct DeviceConfig {
        String wifiSsid;
        String wifiPassword;
        String macIp; // leer = Broadcast
        TouchCalib::Calibration touchCal;
        // Standard "clear" fuer NEUE Geraete (Auftrag 29.10.) - gilt nur
        // fuer diesen frischen Struct-Default (kein config.json/NVS
        // vorhanden). Eine BESTEHENDE config.json mit gespeichertem
        // "font"-Feld ueberschreibt diesen Default beim Laden immer
        // (siehe storage.cpp::parseJson()/loadFromPrefs()) und bleibt
        // dadurch unveraendert - "pixel" bleibt "pixel".
        String font = "clear"; // "pixel" oder "clear" - additiv, siehe storage.cpp
        String source = "auto"; // "auto"/"wifi"/"usb" - additiv, siehe storage.cpp
        // Sprache (Auftrag "Sprache DE/EN") - additiv wie font/source,
        // "de"/"en". Fehlt das Feld in einer aelteren config.json, gilt
        // dieser Default.
        String language = "de";
        // Luefter-Boost (Auftrag "Luefter-Boost") - additiv wie font/source,
        // fehlt das Feld in einer aelteren config.json gelten diese
        // Standardwerte (identisch zum Mac-App-Default, siehe config.py).
        int fanBoostSec = 60; // 0 = "Bis Tippen"
        String fanBoostRpm = "MAX"; // "2000"/"3000"/"4000"/"MAX"
        // Helligkeit (Auftrag "Helligkeitsregler fuers Display") - additiv
        // wie oben, 10-100 (Prozent), Default 100 = wie bisher (volle
        // Helligkeit, unveraendertes Verhalten fuer bestehende Geraete).
        int brightness = 100;
        // Wake-on-LAN (Auftrag "Laptop per CYD wecken") - MAC-Adresse des
        // Windows-Laptops, Format "XX:XX:XX:XX:XX:XX", additiv wie oben.
        // Leer = Feature inaktiv (Antippen der CPU-Temp-Kachel tut dann
        // nichts). Bewusst NIE im Quellcode/secrets.h, da oeffentliches
        // Repo - nur hier, am Geraet eingegeben, auf SD/NVS gespeichert.
        String wolMacAddress = "";
        // Schutz gegen Befehlswiederholung (Auftrag "Sicherheit und
        // Lizenzen", Teil 2) - steigt bei JEDEM gesendeten Befehl
        // (Luefter-Boost/Ruhemodus/Shutdown, siehe cmd_auth.cpp), wird
        // sofort gespeichert. 0 = frisches Geraet/noch nie ein Befehl
        // gesendet - erster Wert ist dann 1.
        uint32_t cmdCounter = 0;
        // Controls-Screen (Auftrag "Controls-Screen") - frei aenderbare
        // Namen der beiden Geraete-Kacheln, additiv wie oben. Bestehende
        // Werte in einer vorhandenen config.json werden NIE ueberschrieben,
        // nur beim Fehlen des Feldes gilt dieser Standardwert. Neutraler
        // Platzhalter "Laptop" seit Auftrag "Modularer Listener" (vorher
        // "Acer Predator" - Alex' eigenes Geraet als Vorgabe fuer JEDEN
        // neuen Nutzer, nach Ruecksprache geaendert). Betrifft NUR neue,
        // noch nie konfigurierte Geraete - bereits gespeicherte Werte
        // (wie Alex' eigener "Acer Predator") bleiben unveraendert, da
        // dieser Standardwert nur beim FEHLEN des Feldes greift.
        String labelLaptop = "Laptop";
        String labelMac = "Mac Mini M4";
        // Auftrag "Umlaute und Umrandung" - additiv wie oben. Seit
        // Schritt A3 echter Umlaut im Standardwert (Schritt A0 nutzte
        // kurzzeitig "Luefter Mac" als ASCII-Platzhalter, bis die neue
        // Schrift eingebaut war) - bestehende, von Alex selbst
        // gespeicherte Werte bleiben davon unberuehrt.
        String labelFan = "Lüfter Mac";
        // WoL-Geraete (Auftrag "WoL-Geraete") - ersetzt wolMacAddress/
        // labelLaptop als eigentliche Datenquelle, sobald Alex einmal
        // etwas in den neuen Einstellungen aendert. Bis dahin bleiben
        // wolMacAddress/labelLaptop oben die alleinige Quelle - siehe
        // Storage::load()/die Migration dort. wolDeviceCount==0 und ein
        // leeres wolMacAddress zusammen heissen "kein Geraet eingerichtet".
        WolDevice wolDevices[Config::MAX_WOL_DEVICES];
        uint8_t wolDeviceCount = 0;
        // Auftrag "Alexa Aus" - additiv wie oben, Standard AUS (sicher
        // fuer neue/GitHub-Nutzer, die "Alexa, schalte X aus" noch nicht
        // bewusst eingerichtet haben). Gilt fuer Geraet 0 (wolDevices[0]).
        bool alexaShutdownEnabled = false;
        // Alexa im oeffentlichen Build (Release v1.2.0) - additiv wie oben.
        // Standard AUS: Dienst/SSDP/Port 80 starten nur, wenn eingeschaltet.
        // alexaName = Name des ersten WoL-Geraets fuer Alexa (Vorgabe
        // "Laptop"). Fehlen die Felder in einer aelteren config.json/NVS,
        // gelten diese Standardwerte - bestehende Werte bleiben unberuehrt.
        bool alexaEnabled = false;
        String alexaName = "Laptop";
        bool valid = false;
    };

    // true, wenn die SD-Karte beim Start gefunden und gemountet werden
    // konnte. false => Display zeigt "Keine SD-Karte", Einstellungen
    // landen dann nur in NVS (Preferences).
    bool sdAvailable();

    void begin();

    // Liest zuerst von der SD-Karte, bei Fehlschlag aus dem internen
    // Speicher (NVS). false, wenn nirgendwo eine gueltige Konfiguration
    // gefunden wurde (-> Ersteinrichtung muss laufen).
    bool load(DeviceConfig& out);

    // Schreibt auf die SD-Karte, wenn verfuegbar - sonst in den internen
    // Speicher (NVS) als Notloesung.
    bool save(const DeviceConfig& cfg);
}
