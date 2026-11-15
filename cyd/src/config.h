#pragma once
#include <Arduino.h>

// Pruefung ohne Alex (Auftrag "Ueberschriften und WLAN-Zurueck",
// Schritt 6) - beide im Normalbetrieb AUS, nur fuer einen lokalen
// Testflash einkommentieren.
//
// LAYOUT_DEBUG: jede ueber UiText::draw()/drawHeader() gezeichnete
// Textbox wird gemessen und gegen Bildschirmrand/eigene Box/
// Ueberlappung mit anderen Boxen DERSELBEN Seite sowie eine Schriftgroesse
// unter 11px geprueft - Verstoesse laufen sofort per Serial (siehe
// src/ui_text.cpp).
// #define LAYOUT_DEBUG 1
//
// SCREEN_TOUR: zeigt beim Start alle Bildschirme nacheinander in DE und
// EN mit Extremtexten (siehe main.cpp::runScreenTour()), 4s pro
// Bildschirm, Name im Serial-Log - statt des normalen Betriebs.
// #define SCREEN_TOUR 1
//
// Alexa (Release v1.2.0): Das fruehere Build-Flag ALEXA_TEST ist entfernt.
// Alexa ist im normalen Build enthalten, aber standardmaessig AUS (Menue
// Einrichtung > Alexa, gespeichert in config.json: "alexa_enabled"). Nur wenn
// eingeschaltet, gibt sich das CYD im Heimnetz als Philips-Hue-Lampe aus
// (fauxmoESP, SSDP+HTTP auf Port 80) - ausgeschaltet startet weder der
// Dienst noch SSDP noch Port 80, siehe main.cpp (Suche nach "alexaActive").

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
    // Eigener Port fuer den Shutdown-Befehl an den Windows-Laptop
    // (Auftrag "Windows-Laptop herunterfahren") - der dortige
    // Hintergrund-Listener (windows/) lauscht hier, komplett getrennt
    // vom Mac-Weg (Port 4211) und vom WoL-Magic-Packet-Port (9).
    constexpr uint16_t WIN_SHUTDOWN_UDP_PORT = 4212;
    // Hoechstzahl an WoL-Geraeten (Auftrag "WoL-Geraete") - Index 0 ist
    // der (ggf. aus der alten WoL-MAC migrierte) erste Eintrag.
    constexpr uint8_t MAX_WOL_DEVICES = 4;
    // Auftrag "Monitor-Umschaltung" - wie lange triggerWolForDevice()
    // in loop() (nicht blockierend) auf die monitor_ack-Bestaetigung
    // der Mac-App wartet, bevor das WoL-Paket trotzdem gesendet wird.
    constexpr uint32_t MONITOR_ACK_TIMEOUT_MS = 2000;
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
    constexpr const char* APP_VERSION = "v1.2";

    // Bootscreen (Auftrag "Bootscreen mit Apfel", 29.10.) - siehe
    // CLAUDE.md "Bootscreen" fuer Masse/Farben/Positionen im Detail.
    constexpr uint32_t BOOT_SCREEN_MS = 7000;
    constexpr uint32_t BOOT_PULSE_MS = 1500;
    constexpr uint32_t BOOT_TAP_LOCKOUT_MS = 300;

    // Wisch-Erkennung Hauptscreen<->Controls (Auftrag "Controls-Screen",
    // Werte gelockert durch Auftrag "Controls-Bedienung"). Resistiver
    // Touch prellt - diese Werte legen fest, ab wann eine Beruehrung als
    // Wischen statt als Antippen gilt (siehe TouchCalib::pollGesture()).
    // Zum Nachjustieren hier aendern.
    constexpr int16_t SWIPE_MIN_DX_PX = 50;   // Mindest-waagerechte Strecke (vorher 80)
    constexpr int16_t SWIPE_MAX_DY_PX = 60;   // Max. erlaubte senkrechte Abweichung (vorher 40)
    constexpr uint32_t SWIPE_MIN_HOLD_MS = 80; // Mindestdauer der Beruehrung
    // Zusaetzlich muss die waagerechte Strecke mindestens das 1,5-fache
    // der groessten senkrechten Abweichung sein (Auftrag "Controls-
    // Bedienung", Schritt 3) - verhindert, dass ein schraeger/kurzer
    // Wisch (der jetzt dank der gelockerten Werte oben leichter durch-
    // kommt) mit einem schnellen Antippen verwechselt wird.
    constexpr float SWIPE_MIN_HORIZONTAL_RATIO = 1.5f;
    // Wie bei den bestehenden Lang-Antippen-Griffen (FAN-Boost/Shutdown):
    // ein kurzer Aussetzer des resistiven Sensors darf eine Beruehrung
    // nicht vorzeitig als "Loslassen" werten.
    constexpr uint32_t TOUCH_RELEASE_DEBOUNCE_MS = 150;

    // Strengere Antippen-Erkennung (Auftrag "Controls-Bedienung", Schritt
    // 2) - verhindert, dass eine missglueckte Wischgeste (zu kurz/zu
    // schraeg, faellt unten durch die Wisch-Pruefung) faelschlich als
    // sauberes Antippen gewertet wird. TAP_MAX_MOVE_PX gilt fuer die
    // GROESSTE Bewegung waehrend der GANZEN Beruehrung (nicht nur Start/
    // Ende) - ueberschreitet sie das, verfaellt die Beruehrung komplett
    // (weder Kachel-Tap noch Wisch), egal wo sie endet.
    constexpr int16_t TAP_MAX_MOVE_PX = 10;
    // Dauergrenzen fuer ein gueltiges Antippen (Mac Mini/Luefter Mac/
    // Menue/Zurueck-Pfeil UND der kurze Tipp auf Acer) - zu kurz = eher
    // ein Sensor-Zucken, zu lang = eher ein (missglueckter) Wisch-/
    // Halte-Versuch.
    constexpr uint32_t TAP_MIN_HOLD_MS = 40;
    constexpr uint32_t TAP_MAX_HOLD_MS = 1500;

    // Feine Umrandung um jede Controls-Kachel (Auftrag "Umlaute und
    // Umrandung", Schritt A0) - eigene Konstanten, damit Alex Staerke/
    // Farbe aendern kann, ohne Code zu suchen.
    constexpr int16_t CTRL_OUTLINE_PX = 1;
    constexpr uint16_t CTRL_OUTLINE_COLOR = 0xFFFF; // reines Weiss

    // Alexa (Release v1.2.0): Standardname, unter dem Alexa das Geraet
    // findet (aenderbar im Menue Einrichtung > Alexa), und maximale
    // Namenslaenge. Erlaubt sind nur Buchstaben A-Z/a-z, Ziffern, Leerzeichen
    // und Bindestrich (Alexa-/XML-sicher, siehe main.cpp::sanitizeAlexaName()).
    constexpr const char* ALEXA_DEFAULT_NAME = "Laptop";
    constexpr uint8_t ALEXA_NAME_MAX_LEN = 24;
}
