#pragma once
// Zentrale, zweisprachige Textliste (Auftrag "Sprache DE/EN") - jeder
// sichtbare Text auf dem Geraet bekommt hier eine feste ID. i18n.cpp
// haelt zwei Arrays (DE/EN) in EXAKT derselben Reihenfolge wie dieses
// Enum, geprueft per static_assert gegen StringId::COUNT. Neue Eintraege
// IMMER in BEIDEN Sprachdateien an derselben Position ergaenzen, sonst
// verschiebt sich die Zuordnung.
#include <Arduino.h>

enum class StringId : uint16_t {
    APP_NAME, // "Eiswolfs iBoard" - Eigenname, bleibt in beiden Sprachen gleich

    // Kalibrierung (setup_flow.cpp)
    CALIB_TITLE,
    CALIB_TOP_LEFT,
    CALIB_TOP_RIGHT,
    CALIB_BOTTOM_RIGHT,
    CALIB_BOTTOM_LEFT,
    CALIB_FAILED,
    CALIB_RETRYING,
    CALIB_TOUCH_TEST_POINT,
    CALIB_SAVED,
    CALIB_TEST_MISSED,

    // Bildschirmtastatur (setup_flow.cpp)
    KB_DEL,
    KB_SHIFT,
    KB_SYMBOLS,
    KB_LETTERS,
    KB_SPACE,
    KB_SHOW,
    KB_HIDE,
    KB_BACK,
    KB_NEXT,
    KB_WIFI_NAME_PROMPT,
    KB_WIFI_PASSWORD_PREFIX, // + SSID dahinter angehaengt

    // WLAN-Auswahl/Verbindungstest (setup_flow.cpp)
    WIFI_SCANNING,
    WIFI_CHOOSE,
    WIFI_HIDDEN,
    WIFI_RESCAN,
    WIFI_MANUAL_ENTRY,
    WIFI_CONNECTING_TO,
    WIFI_CONNECTED,
    WIFI_CONNECT_FAILED,
    WIFI_BACK_TO_INPUT,

    // Sprachauswahl (setup_flow.cpp, neuer erster Ersteinrichtungs-Schritt)
    LANGUAGE_TITLE,

    // Haupt-Statuszeile / "Keine Verbindung" (main.cpp)
    NO_CONNECTION_LINE1,
    NO_CONNECTION_LINE2,
    WIFI_LABEL_PREFIX,
    IP_LABEL_PREFIX,
    STATUS_LABEL_PREFIX,
    STATUS_CONNECTED,
    STATUS_NOT_CONNECTED,
    MENU_BUTTON,

    // Ruhemodus-Rueckmeldung (PWR-Kachel, main.cpp)
    PWR_SLEEP_FEEDBACK,
    PWR_NOT_AVAILABLE,

    // Haupt-Menue (main.cpp::runMainMenu())
    MENU_HEADER,
    MENU_CONNECTION_LABEL,
    MENU_FONT_LABEL,
    MENU_CONNECTION_AUTO,
    MENU_CONNECTION_WIFI,
    MENU_CONNECTION_USB,
    MENU_FONT_PIXEL,
    MENU_FONT_CLEAR,
    MENU_FAN_BUTTON,
    MENU_SETUP_BUTTON,
    MENU_LANGUAGE_LABEL, // + "DE"/"EN" dahinter angehaengt, siehe main.cpp
    MENU_BACK,

    // Einrichtung-Untermenue (main.cpp)
    SETUP_TITLE,
    SETUP_WIFI_ONLY,
    SETUP_RECALIBRATE,

    // Luefter-Boost-Menue (main.cpp)
    FAN_BOOST_TITLE,
    FAN_BOOST_30SEC,
    FAN_BOOST_1MIN,
    FAN_BOOST_2MIN,
    FAN_BOOST_5MIN,
    FAN_BOOST_10MIN,
    FAN_BOOST_UNTIL_TAP,
    FAN_BOOST_MAX_SPEED,

    // Helligkeitsregler (Auftrag "Helligkeitsregler fuers Display",
    // main.cpp::runBrightnessMenu()) - ein Eintrag dient als Button-Label
    // IM Einrichtung-Untermenue UND als Bildschirmtitel der Reglerseite.
    SETUP_BRIGHTNESS,

    COUNT
};

namespace I18n {
    // "de" oder "en" - alles andere faellt auf "de" zurueck.
    void setLanguage(const String& lang);
    String currentLanguage(); // "de"/"en", fuer die Menue-Anzeige
    const char* t(StringId id);
}
