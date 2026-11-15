#include "setup_flow.h"
#include "touch_calib.h"
#include "config.h"
#include "i18n.h"
#include "ui_text.h"
#include <WiFi.h>

namespace SetupFlow {

namespace {

struct Rect {
    int16_t x, y, w, h;
    bool contains(int16_t px, int16_t py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

// Ueber UiText::draw() (Auftrag "Ueberschriften und WLAN-Zurueck",
// schliesst zugleich die bisherige Luecke aus "Umlaute und Umrandung"
// Schritt A2/A3 - dieses ganze Modul haengte bis hierher noch am
// eingebauten, reinen ASCII-Font, siehe CLAUDE.md). Gleiches Muster wie
// main.cpp::drawButton().
void drawButton(TFT_eSPI& tft, const Rect& r, const char* label, uint16_t color, bool filled = false) {
    uint16_t bg = filled ? color : TFT_BLACK;
    uint16_t fg = filled ? TFT_BLACK : color;
    tft.fillRect(r.x, r.y, r.w, r.h, bg);
    if (!filled) tft.drawRect(r.x, r.y, r.w, r.h, color);
    UiText::Box box = { (int16_t)(r.x + 2), (int16_t)(r.y + 2), (int16_t)(r.w - 4), (int16_t)(r.h - 4) };
    UiText::draw(tft, box, label, fg, bg, UiText::Align::Center, 2);
}

TouchCalib::Point blockingTap(TFT_eSPI& tft) {
    TouchCalib::Point p;
    while (!TouchCalib::wasTapped(p)) {
        delay(10);
    }
    return p;
}

// ---------------------------------------------------------------------
// Sprachauswahl (Auftrag "Sprache DE/EN") - allererster Ersteinrichtungs-
// Schritt, VOR der Kalibrierung, damit der Rest der Einrichtung schon in
// der gewaehlten Sprache laeuft. Bewusst mit festen, zweisprachigen
// Button-Beschriftungen ("Deutsch"/"English") statt ueber I18n::t() -
// die Sprache steht ja erst NACH diesem Tap fest.
void runLanguageFlow(TFT_eSPI& tft, Storage::DeviceConfig& cfg) {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;

    tft.fillScreen(TFT_BLACK);
    UiText::draw(tft, { 10, (int16_t)(H / 2 - 66), (int16_t)(W - 20), 26 },
                 "Sprache / Language", TFT_WHITE, TFT_BLACK);

    Rect deBtn = { (int16_t)(W / 2 - 110), (int16_t)(H / 2 - 20), 100, 40 };
    Rect enBtn = { (int16_t)(W / 2 + 10), (int16_t)(H / 2 - 20), 100, 40 };
    drawButton(tft, deBtn, "Deutsch", TFT_CYAN);
    drawButton(tft, enBtn, "English", TFT_CYAN);

    while (true) {
        TouchCalib::Point tap = blockingTap(tft);
        if (deBtn.contains(tap.x, tap.y)) {
            cfg.language = "de";
            I18n::setLanguage("de");
            return;
        }
        if (enBtn.contains(tap.x, tap.y)) {
            cfg.language = "en";
            I18n::setLanguage("en");
            return;
        }
    }
}

// ---------------------------------------------------------------------
// Touch-Kalibrierung
// ---------------------------------------------------------------------

struct RawAvg {
    long sumX = 0, sumY = 0;
    uint16_t n = 0;
    void add(int16_t x, int16_t y) { sumX += x; sumY += y; n++; }
    int16_t avgX() const { return n ? (int16_t)(sumX / n) : 0; }
    int16_t avgY() const { return n ? (int16_t)(sumY / n) : 0; }
};

RawAvg waitForRawTap() {
    RawAvg avg;
    while (!TouchCalib::rawPoint().touched) delay(10);
    uint32_t start = millis();
    while (millis() - start < 250) {
        TouchCalib::Point p = TouchCalib::rawPoint();
        if (p.touched) avg.add(p.x, p.y);
        delay(10);
    }
    while (TouchCalib::rawPoint().touched) delay(10);
    delay(150);
    return avg;
}

void drawTarget(TFT_eSPI& tft, int16_t x, int16_t y) {
    tft.drawCircle(x, y, 14, TFT_DARKGREY);
    tft.drawFastHLine(x - 16, y, 32, TFT_GREEN);
    tft.drawFastVLine(x, y - 16, 32, TFT_GREEN);
    tft.fillCircle(x, y, 3, TFT_GREEN);
}

TouchCalib::Calibration runCalibrationFlow(TFT_eSPI& tft) {
    const int16_t M = 20;
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;

    TouchCalib::Point targets[4] = {
        { M,     M,     true },
        { W - M, M,     true },
        { W - M, H - M, true },
        { M,     H - M, true },
    };
    const char* labels[4] = {
        I18n::t(StringId::CALIB_TOP_LEFT), I18n::t(StringId::CALIB_TOP_RIGHT),
        I18n::t(StringId::CALIB_BOTTOM_RIGHT), I18n::t(StringId::CALIB_BOTTOM_LEFT)
    };

    while (true) {
        RawAvg samples[4];
        for (int i = 0; i < 4; i++) {
            tft.fillScreen(TFT_BLACK);
            UiText::draw(tft, { 4, 0, (int16_t)(W - 8), 20 }, I18n::t(StringId::APP_NAME),
                         TFT_WHITE, TFT_BLACK, UiText::Align::Left);
            UiText::draw(tft, { 10, (int16_t)(H / 2 - 44), (int16_t)(W - 20), 24 },
                         I18n::t(StringId::CALIB_TITLE), TFT_WHITE, TFT_BLACK);
            UiText::draw(tft, { 10, (int16_t)(H / 2 - 22), (int16_t)(W - 20), 20 },
                         labels[i], TFT_WHITE, TFT_BLACK);
            drawTarget(tft, targets[i].x, targets[i].y);
            RawAvg a = waitForRawTap();
            samples[i] = a;
        }

        TouchCalib::Point raws[4];
        for (int i = 0; i < 4; i++) {
            raws[i] = { samples[i].avgX(), samples[i].avgY(), true };
        }

        TouchCalib::Calibration fitted = TouchCalib::fit(targets, raws);
        if (!fitted.valid) {
            tft.fillScreen(TFT_BLACK);
            UiText::draw(tft, { 10, (int16_t)(H / 2 - 24), (int16_t)(W - 20), 24 },
                         I18n::t(StringId::CALIB_FAILED), TFT_RED, TFT_BLACK);
            UiText::draw(tft, { 10, (int16_t)(H / 2 + 4), (int16_t)(W - 20), 20 },
                         I18n::t(StringId::CALIB_RETRYING), TFT_RED, TFT_BLACK);
            delay(1500);
            continue;
        }

        TouchCalib::setCalibration(fitted);

        tft.fillScreen(TFT_BLACK);
        UiText::draw(tft, { 10, (int16_t)(H / 2 - 44), (int16_t)(W - 20), 24 },
                     I18n::t(StringId::CALIB_TOUCH_TEST_POINT), TFT_WHITE, TFT_BLACK);
        drawTarget(tft, W / 2, H / 2);
        TouchCalib::Point tap = blockingTap(tft);

        int16_t dx = tap.x - W / 2;
        int16_t dy = tap.y - H / 2;
        bool ok = (dx * dx + dy * dy) <= (40 * 40);

        tft.fillScreen(TFT_BLACK);
        if (ok) {
            UiText::draw(tft, { 10, (int16_t)(H / 2 - 13), (int16_t)(W - 20), 26 },
                         I18n::t(StringId::CALIB_SAVED), TFT_GREEN, TFT_BLACK);
            delay(1000);
            return fitted;
        } else {
            UiText::draw(tft, { 10, (int16_t)(H / 2 - 22), (int16_t)(W - 20), 20 },
                         I18n::t(StringId::CALIB_TEST_MISSED), TFT_RED, TFT_BLACK);
            UiText::draw(tft, { 10, (int16_t)(H / 2 + 2), (int16_t)(W - 20), 20 },
                         I18n::t(StringId::CALIB_RETRYING), TFT_RED, TFT_BLACK);
            delay(1500);
            // continue -> von vorne mit den 4 Ecken
        }
    }
}

// ---------------------------------------------------------------------
// Bildschirmtastatur
// ---------------------------------------------------------------------

struct KeyRow {
    const char* lowerKeys[12];
    const char* upperKeys[12];
    int count;
};

// Reihen: Ziffern/Symbole umschaltbar, Buchstaben mit Shift.
// ":" fest mit auf der Ziffernreihe (Auftrag "Wake-on-LAN") - MAC-
// Adressen brauchen den Doppelpunkt, der sonst auf keiner Ebene der
// Tastatur vorkommt. "." dazu (Auftrag "WoL-Geraete"/"Modularer
// Listener", von Alex live als fehlend gemeldet, 10.11. - die
// optionale Shutdown-IP braucht Punkte, die es auf KEINER Ebene gab).
// Zwoelf statt zehn Spalten, eigene Spaltenbreite (colW0), unabhaengig
// von den anderen Reihen (weiterhin 10 Spalten).
const char* ROW_DIGITS[] = { "1","2","3","4","5","6","7","8","9","0",":","." };
const char* ROW_SYMBOLS[] = { "!","@","#","$","%","^","&","*","-","_",":","." };
const char* ROW1_LOWER[] = { "q","w","e","r","t","y","u","i","o","p" };
const char* ROW1_UPPER[] = { "Q","W","E","R","T","Y","U","I","O","P" };
const char* ROW2_LOWER[] = { "a","s","d","f","g","h","j","k","l" };
const char* ROW2_UPPER[] = { "A","S","D","F","G","H","J","K","L" };
const char* ROW3_LOWER[] = { "z","x","c","v","b","n","m" };
const char* ROW3_UPPER[] = { "Z","X","C","V","B","N","M" };

// Umlaut-Ebene (Auftrag "Ueberschriften und WLAN-Zurueck", Schritt 3d) -
// bisher waren AE/OE/UE/ss auf der Tastatur ueberhaupt nicht eingebbar
// (geprueft, nicht geraten: Reihe 1-3 zeigten bei symbols==true
// unveraendert dieselben Buchstaben wie sonst, nur Reihe 0 schaltete
// um - der "#+-"-Umschalter hatte auf Reihe 1 also gar keine Wirkung).
// Ersetzt deshalb Reihe 1 (10 Plaetze, 7 gebraucht + 3 ungenutzt leer)
// bei symbols==true durch die sieben Umlaute - gleiche Tastengroesse
// wie alle anderen Buchstaben, kein eigenes, kleineres UI-Element.
const char* ROW1_UMLAUT[] = { "ä","ö","ü","Ä","Ö","Ü","ß","","","" };

// "initialText" (Auftrag "Modularer Listener", von Alex live gewuenscht,
// 10.11.: "wenn ich auf einen existierenden Namen tippe, moechte ich
// nur etwas aendern... nicht jedesmal alles neu eingeben") - vorbelegt
// das Feld beim Oeffnen, Tippfehler lassen sich dann per Ruecktaste
// korrigieren statt alles neu einzutippen. Leerer String (Vorgabe,
// alle bisherigen Aufrufer unveraendert) = wie bisher leer beginnen.
String runKeyboard(TFT_eSPI& tft, const char* title, bool isPassword, bool& cancelled, const char* initialText = "") {
    const int16_t W = Config::SCREEN_WIDTH;
    String text = initialText;
    // Grossbuchstabe nur am Anfang (Release v1.2.0, Wunsch von Alex: "nach dem
    // ersten Buchstaben automatisch klein weiterschreiben"): Bei leerem Feld
    // startet die Umschaltung AN (erster Buchstabe gross), springt nach dem
    // ersten getippten Buchstaben automatisch AUS (shiftAuto). Wer die
    // Umschaltung selbst antippt, hat danach wie bisher eine dauerhafte
    // Grossschreibung bis zum naechsten Antippen. Wird das Feld per
    // Ruecktaste wieder ganz geleert, gilt der Anfangszustand erneut. Mit
    // vorbelegtem Text (Bearbeiten) startet die Umschaltung AUS.
    bool shiftAuto = (text.length() == 0);
    bool shift = shiftAuto;
    auto afterLetter = [&]() {
        if (shiftAuto) {
            shift = false;
            shiftAuto = false;
        }
    };
    bool symbols = false;
    // Passwort gleich im Klartext (Release v1.2.0, Wunsch von Alex: auf dem
    // kleinen Bildschirm ist die Eingabe so schon schwer genug). "Verbergen"
    // schaltet auf Sterne um.
    bool showPassword = true;
    cancelled = false;

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
#ifdef LAYOUT_DEBUG
        UiText::beginScreen("Tastatur");
#endif
        // Hinweistext ueber dem Feld: vorher per setCursor()+print()
        // ungeprueft an feste Position gezeichnet, lief bei laengeren
        // Titeln (z.B. "Passwort fuer <langer WLAN-Name>") rechts aus
        // dem Bild - UiText::draw() kuerzt jetzt automatisch mit "...",
        // wenn kein Platz ist.
        UiText::draw(tft, { 4, 0, (int16_t)(W - 8), 14 }, title, TFT_WHITE, TFT_BLACK, UiText::Align::Left);

        tft.drawRect(4, 16, W - 8, 20, TFT_DARKGREY);
        String shown;
        if (isPassword && !showPassword) {
            for (uint16_t i = 0; i < text.length(); i++) shown += "*";
        } else {
            shown = text;
        }
        UiText::draw(tft, { 8, 16, (int16_t)(W - 16), 20 }, shown, TFT_WHITE, TFT_BLACK, UiText::Align::Left);

        int16_t keyH = 28;
        int16_t y = 40;

        // Reihe 0: Ziffern/Symbole + feste ":"/"."-Knoepfe (Auftrag
        // "Wake-on-LAN"/"Modularer Listener") - 12 statt 10 Spalten,
        // eigene Breite (colW0).
        const char** row0 = symbols ? ROW_SYMBOLS : ROW_DIGITS;
        int16_t colW0 = W / 12;
        int16_t colW = W / 10;
        for (int i = 0; i < 12; i++) {
            drawButton(tft, { (int16_t)(i * colW0), y, colW0, keyH }, row0[i], TFT_CYAN);
        }
        y += keyH;

        // Reihe 1: normale Buchstaben, bei symbols==true die Umlaute
        // (Auftrag "Ueberschriften und WLAN-Zurueck", Schritt 3d).
        const char** row1 = symbols ? ROW1_UMLAUT : (shift ? ROW1_UPPER : ROW1_LOWER);
        for (int i = 0; i < 10; i++) {
            drawButton(tft, { (int16_t)(i * colW), y, colW, keyH }, row1[i], TFT_CYAN);
        }
        y += keyH;

        // Reihe 2 (9 Buchstaben) + Backspace rechts
        const char** row2 = shift ? ROW2_UPPER : ROW2_LOWER;
        int16_t col2W = (W - 48) / 9;
        for (int i = 0; i < 9; i++) {
            drawButton(tft, { (int16_t)(i * col2W), y, col2W, keyH }, row2[i], TFT_CYAN);
        }
        drawButton(tft, { (int16_t)(W - 48), y, 48, keyH }, I18n::t(StringId::KB_DEL), TFT_RED);
        y += keyH;

        // Reihe 3: Shift, Buchstaben, Symbole-Taste
        const char** row3 = shift ? ROW3_UPPER : ROW3_LOWER;
        drawButton(tft, { 0, y, 36, keyH }, I18n::t(StringId::KB_SHIFT), TFT_YELLOW, shift);
        int16_t col3W = (W - 36 - 50) / 7;
        for (int i = 0; i < 7; i++) {
            drawButton(tft, { (int16_t)(36 + i * col3W), y, col3W, keyH }, row3[i], TFT_CYAN);
        }
        drawButton(tft, { (int16_t)(W - 50), y, 50, keyH }, symbols ? I18n::t(StringId::KB_LETTERS) : I18n::t(StringId::KB_SYMBOLS), TFT_YELLOW, symbols);
        y += keyH;

        // Reihe 4: Leertaste, Anzeigen (nur Passwort), Zurueck, Weiter
        int16_t spaceW = isPassword ? 100 : 150;
        drawButton(tft, { 0, y, spaceW, keyH }, I18n::t(StringId::KB_SPACE), TFT_CYAN);
        int16_t nextX = spaceW;
        if (isPassword) {
            drawButton(tft, { nextX, y, 80, keyH }, showPassword ? I18n::t(StringId::KB_HIDE) : I18n::t(StringId::KB_SHOW), TFT_YELLOW, showPassword);
            nextX += 80;
        }
        int16_t remaining = W - nextX;
        int16_t backW = remaining / 2;
        drawButton(tft, { nextX, y, backW, keyH }, I18n::t(StringId::KB_BACK), TFT_ORANGE);
        drawButton(tft, { (int16_t)(nextX + backW), y, (int16_t)(remaining - backW), keyH }, I18n::t(StringId::KB_NEXT), TFT_GREEN);
    };

    render();

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            delay(10);
            continue;
        }

