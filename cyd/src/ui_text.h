#pragma once
// Zentrale Textbox-Funktion (Auftrag "Ueberschriften und WLAN-Zurueck")
// - von main.cpp UND setup_flow.cpp genutzt, deshalb ein eigenes,
// gemeinsames Modul statt Duplikat je Datei (anders als das
// Rect/drawButton-mini-Muster, das pro Screen bewusst dupliziert wird -
// hier geht es um dieselben Schriftobjekte/dieselbe Logik in BEIDEN
// Uebersetzungseinheiten, ein Duplikat wuerde den Font doppelt im Flash
// ablegen).
#include <Arduino.h>
#include <TFT_eSPI.h>

namespace UiText {

    struct Box {
        int16_t x, y, w, h;
    };

    struct Rect {
        int16_t x, y, w, h;
        bool contains(int16_t px, int16_t py) const {
            return px >= x && px < x + w && py >= y && py < y + h;
        }
    };

    enum class Align { Left, Center, Right };

    // Zeichnet Text vertikal mittig in die Box (ueber TFT_eSPIs eigene
    // MC/ML/MR_DATUM-Vertikalzentrierung - laut Flightradar-Projekt
    // bereits korrekt fuer GFXFF-Schriften, unabhaengig vom Baseline-
    // Anker, siehe dortige CLAUDE.md). Waehlt automatisch die groesste
    // erzeugte Schriftgroesse, die hinein passt (UiBoldSmall -> UiBoldS2
    // -> UiBoldS3), kuerzt mit "..." falls selbst die kleinste nicht
    // passt. Loescht NUR die eigene Box - mit "bg", das auch fuer die
    // Textfarbe als Hintergrund gesetzt wird (keine Kantenglaettung
    // gegen eine falsche Farbe, kein dunkler Saum).
    void draw(TFT_eSPI& tft, const Box& box, const String& text, uint16_t fg, uint16_t bg,
              Align align = Align::Center, int16_t paddingX = 4);

    // Einheitliche Kopfzeile (Schritt 2) - 34px hoch, Titel in der
    // Groesse der Controls-Ueberschrift (UiBold18, vertikal mittig).
    // showBack=true zeichnet zusaetzlich den Zurueck-Pfeil links (gleiche
    // Optik/Geometrie wie der bestehende Controls-Zurueck-Pfeil, siehe
    // main.cpp::drawCtrlBackButton() - bewusst eine EIGENE Funktion hier,
    // der Controls-Screen selbst bleibt unveraendert/unangetastet).
    // Inhalt soll ab UiText::CONTENT_Y beginnen.
    constexpr int16_t HEADER_H = 34;
    constexpr int16_t CONTENT_Y = 38;
    constexpr Rect BACK_TAP_RECT = { 0, 0, 80, 38 };

    void drawHeader(TFT_eSPI& tft, const char* title, bool showBack, bool backFlashed = false);

#ifdef LAYOUT_DEBUG
    // Schritt 6: jede draw()/drawHeader()-Box wird registriert und gegen
    // Bildschirmrand/eigene Box-Groesse/Ueberlappung mit anderen Boxen
    // DERSELBEN Seite geprueft, Verstoesse werden sofort per Serial
    // gemeldet. beginScreen() vor jedem neu aufgebauten Bildschirm
    // aufrufen (leert die Liste).
    void beginScreen(const char* screenName);
#endif
}
