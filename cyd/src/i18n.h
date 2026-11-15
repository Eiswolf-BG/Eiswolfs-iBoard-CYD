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

    // "n/v"-Anzeige (Auftrag "Controls-Screen") - wiederverwendet auf
    // allen drei interaktiven Controls-Kacheln (Acer/Mac/Luefter), wenn
    // ein Befehl nicht bestaetigt wird bzw. die Kachel unkonfiguriert/
    // nicht erreichbar ist.
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

    // Wake-on-LAN fuer den Windows-Laptop (Auftrag, main.cpp) - Button-
    // Label im Einrichtung-Untermenue, Tastatur-Titel bei der MAC-
    // Eingabe. Seit Auftrag "Controls-Screen" gibt es dafuer keine
    // eigene Text-Rueckmeldung mehr auf der Kachel, nur noch eine
    // Farbe (siehe CLAUDE.md) - die frueheren WOL_SENT_FEEDBACK/
    // SHUTDOWN_SENT_FEEDBACK-Eintraege wurden deshalb entfernt.
    // Seit Auftrag "WoL-Geraete": bis zu 4 Geraete statt nur dem Acer -
    // derselbe Eintrag dient weiterhin als Button-Label im Einrichtung-
    // Untermenue UND als Titel der neuen Geraeteliste-Seite (gleiches
    // Muster wie SETUP_BRIGHTNESS oben).
    SETUP_WOL_MAC,
    KB_WOL_MAC_PROMPT,

    // "?"-Erklaerungen im Einrichtung-Untermenue (Auftrag "erklaere die
    // Menues") - je ein kurzer, mehrzeiliger Infotext (manuelle "\n"-
    // Umbrueche, dieses Projekt hat keinen automatischen Wortumbruch-
    // Helfer wie die Schwesterprojekte). HELP_OK bewusst nicht
    // uebersetzt (identisch in beiden Sprachen).
    HELP_OK,
    SETUP_WIFI_HELP,
    SETUP_CAL_HELP,
    SETUP_BRIGHTNESS_HELP,
    SETUP_WOL_HELP,

    // Schutz gegen Befehlswiederholung (Auftrag "Sicherheit und
    // Lizenzen", Teil 2) - Resync-Menuepunkt + Bestaetigungstext.
    SETUP_RESYNC,
    SETUP_RESYNC_HELP,
    RESYNC_SENT_FEEDBACK,

    // Controls-Screen (Auftrag "Controls-Screen") - Titel oben, sowie der
    // Text im Luefter-Koerper, wenn der Boost im Modus "Bis Tippen" laeuft
    // (keine Restzeit bekannt, zeigt stattdessen diesen Text).
    CONTROLS_TITLE,
    FAN_BOOST_ON_INDEFINITE,

    // WoL-Geraeteliste (Auftrag "WoL-Geraete", main.cpp) - Seite 3
    // "Geraete", die Verwaltungsseite unter Einrichtung sowie das
    // Hinzufuegen/Bearbeiten/Loeschen eines Eintrags.
    DEVICES_PAGE_TITLE,
    WOL_ADD_DEVICE,
    WOL_NAME_PROMPT,
    WOL_SHUTDOWN_IP_PROMPT,
    WOL_INVALID_MAC,
    WOL_EMPTY_NAME,
    WOL_DELETE_CONFIRM_PREFIX,
    CONFIRM_YES,
    CONFIRM_NO,
    WOL_SHUTDOWN_NOT_SET_UP,

    // Auftrag "Monitor-Umschaltung" - neuer optionaler Schritt im WoL-
    // Geraet-Bearbeiten/Hinzufuegen-Dialog (main.cpp::
    // runWolDeviceEntryFlow()/runMonitorInputPicker()).
    MONITOR_INPUT_TITLE,
    MONITOR_INPUT_OFF,
    MONITOR_INPUT_HDMI1,
    MONITOR_INPUT_HDMI2,
    MONITOR_INPUT_DP1,
    MONITOR_INPUT_DP2,
    MONITOR_INPUT_HELP,

    // Auftrag "Alexa Aus" - neuer Ein/Aus-Schalter im Einrichtung-Menue
    // (runSetupMenu()) plus Hilfetext und Hinweis bei fehlender IP.
    ALEXA_SHUTDOWN_MENU,
    ALEXA_SHUTDOWN_HELP,
    ALEXA_SHUTDOWN_NO_IP,

    // Release v1.2.0 - eigenes Alexa-Menue unter Einrichtung (runAlexaMenu()):
    // Ein/Aus, Name, Hinweis, Neustart-Meldung. Texte mit "\n" von Hand
    // umgebrochen (max. ca. 33 Zeichen je Zeile, siehe showHelpPopup()).
    ALEXA_MENU_BUTTON,
    ALEXA_MENU_HELP,
    ALEXA_ENABLE_LABEL,
    ALEXA_ENABLE_HELP,
    ALEXA_NAME_LABEL,
    ALEXA_NAME_HELP,
    ALEXA_NAME_PROMPT,
    ALEXA_NAME_FIXED,
    ALEXA_HINT,
    ALEXA_RESTART_TEXT,

    COUNT
};

namespace I18n {
    // "de" oder "en" - alles andere faellt auf "de" zurueck.
    void setLanguage(const String& lang);
    String currentLanguage(); // "de"/"en", fuer die Menue-Anzeige
    const char* t(StringId id);
}
