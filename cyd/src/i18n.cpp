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
    "Testpunkt berühren",
    "Kalibrierung gespeichert",
    "Testpunkt verfehlt",

    "DEL",
    "SH",
    "#+-",
    "123",
    "SPACE",
    "Anzeigen",
    "Verbergen",
    "Zurück",
    "Weiter",
    "WLAN-Name eingeben",
    "Passwort fuer ",

    "Suche WLAN-Netzwerke...",
    "WLAN wählen",
    "(verborgen)",
    "Neu suchen",
    "Manuell eingeben",
    "Verbinde mit:",
    "Verbunden!",
    "Verbindung fehlgeschlagen",
    "Zurück zur Eingabe",

    "Sprache / Language",

    "Keine Verbindung",
    "zum Mac",
    "WLAN: ",
    "IP: ",
    "Status: ",
    "verbunden",
    "nicht verbunden",
    "Menü",

    "n/v",

    "Eiswolfs iBoard Menü",
    "Verbindung:",
    "Schrift:",
    "Auto",
    "WLAN",
    "USB",
    "Pixel",
    "Klar",
    "Lüfter",
    "Einrichtung",
    "Sprache: ",
    "Zurück",

    "Einrichtung",
    "Nur WLAN ändern",
    "Touch neu kalibrieren",

    "Lüfter-Boost",
    "30 Sek.",
    "1 Min.",
    "2 Min.",
    "5 Min.",
    "10 Min.",
    "Bis Tippen",
    "Vollgas",

    "Helligkeit",

    "WoL-Geräte",
    "MAC-Adresse eingeben",

    "OK",
    "Ändert nur das WLAN-Netzwerk.\nTouch-Kalibrierung bleibt\nunverändert erhalten.",
    "Startet die Touch-Kalibrierung\nneu (vier Eckpunkte antippen).\nWLAN bleibt unverändert.",
    "Stellt die Display-Helligkeit\nin 10%-Schritten ein (10-100%).",
    "Bis zu 4 Geräte verwalten:\nName/MAC ändern, per rotem X\nlöschen, unten hinzufügen.\nAntippen = wecken (WoL), langes\nHalten = ausschalten (falls dafür\neine IP eingetragen wurde).",

    "Zähler neu abgleichen",
    "Nur nötig, falls Lüfter-Boost/\nRuhemodus/Shutdown nach einem\nWerksreset nicht mehr reagieren.\nMac-App/Windows-Listener müssen\ndafür zuerst im jeweiligen Menü\nauf Empfang gestellt werden.",
    "Gesendet...",

    "Controls",
    "AN",

    "Geräte",
    "Gerät hinzufügen",
    "Name eingeben",
    "IP fürs Ausschalten (optional, Zurück = überspringen)",
    "Ungültige MAC-Adresse",
    "Name darf nicht leer sein",
    "Wirklich löschen: ",
    "Ja",
    "Nein",
    "Ausschalten nicht eingerichtet",

    "Monitor beim Einschalten",
    "Aus",
    "HDMI 1",
    "HDMI 2",
    "DisplayPort 1",
    "DisplayPort 2",
    "Schaltet den Monitor beim Einschalten dieses Geräts automatisch auf den gewählten Eingang um (per DDC/CI über die Mac-App). \"Aus\" lässt den Monitor unverändert.",

    "Alexa: Ausschalten",
    "Ein: \"Alexa, schalte [Name] aus\"\nfährt den Laptop herunter\n(Windows-Listener, inkl. Monitor-\nRückschaltung).\nAus (Standard): Befehl ohne\nWirkung. Nur einschalten, wenn\nAlexa ihn zuverlässig versteht.",
    "IP nötig zum Herunterfahren",

    "Alexa",
    "Alexa-Sprachsteuerung für das\nerste Gerät. Standard: Aus.\nEinschalten, Name wählen, dann in\nder Alexa-App Geräte suchen.\nÄnderung: iBoard startet neu.",
    "Alexa einschalten",
    "Ein: Das iBoard meldet sich im\nHeimnetz als Lampe, Alexa\nkann \"an\" und \"aus\" senden.\nAus (Standard): Es läuft nichts,\nkein Dienst, kein offener Port.\nÄnderung: iBoard startet neu.",
    "Name: ",
    "Unter diesem Namen findet Alexa\ndas Gerät, z. B. \"Laptop\".\nDann: \"Alexa, schalte Laptop an\".\nErlaubt: Buchstaben, Ziffern,\nLeerzeichen, Bindestrich.\nNach Änderung: Geräte neu suchen.",
    "Alexa-Name eingeben",
    "Nicht erlaubte Zeichen\nwurden entfernt.",
    "Danach in der Alexa-App:\nGeräte suchen",
    "Gespeichert.\nDas iBoard startet neu.",
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
    "to Mac",
    "WiFi: ",
    "IP: ",
    "Status: ",
    "connected",
    "not connected",
    "Menu",

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

    "WoL devices",
    "Enter MAC address",

    "OK",
    "Changes only the WiFi network.\nTouch calibration stays\nunchanged.",
    "Restarts touch calibration\n(tap the four corner points).\nWiFi stays unchanged.",
    "Adjusts display brightness\nin 10% steps (10-100%).",
    "Manage up to 4 devices:\nchange name/MAC, delete with\nthe red X, add one below. Tap\nto wake (WoL), long-press to\nshut down (if an IP was set).",

    "Resync counter",
    "Only needed if fan boost/sleep/\nshutdown stop responding after\na factory reset. The Mac app/\nWindows listener must first be\nset to listen in their own menu.",
    "Sent...",

    "Controls",
    "ON",

    "Devices",
    "Add device",
    "Enter name",
    "Shutdown IP (optional, Back = skip)",
    "Invalid MAC address",
    "Name cannot be empty",
    "Really delete: ",
    "Yes",
    "No",
    "Shutdown not set up",

    "Monitor on power-on",
    "Off",
    "HDMI 1",
    "HDMI 2",
    "DisplayPort 1",
    "DisplayPort 2",
    "Automatically switches the monitor to the selected input when this device powers on (via DDC/CI through the Mac app). \"Off\" leaves the monitor unchanged.",

    "Alexa: Shutdown",
    "On: \"Alexa, turn off [name]\"\nshuts the laptop down (via the\nWindows listener, incl. switching\nthe monitor back).\nOff (default): the command does\nnothing. Only enable once Alexa\nreliably understands it.",
    "IP needed to shut down",

    "Alexa",
    "Voice control via Alexa for the\nfirst device. Default: Off.\nTurn on, pick a name, then search\nfor devices in the Alexa app.\nChanges restart the iBoard.",
    "Enable Alexa",
    "On: the iBoard shows up in your\nhome network as a light and\nAlexa can send \"on\" and \"off\".\nOff (default): nothing runs, no\nservice and no open port.\nChanging it restarts the iBoard.",
    "Name: ",
    "Alexa finds the device under this\nname, e.g. \"Laptop\".\nSay: \"Alexa, turn on Laptop\".\nAllowed: letters, digits,\nspaces, hyphen.\nAfter a change: search again.",
    "Enter Alexa name",
    "Disallowed characters\nwere removed.",
    "Afterwards in the Alexa app:\nSearch for devices",
    "Saved.\nThe iBoard is restarting.",
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