        int16_t keyH = 28;
        int16_t colW0 = W / 12;
        int16_t colW = W / 10;
        int16_t y0 = 40, y1 = y0 + keyH, y2 = y1 + keyH, y3 = y2 + keyH, y4 = y3 + keyH;

        bool redraw = true;

        if (tap.y >= y0 && tap.y < y1) {
            const char** row0 = symbols ? ROW_SYMBOLS : ROW_DIGITS;
            int idx = tap.x / colW0;
            if (idx >= 0 && idx < 12) text += row0[idx];
        } else if (tap.y >= y1 && tap.y < y2) {
            const char** row1 = symbols ? ROW1_UMLAUT : (shift ? ROW1_UPPER : ROW1_LOWER);
            int idx = tap.x / colW;
            if (idx >= 0 && idx < 10) { text += row1[idx]; afterLetter(); }
        } else if (tap.y >= y2 && tap.y < y3) {
            if (tap.x >= W - 48) {
                if (text.length() > 0) text.remove(text.length() - 1);
                if (text.length() == 0) { shift = true; shiftAuto = true; }
            } else {
                const char** row2 = shift ? ROW2_UPPER : ROW2_LOWER;
                int16_t col2W = (W - 48) / 9;
                int idx = tap.x / col2W;
                if (idx >= 0 && idx < 9) { text += row2[idx]; afterLetter(); }
            }
        } else if (tap.y >= y3 && tap.y < y4) {
            if (tap.x < 36) {
                shift = !shift;
                shiftAuto = false; // von Hand umgeschaltet: bleibt, bis erneut angetippt
            } else if (tap.x >= W - 50) {
                symbols = !symbols;
            } else {
                const char** row3 = shift ? ROW3_UPPER : ROW3_LOWER;
                int16_t col3W = (W - 36 - 50) / 7;
                int idx = (tap.x - 36) / col3W;
                if (idx >= 0 && idx < 7) { text += row3[idx]; afterLetter(); }
            }
        } else if (tap.y >= y4) {
            int16_t spaceW = isPassword ? 100 : 150;
            if (tap.x < spaceW) {
                text += " ";
            } else {
                int16_t nextX = spaceW;
                if (isPassword) {
                    if (tap.x < nextX + 80) {
                        showPassword = !showPassword;
                        goto doRedraw;
                    }
                    nextX += 80;
                }
                int16_t remaining = W - nextX;
                int16_t backW = remaining / 2;
                if (tap.x < nextX + backW) {
                    cancelled = true;
                    return text;
                } else {
                    return text;
                }
            }
        } else {
            redraw = false;
        }

