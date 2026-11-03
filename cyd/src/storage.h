#pragma once
#include <Arduino.h>
#include "touch_calib.h"

namespace Storage {

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
