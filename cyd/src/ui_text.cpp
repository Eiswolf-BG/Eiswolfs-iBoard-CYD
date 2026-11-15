#include "ui_text.h"
#include "config.h"
#include "fonts/ui_bold_font.h"

namespace UiText {

namespace {

struct FontChoice {
    const GFXfont* font;
};

// Schrumpf-Reihenfolge fuer draw() - groesste zuerst. UiBoldSmall ist
// die Standardgroesse (ersetzt den frueheren eingebauten GLCD-Font
// ueberall), UiBoldS2/UiBoldS3 sind die beiden Schrumpf-Stufen (siehe
// tools/generate_ui_font.py), UiBoldS3 (Gesamthoehe 15px) ist die
// kleinste erzeugte Groesse - klar ueber der geforderten 11px-
// Untergrenze.
const FontChoice BOX_CANDIDATES[] = {
    { &UiBoldSmall },
    { &UiBoldS2 },
    { &UiBoldS3 },
};
constexpr int BOX_CANDIDATE_COUNT = sizeof(BOX_CANDIDATES) / sizeof(BOX_CANDIDATES[0]);

// Kopfzeilen-Titel: dieselbe Groesse, die Controls tatsaechlich benutzt
// (main.cpp::drawControlsStaticLayout() ruft pickClearFont(title, W-20,
// 20) - bei maxH=20 passt KEINE der drei CLEAR_CANDIDATES-Groessen
// [UiBold24 yAdvance=56, UiBold18=41, UiBold9=23] tatsaechlich in die
// Hoehe, pickClearFont() faellt deshalb immer auf die letzte Kandidatin
// UiBold9 zurueck - das ist also de facto die "Controls-Groesse", nicht
// UiBold18, siehe tft.fontHeight()/TFT_eSPI.cpp::fontHeight() [liest
// gfxFont->yAdvance] - geprueft, nicht geraten). Fuer sehr lange Titel
// (z.B. ein langer englischer Text neben dem Zurueck-Pfeil) faellt der
// Titel zusaetzlich auf die BOX_CANDIDATES-Schrumpfstufen zurueck.
const GFXfont* HEADER_TITLE_FONT = &UiBold9;

void applyBoxFont(TFT_eSPI& tft, const GFXfont* f) {
    tft.setFreeFont(f);
    tft.setTextSize(1);
}

// Kuerzt text so, dass "<gekuerzt>..." bei der aktuell aktiven Schrift
// in maxW passt. Wird erst versucht, NACHDEM alle Groessen erfolglos
// durchprobiert wurden (kuerzt bei der kleinsten erzeugten Groesse).
String truncateToFit(TFT_eSPI& tft, const String& text, int16_t maxW) {
    if (tft.textWidth(text) <= maxW) return text;
    const char* ellipsis = "...";
    int16_t ellipsisW = tft.textWidth(ellipsis);
    if (ellipsisW > maxW) return ""; // Box zu winzig - lieber nichts als ueberlaufen
    String shortened = text;
    while (shortened.length() > 0) {
        shortened.remove(shortened.length() - 1);
        if (tft.textWidth(shortened) + ellipsisW <= maxW) {
            return shortened + ellipsis;
        }
    }
    return ellipsis;
}

#ifdef LAYOUT_DEBUG
constexpr int MAX_DEBUG_BOXES = 48;
Box g_debugBoxes[MAX_DEBUG_BOXES];
int g_debugBoxCount = 0;
const char* g_debugScreenName = "";

void reportViolation(const char* what) {
    Serial.printf("[layout] Verstoss auf '%s': %s\n", g_debugScreenName, what);
}

void checkAndRegisterBox(const Box& box, int16_t renderedFontHeight) {
    char msg[96];
    if (box.x < 0 || box.y < 0 || box.x + box.w > Config::SCREEN_WIDTH || box.y + box.h > Config::SCREEN_HEIGHT) {
        snprintf(msg, sizeof(msg), "Box (%d,%d,%d,%d) ragt ueber den Bildschirmrand",
                 box.x, box.y, box.w, box.h);
        reportViolation(msg);
    }
    if (renderedFontHeight > 0 && renderedFontHeight < 11) {
        snprintf(msg, sizeof(msg), "Box (%d,%d,%d,%d): Schrift %dpx unter 11px",
                 box.x, box.y, box.w, box.h, renderedFontHeight);
        reportViolation(msg);
    }
    for (int i = 0; i < g_debugBoxCount; i++) {
        const Box& other = g_debugBoxes[i];
        bool overlap = box.x < other.x + other.w && box.x + box.w > other.x &&
                        box.y < other.y + other.h && box.y + box.h > other.y;
        if (overlap) {
            snprintf(msg, sizeof(msg), "Box (%d,%d,%d,%d) ueberlappt Box (%d,%d,%d,%d)",
                     box.x, box.y, box.w, box.h, other.x, other.y, other.w, other.h);
            reportViolation(msg);
        }
    }
    if (g_debugBoxCount < MAX_DEBUG_BOXES) g_debugBoxes[g_debugBoxCount++] = box;
}
#endif

} // namespace

void draw(TFT_eSPI& tft, const Box& box, const String& text, uint16_t fg, uint16_t bg,
          Align align, int16_t paddingX) {
    tft.fillRect(box.x, box.y, box.w, box.h, bg);
    tft.setTextColor(fg, bg);

    int16_t maxW = (int16_t)(box.w - 2 * paddingX);
    if (maxW < 0) maxW = 0;

    int chosen = BOX_CANDIDATE_COUNT - 1;
    for (int i = 0; i < BOX_CANDIDATE_COUNT; i++) {
        applyBoxFont(tft, BOX_CANDIDATES[i].font);
        if (tft.textWidth(text) <= maxW && tft.fontHeight() <= box.h) {
            chosen = i;
            break;
        }
    }
    applyBoxFont(tft, BOX_CANDIDATES[chosen].font);

    String toDraw = text;
    if (tft.textWidth(toDraw) > maxW) {
        toDraw = truncateToFit(tft, toDraw, maxW);
    }

    int16_t cy = (int16_t)(box.y + box.h / 2);
    switch (align) {
        case Align::Left:
            tft.setTextDatum(ML_DATUM);
            tft.drawString(toDraw, (int16_t)(box.x + paddingX), cy);
            break;
        case Align::Right:
            tft.setTextDatum(MR_DATUM);
            tft.drawString(toDraw, (int16_t)(box.x + box.w - paddingX), cy);
            break;
        case Align::Center:
        default:
            tft.setTextDatum(MC_DATUM);
            tft.drawString(toDraw, (int16_t)(box.x + box.w / 2), cy);
            break;
    }
    tft.setTextDatum(TL_DATUM);

#ifdef LAYOUT_DEBUG
    checkAndRegisterBox(box, tft.fontHeight());
#endif
}

void drawHeader(TFT_eSPI& tft, const char* title, bool showBack, bool backFlashed) {
    tft.fillRect(0, 0, Config::SCREEN_WIDTH, HEADER_H, TFT_BLACK);

    if (showBack) {
        // Gleiche Optik/Geometrie wie main.cpp::drawCtrlBackButton() -
        // bewusst eine eigene Funktion, der Controls-Screen bleibt
        // unangetastet (siehe Auftrag-Schutzregel).
        const int16_t bx = 8, by = 6, bw = 44, bh = 28, radius = 7;
        uint16_t bg = backFlashed ? TFT_WHITE : TFT_BLACK;
        uint16_t arrowColor = backFlashed ? TFT_BLACK : TFT_WHITE;
        tft.fillRoundRect(bx, by, bw, bh, radius, bg);
        tft.drawRoundRect(bx, by, bw, bh, radius, TFT_WHITE);
        int16_t tipX = (int16_t)(bx + 15), tipY = (int16_t)(by + bh / 2);
        int16_t armX = (int16_t)(bx + 29);
        tft.drawWideLine(tipX, tipY, armX, (int16_t)(tipY - 9), 3.0f, arrowColor, bg);
        tft.drawWideLine(tipX, tipY, armX, (int16_t)(tipY + 9), 3.0f, arrowColor, bg);
    }

    Box titleBox = showBack
        ? Box{ 88, 0, (int16_t)(Config::SCREEN_WIDTH - 88 - 4), HEADER_H }
        : Box{ 8, 0, (int16_t)(Config::SCREEN_WIDTH - 16), HEADER_H };

    tft.fillRect(titleBox.x, titleBox.y, titleBox.w, titleBox.h, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    applyBoxFont(tft, HEADER_TITLE_FONT);
    String toDraw = title;
    if (tft.textWidth(toDraw) > titleBox.w) {
        // Faellt auf die kleineren Schrumpfstufen zurueck, dann kuerzt.
        int chosen = BOX_CANDIDATE_COUNT - 1;
        for (int i = 0; i < BOX_CANDIDATE_COUNT; i++) {
            applyBoxFont(tft, BOX_CANDIDATES[i].font);
            if (tft.textWidth(toDraw) <= titleBox.w) { chosen = i; break; }
        }
        applyBoxFont(tft, BOX_CANDIDATES[chosen].font);
        if (tft.textWidth(toDraw) > titleBox.w) toDraw = truncateToFit(tft, toDraw, titleBox.w);
    }

    tft.setTextDatum(showBack ? ML_DATUM : MC_DATUM);
    int16_t tx = showBack ? titleBox.x : (int16_t)(titleBox.x + titleBox.w / 2);
    tft.drawString(toDraw, tx, (int16_t)(titleBox.y + titleBox.h / 2));
    tft.setTextDatum(TL_DATUM);

#ifdef LAYOUT_DEBUG
    checkAndRegisterBox(titleBox, tft.fontHeight());
#endif
}

#ifdef LAYOUT_DEBUG
void beginScreen(const char* screenName) {
    g_debugScreenName = screenName;
    g_debugBoxCount = 0;
}
#endif

}