    doRedraw:
        if (redraw) render();
    }
}

// ---------------------------------------------------------------------
// WLAN-Scan und Auswahl
// ---------------------------------------------------------------------

// Zurueck-Pfeil (Auftrag "Ueberschriften und WLAN-Zurueck", Schritt 4) -
// gleiche Optik/Geometrie wie main.cpp::drawCtrlBackButton() (Controls-
// Screen), hier ueber UiText::drawHeader() gezeichnet. Nur sichtbar,
// wenn allowBack true ist (siehe runWifiFlow()/runWifiOnly() unten fuer
// die genaue Regel, wann das der Fall ist).
String runWifiScan(TFT_eSPI& tft, bool allowBack, bool& wentBack) {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;
    wentBack = false;

    while (true) {
        tft.fillScreen(TFT_BLACK);
        UiText::draw(tft, { 10, (int16_t)(H / 2 - 13), (int16_t)(W - 20), 26 },
                     I18n::t(StringId::WIFI_SCANNING), TFT_WHITE, TFT_BLACK);

        int n = WiFi.scanNetworks();

        tft.fillScreen(TFT_BLACK);
#ifdef LAYOUT_DEBUG
        UiText::beginScreen("WlanListe");
#endif
        UiText::drawHeader(tft, I18n::t(StringId::WIFI_CHOOSE), allowBack);

        // Schritt 3c: 34px Kopfzeile + bis zu 6 Zeilen a 28px + 34px
        // Bodenleiste = 34+168+34 = 236px, passt in die 240px Hoehe.
        const int16_t rowH = 28;
        const int16_t listTop = UiText::CONTENT_Y;
        int visible = min(n, 6);
        // Feste rechte Spalte fuer die Signalstaerke (z.B. "-82dBm"),
        // der Name links bekommt den Rest und wird bei Bedarf gekuerzt.
        const int16_t rssiColW = 64;

        for (int i = 0; i < visible; i++) {
            int16_t ry = (int16_t)(listTop + i * rowH);
            tft.drawFastHLine(0, ry, W, TFT_DARKGREY);
            String name = WiFi.SSID(i);
            if (name.length() == 0) name = I18n::t(StringId::WIFI_HIDDEN);
            String rssi = String(WiFi.RSSI(i)) + "dBm";
            UiText::draw(tft, { 8, (int16_t)(ry + 1), (int16_t)(W - 16 - rssiColW), (int16_t)(rowH - 2) },
                         name, TFT_WHITE, TFT_BLACK, UiText::Align::Left);
            UiText::draw(tft, { (int16_t)(W - 8 - rssiColW), (int16_t)(ry + 1), rssiColW, (int16_t)(rowH - 2) },
                         rssi, TFT_DARKGREY, TFT_BLACK, UiText::Align::Right);
        }

        // Bodenleiste: zwei gleich breite Tasten, je eigener Rahmen
        // (Auftrag Schritt 3c) - vorher lief "Manuell eingeben" rechts
        // aus dem Bild, weil beide Tasten als EIN volltonig gefuelltes
        // drawButton() ohne Breitenpruefung gezeichnet wurden.
        int16_t bottomY = (int16_t)(listTop + visible * rowH + 2);
        Rect rescanBtn = { 0, bottomY, (int16_t)(W / 2), 30 };
        Rect manualBtn = { (int16_t)(W / 2), bottomY, (int16_t)(W - W / 2), 30 };
        drawButton(tft, rescanBtn, I18n::t(StringId::WIFI_RESCAN), TFT_YELLOW);
        drawButton(tft, manualBtn, I18n::t(StringId::WIFI_MANUAL_ENTRY), TFT_YELLOW);

        bool action = false;
        while (!action) {
            TouchCalib::Point tap;
            if (!TouchCalib::wasTapped(tap)) {
                delay(10);
                continue;
            }
            if (allowBack && UiText::BACK_TAP_RECT.contains(tap.x, tap.y)) {
                UiText::drawHeader(tft, I18n::t(StringId::WIFI_CHOOSE), true, true);
                delay(150);
                wentBack = true;
                return "";
            }
            if (rescanBtn.contains(tap.x, tap.y)) {
                action = true; // -> re-scan via aeussere Schleife
                break;
            }
            if (manualBtn.contains(tap.x, tap.y)) {
                bool cancelled = false;
                String ssid = runKeyboard(tft, I18n::t(StringId::KB_WIFI_NAME_PROMPT), false, cancelled);
                if (!cancelled && ssid.length() > 0) return ssid;
                action = true;
                break;
            }
            for (int i = 0; i < visible; i++) {
                Rect r = { 0, (int16_t)(listTop + i * rowH), W, rowH };
                if (r.contains(tap.x, tap.y)) {
                    return WiFi.SSID(i);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------
// Verbindungstest
// ---------------------------------------------------------------------

bool testConnection(TFT_eSPI& tft, const String& ssid, const String& password, String& outIp) {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;

    tft.fillScreen(TFT_BLACK);
    UiText::draw(tft, { 10, (int16_t)(H / 2 - 33), (int16_t)(W - 20), 20 },
                 I18n::t(StringId::WIFI_CONNECTING_TO), TFT_WHITE, TFT_BLACK);
    UiText::draw(tft, { 10, (int16_t)(H / 2 - 8), (int16_t)(W - 20), 24 },
                 ssid, TFT_WHITE, TFT_BLACK);

    WiFi.mode(WIFI_STA);
    WiFi.setHostname("Eiswolfs-iBoard");
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(250);
    }

    tft.fillScreen(TFT_BLACK);
    if (WiFi.status() == WL_CONNECTED) {
        outIp = WiFi.localIP().toString();
        UiText::draw(tft, { 10, (int16_t)(H / 2 - 33), (int16_t)(W - 20), 20 },
                     I18n::t(StringId::WIFI_CONNECTED), TFT_GREEN, TFT_BLACK);
        UiText::draw(tft, { 10, (int16_t)(H / 2 - 8), (int16_t)(W - 20), 24 },
                     outIp, TFT_WHITE, TFT_BLACK);
        delay(1500);
        return true;
    } else {
        UiText::draw(tft, { 10, (int16_t)(H / 2 - 22), (int16_t)(W - 20), 20 },
                     I18n::t(StringId::WIFI_CONNECT_FAILED), TFT_RED, TFT_BLACK);
        UiText::draw(tft, { 10, (int16_t)(H / 2 + 2), (int16_t)(W - 20), 20 },
                     I18n::t(StringId::WIFI_BACK_TO_INPUT), TFT_RED, TFT_BLACK);
        delay(1800);
        return false;
    }
}

// Rueckgabe false, wenn der Nutzer ueber den Zurueck-Pfeil abgebrochen
// hat (nur moeglich, wenn allowBack=true) - cfg bleibt in diesem Fall
// komplett unberuehrt (Auftrag Schritt 4: "ohne etwas zu speichern").
bool runWifiFlow(TFT_eSPI& tft, Storage::DeviceConfig& cfg, bool allowBack) {
    while (true) {
        bool wentBack = false;
        String ssid = runWifiScan(tft, allowBack, wentBack);
        if (wentBack) return false;
        while (true) {
            bool cancelled = false;
            String password = runKeyboard(tft, (String(I18n::t(StringId::KB_WIFI_PASSWORD_PREFIX)) + ssid).c_str(), true, cancelled);
            if (cancelled) break; // zurueck zur Netzwerkliste

            String ip;
            if (testConnection(tft, ssid, password, ip)) {
                cfg.wifiSsid = ssid;
                cfg.wifiPassword = password;
                return true;
            }
            // Fehlschlag -> Passwort erneut eingeben (gleiche SSID)
        }
    }
}

} // namespace

void runFullSetup(TFT_eSPI& tft, Storage::DeviceConfig& cfg) {
    runLanguageFlow(tft, cfg);
    TouchCalib::Calibration cal = runCalibrationFlow(tft);
    cfg.touchCal = cal;
    // allowBack=false: beim allerersten Start (noch keine gueltige
    // Einrichtung) gibt es keinen sinnvollen "vorherigen Schritt" nach
    // der bereits abgeschlossenen Kalibrierung - Pfeil bleibt deshalb
    // hier unsichtbar (Auftrag Schritt 4, letzter Satz).
    runWifiFlow(tft, cfg, false);
    if (cfg.macIp.length() == 0) cfg.macIp = "";
    cfg.valid = true;
    Storage::save(cfg);
}

void runWifiOnly(TFT_eSPI& tft, Storage::DeviceConfig& cfg) {
    // allowBack=true: aufgerufen ueber Menue > Einrichtung > "Nur WLAN
    // aendern" - eine bereits gueltige Einrichtung existiert, Zurueck
    // fuehrt ins Menue zurueck, OHNE etwas zu speichern (siehe
    // runWifiFlow()-Rueckgabewert oben).
    if (!runWifiFlow(tft, cfg, true)) return;
    cfg.valid = true;
    Storage::save(cfg);
}

void runCalibrationOnly(TFT_eSPI& tft, Storage::DeviceConfig& cfg) {
    TouchCalib::Calibration cal = runCalibrationFlow(tft);
    cfg.touchCal = cal;
    cfg.valid = true;
    Storage::save(cfg);
}

// Dünner, öffentlicher Zugang zur sonst anonymen runKeyboard() (Auftrag
// "Wake-on-LAN fuer den Windows-Laptop") - fuer die MAC-Adress-Eingabe
// im Menue, kein Passwort-Maskieren noetig.
String runTextEntry(TFT_eSPI& tft, const char* title, bool& cancelled, const char* initialText) {
    return runKeyboard(tft, title, false, cancelled, initialText);
}

}
