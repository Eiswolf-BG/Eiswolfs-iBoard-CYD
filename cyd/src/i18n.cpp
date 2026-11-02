#include "i18n.h"

// Reihenfolge MUSS exakt zum StringId-Enum in i18n.h passen - geprueft
// per static_assert unten gegen StringId::COUNT.
static const char* const I18N_DE[] = {
    "Eiswolfs iBoard",

    "Kalibrierung",
    "Oben links",
    "Oben rechts",
    "Unten rechts",
    "Unten links",
    "Kalibrierung fehlgeschlagen",
    "Wird wiederholt...",
    "Testpunkt beruehren",
    "Kalibrierung gespeichert",
    "Testpunkt verfehlt",

    "DEL",
    "SH",
    "#+-",
    "123",
    "SPACE",
    "Anzeigen",
    "Verbergen",
    "Zurueck",
    "Weiter",
    "WLAN-Name eingeben",
    "Passwort fuer ",

    "Suche WLAN-Netzwerke...",
    "WLAN waehlen",
    "(verborgen)",
    "Neu suchen",
    "Manuell eingeben",
    "Verbinde mit:",
    "Verbunden!",
    "Verbindung fehlgeschlagen",
    "Zurueck zur Eingabe",

    "Sprache / Language",

    "Keine Verbindung",
    "zum Mac",
    "WLAN: ",
    "IP: ",
    "Status: ",
    "verbunden",
    "nicht verbunden",
    "Menue",

    "RUHE ...",
    "n/v",

    "Eiswolfs iBoard Menue",
    "Verbindung:",
    "Schrift:",
    "Auto",
    "WLAN",
    "USB",
    "Pixel",
    "Klar",
    "Luefter",
    "Einrichtung",
    "Sprache: ",
    "Zurueck",

    "Einrichtung",
    "Nur WLAN aendern",
    "Touch neu kalibrieren",

    "Luefter-Boost",
    "30 Sek.",
    "1 Min.",
    "2 Min.",
    "5 Min.",
    "10 Min.",
    "Bis Tippen",
    "Vollgas",

    "Helligkeit",
};

static const char* const I18N_EN[] = {
    "Eiswolfs iBoard",

    "Calibration",
    "Top left",
    "Top right",
    "Bottom right",
    "Bottom left",
    "Calibration failed",
    "Retrying...",
    "Touch the test point",
    "Calibration saved",
    "Test point missed",

    "DEL",
    "SH",
    "#+-",
    "123",
    "SPACE",
    "Show",
    "Hide",
    "Back",
    "Next",
    "Enter WiFi name",
    "Password for ",

    "Scanning for WiFi networks...",
    "Choose WiFi",
    "(hidden)",
    "Scan again",
    "Enter manually",
    "Connecting to:",
    "Connected!",
    "Connection failed",
    "Back to input",

    "Sprache / Language",

    "No connection",
    "to the Mac",
    "WiFi: ",
    "IP: ",
    "Status: ",
    "connected",
    "not connected",
    "Menu",

    "SLEEP...",
    "n/a",

    "Eiswolfs iBoard Menu",
    "Connection:",
    "Font:",
    "Auto",
    "WiFi",
    "USB",
    "Pixel",
    "Clear",
    "Fan",
    "Setup",
    "Language: ",
    "Back",

    "Setup",
    "Change WiFi only",
    "Recalibrate touch",

    "Fan Boost",
    "30 sec",
    "1 min",
    "2 min",
    "5 min",
    "10 min",
    "Until tapped",
    "Max speed",

    "Brightness",
};

static_assert(sizeof(I18N_DE) / sizeof(I18N_DE[0]) == (size_t)StringId::COUNT,
              "I18N_DE table size must match StringId::COUNT");
static_assert(sizeof(I18N_EN) / sizeof(I18N_EN[0]) == (size_t)StringId::COUNT,
              "I18N_EN table size must match StringId::COUNT");

namespace I18n {

namespace {
    bool useEnglish = false;
}

void setLanguage(const String& lang) {
    useEnglish = (lang == "en");
}

String currentLanguage() {
    return useEnglish ? "en" : "de";
}

const char* t(StringId id) {
    size_t idx = (size_t)id;
    if (idx >= (size_t)StringId::COUNT) return "";
    return useEnglish ? I18N_EN[idx] : I18N_DE[idx];
}

}
