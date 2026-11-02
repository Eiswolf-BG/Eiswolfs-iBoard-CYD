#pragma once
#include <Arduino.h>

namespace Config {
    // Bildschirm (Querformat, siehe tft.setRotation(1) in main.cpp).
    constexpr int16_t SCREEN_WIDTH = 320;
    constexpr int16_t SCREEN_HEIGHT = 240;

    // Touch-Pins - identisch zum Flightradar-Projekt (nur gelesen, siehe
    // ../../CYD Flightradar/src/config.h), klassisches ESP32-2432S028.
    constexpr uint8_t TOUCH_CLK_PIN  = 25;
    constexpr uint8_t TOUCH_CS_PIN   = 33;
    constexpr uint8_t TOUCH_MOSI_PIN = 32;
    constexpr uint8_t TOUCH_MISO_PIN = 39;
    constexpr uint8_t TOUCH_IRQ_PIN  = 36;

    // SD-Karten-Pins (eigener SPI-Bus/HSPI) - identisch zum
    // Flightradar-Projekt (nur gelesen), ueberschneidet sich nicht mit
    // Display (VSPI: 12/13/14/15) oder Touch (25/32/33/39/36).
    constexpr uint8_t SD_SPI_CS_PIN   = 5;
    constexpr uint8_t SD_SPI_MOSI_PIN = 23;
    constexpr uint8_t SD_SPI_MISO_PIN = 19;
    constexpr uint8_t SD_SPI_CLK_PIN  = 18;

    constexpr const char* SD_CONFIG_DIR = "/iBoard";
    constexpr const char* SD_CONFIG_FILE = "/iBoard/config.json";

    constexpr uint16_t UDP_PORT = 4210;
    // Eigener Port fuer Luefter-Boost-Befehle vom CYD an die Mac-App
    // (Auftrag "Luefter-Boost") - bewusst NICHT der Datenport oben, die
    // Mac-App lauscht dort separat (siehe fan_boost.py::CydCommandListener).
    constexpr uint16_t FAN_CMD_UDP_PORT = 4211;
    // Langes Antippen der FAN-Kachel startet/beendet den Luefter-Boost.
    constexpr uint32_t FAN_LONGPRESS_MS = 500;
    // Wie lange das CYD nach einem gesendeten Boost-/Auto-Befehl auf die
    // Bestaetigung im Datenstrom (Feld "fan_boost") wartet, bevor es
    // kurz "n/v" zeigt (siehe Auftrag).
    constexpr uint32_t FAN_CMD_CONFIRM_TIMEOUT_MS = 3000;
    // Vorher 3000ms - loeste bei normalen, kurzen WLAN-Aussetzern
    // (z.B. Modem-Sleep-Intervall) faelschlich "Keine Verbindung" aus,
    // siehe CLAUDE.md "Stolperfallen: staendiges Wechseln". Umschalten
    // zurueck auf die Anzeige passiert weiterhin sofort beim naechsten
    // Paket, nicht erst nach einer Wartezeit.
    constexpr uint32_t NO_DATA_TIMEOUT_MS = 6000;
    constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 5000;
    constexpr int HISTORY_LEN = 60; // ca. 1 Minute bei 1 Paket/s

    // Haelt man den Touchscreen beim Einschalten so lange gedrueckt,
    // startet die Ersteinrichtung erzwungen (Notausgang bei schlechter
    // Kalibrierung).
    constexpr uint32_t BOOT_HOLD_SETUP_MS = 3000;

    // Ausfuehrliches Serial-Log (Paketeingang mit Zeitstempel/Abstand,
    // Reset-Grund) fuer die Fehlersuche bei "springt auf Keine
    // Verbindung" - im Normalbetrieb false, siehe CLAUDE.md.
    constexpr bool DEBUG_LOG = false;

    // Kopfzeilen-Layout (Auftrag "Watt-Kachel", 29.10.): false = neue,
    // groessere Watt-Kachel rechts in der Kopfzeile (Normalbetrieb).
    // true = altes, schmales 20px-Kopfzeilen-Layout (Watt-Text klein
    // oben rechts) - zum schnellen Zurueckwechseln bei Nichtgefallen,
    // siehe CLAUDE.md "Kopfzeilen-Layout".
    constexpr bool USE_OLD_HEADER = false;

    // Firmware-Version, angezeigt unten rechts im Bootscreen - einzige
    // Stelle im Code, die pro Aenderung gepflegt werden muss.
    constexpr const char* APP_VERSION = "v1.0";

    // Bootscreen (Auftrag "Bootscreen mit Apfel", 29.10.) - siehe
    // CLAUDE.md "Bootscreen" fuer Masse/Farben/Positionen im Detail.
    constexpr uint32_t BOOT_SCREEN_MS = 7000;
    constexpr uint32_t BOOT_PULSE_MS = 1500;
    constexpr uint32_t BOOT_TAP_LOCKOUT_MS = 300;
}
