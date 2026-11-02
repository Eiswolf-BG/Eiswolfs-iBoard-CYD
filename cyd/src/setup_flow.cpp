#include "setup_flow.h"
#include "touch_calib.h"
#include "config.h"
#include "i18n.h"
#include <WiFi.h>

namespace SetupFlow {

namespace {

struct Rect {
    int16_t x, y, w, h;
    bool contains(int16_t px, int16_t py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

void drawButton(TFT_eSPI& tft, const Rect& r, const char* label, uint16_t color, bool filled = false) {
    if (filled) {
        tft.fillRect(r.x, r.y, r.w, r.h, color);
        tft.setTextColor(TFT_BLACK, color);
    } else {
        tft.fillRect(r.x, r.y, r.w, r.h, TFT_BLACK);
        tft.drawRect(r.x, r.y, r.w, r.h, color);
        tft.setTextColor(color, TFT_BLACK);
    }
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.drawString(label, r.x + r.w / 2, r.y + r.h / 2);
    tft.setTextDatum(TL_DATUM);
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
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.drawString("Sprache / Language", W / 2, H / 2 - 50);
    tft.setTextDatum(TL_DATUM);

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
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextSize(1);
            tft.setCursor(4, 2);
            tft.print(I18n::t(StringId::APP_NAME));
            tft.setTextDatum(MC_DATUM);
            tft.drawString(I18n::t(StringId::CALIB_TITLE), W / 2, H / 2 - 30);
            tft.drawString(labels[i], W / 2, H / 2 - 12);
            tft.setTextDatum(TL_DATUM);
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
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(I18n::t(StringId::CALIB_FAILED), W / 2, H / 2);
            tft.drawString(I18n::t(StringId::CALIB_RETRYING), W / 2, H / 2 + 16);
            tft.setTextDatum(TL_DATUM);
            delay(1500);
            continue;
        }

        TouchCalib::setCalibration(fitted);

        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(I18n::t(StringId::CALIB_TOUCH_TEST_POINT), W / 2, H / 2 - 30);
        tft.setTextDatum(TL_DATUM);
        drawTarget(tft, W / 2, H / 2);
        TouchCalib::Point tap = blockingTap(tft);

        int16_t dx = tap.x - W / 2;
        int16_t dy = tap.y - H / 2;
        bool ok = (dx * dx + dy * dy) <= (40 * 40);

        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        if (ok) {
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.drawString(I18n::t(StringId::CALIB_SAVED), W / 2, H / 2);
            tft.setTextDatum(TL_DATUM);
            delay(1000);
            return fitted;
        } else {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawString(I18n::t(StringId::CALIB_TEST_MISSED), W / 2, H / 2 - 10);
            tft.drawString(I18n::t(StringId::CALIB_RETRYING), W / 2, H / 2 + 10);
            tft.setTextDatum(TL_DATUM);
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
const char* ROW_DIGITS[] = { "1","2","3","4","5","6","7","8","9","0" };
const char* ROW_SYMBOLS[] = { "!","@","#","$","%","^","&","*","-","_" };
const char* ROW1_LOWER[] = { "q","w","e","r","t","y","u","i","o","p" };
const char* ROW1_UPPER[] = { "Q","W","E","R","T","Y","U","I","O","P" };
const char* ROW2_LOWER[] = { "a","s","d","f","g","h","j","k","l" };
const char* ROW2_UPPER[] = { "A","S","D","F","G","H","J","K","L" };
const char* ROW3_LOWER[] = { "z","x","c","v","b","n","m" };
const char* ROW3_UPPER[] = { "Z","X","C","V","B","N","M" };

String runKeyboard(TFT_eSPI& tft, const char* title, bool isPassword, bool& cancelled) {
    const int16_t W = Config::SCREEN_WIDTH;
    String text = "";
    bool shift = false;
    bool symbols = false;
    bool showPassword = false;
    cancelled = false;

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(1);
        tft.setCursor(4, 2);
        tft.print(title);

        tft.drawRect(4, 16, W - 8, 20, TFT_DARKGREY);
        tft.setCursor(8, 22);
        if (isPassword && !showPassword) {
            String mask;
            for (uint16_t i = 0; i < text.length(); i++) mask += "*";
            tft.print(mask);
        } else {
            tft.print(text);
        }

        int16_t keyH = 28;
        int16_t y = 40;

        // Reihe 0: Ziffern/Symbole
        const char** row0 = symbols ? ROW_SYMBOLS : ROW_DIGITS;
        int16_t colW = W / 10;
        for (int i = 0; i < 10; i++) {
            drawButton(tft, { (int16_t)(i * colW), y, colW, keyH }, row0[i], TFT_CYAN);
        }
        y += keyH;

        // Reihe 1
        const char** row1 = shift ? ROW1_UPPER : ROW1_LOWER;
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
        int16_t colW = W / 10;
        int16_t y0 = 40, y1 = y0 + keyH, y2 = y1 + keyH, y3 = y2 + keyH, y4 = y3 + keyH;

        bool redraw = true;

        if (tap.y >= y0 && tap.y < y1) {
            const char** row0 = symbols ? ROW_SYMBOLS : ROW_DIGITS;
            int idx = tap.x / colW;
            if (idx >= 0 && idx < 10) text += row0[idx];
        } else if (tap.y >= y1 && tap.y < y2) {
            const char** row1 = shift ? ROW1_UPPER : ROW1_LOWER;
            int idx = tap.x / colW;
            if (idx >= 0 && idx < 10) text += row1[idx];
        } else if (tap.y >= y2 && tap.y < y3) {
            if (tap.x >= W - 48) {
                if (text.length() > 0) text.remove(text.length() - 1);
            } else {
                const char** row2 = shift ? ROW2_UPPER : ROW2_LOWER;
                int16_t col2W = (W - 48) / 9;
                int idx = tap.x / col2W;
                if (idx >= 0 && idx < 9) text += row2[idx];
            }
        } else if (tap.y >= y3 && tap.y < y4) {
            if (tap.x < 36) {
                shift = !shift;
            } else if (tap.x >= W - 50) {
                symbols = !symbols;
            } else {
                const char** row3 = shift ? ROW3_UPPER : ROW3_LOWER;
                int16_t col3W = (W - 36 - 50) / 7;
                int idx = (tap.x - 36) / col3W;
                if (idx >= 0 && idx < 7) text += row3[idx];
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

String runWifiScan(TFT_eSPI& tft) {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;

    while (true) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(I18n::t(StringId::WIFI_SCANNING), W / 2, H / 2);
        tft.setTextDatum(TL_DATUM);

        int n = WiFi.scanNetworks();

        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(4, 2);
        tft.print(I18n::t(StringId::APP_NAME));
        tft.setCursor(4, 12);
        tft.print(I18n::t(StringId::WIFI_CHOOSE));

        const int16_t rowH = 28;
        const int16_t listTop = 24;
        int visible = min(n, 6);

        for (int i = 0; i < visible; i++) {
            Rect r = { 0, (int16_t)(listTop + i * rowH), W, rowH };
            String label = WiFi.SSID(i);
            if (label.length() == 0) label = I18n::t(StringId::WIFI_HIDDEN);
            label += "  " + String(WiFi.RSSI(i)) + "dBm";
            drawButton(tft, r, label.c_str(), TFT_CYAN);
        }

        int16_t bottomY = listTop + visible * rowH + 4;
        Rect rescanBtn = { 0, bottomY, (int16_t)(W / 2), 28 };
        Rect manualBtn = { (int16_t)(W / 2), bottomY, (int16_t)(W / 2), 28 };
        drawButton(tft, rescanBtn, I18n::t(StringId::WIFI_RESCAN), TFT_YELLOW);
        drawButton(tft, manualBtn, I18n::t(StringId::WIFI_MANUAL_ENTRY), TFT_YELLOW);

        uint32_t screenStartMs = millis();
        bool action = false;
        while (!action) {
            TouchCalib::Point tap;
            if (!TouchCalib::wasTapped(tap)) {
                delay(10);
                continue;
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
        (void)screenStartMs;
    }
}

// ---------------------------------------------------------------------
// Verbindungstest
// ---------------------------------------------------------------------

bool testConnection(TFT_eSPI& tft, const String& ssid, const String& password, String& outIp) {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(I18n::t(StringId::WIFI_CONNECTING_TO), W / 2, H / 2 - 20);
    tft.drawString(ssid, W / 2, H / 2);
    tft.setTextDatum(TL_DATUM);

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
    tft.setTextDatum(MC_DATUM);
    if (WiFi.status() == WL_CONNECTED) {
        outIp = WiFi.localIP().toString();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString(I18n::t(StringId::WIFI_CONNECTED), W / 2, H / 2 - 20);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(outIp, W / 2, H / 2);
        tft.setTextDatum(TL_DATUM);
        delay(1500);
        return true;
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString(I18n::t(StringId::WIFI_CONNECT_FAILED), W / 2, H / 2 - 10);
        tft.drawString(I18n::t(StringId::WIFI_BACK_TO_INPUT), W / 2, H / 2 + 10);
        tft.setTextDatum(TL_DATUM);
        delay(1800);
        return false;
    }
}

void runWifiFlow(TFT_eSPI& tft, Storage::DeviceConfig& cfg) {
    while (true) {
        String ssid = runWifiScan(tft);
        while (true) {
            bool cancelled = false;
            String password = runKeyboard(tft, (String(I18n::t(StringId::KB_WIFI_PASSWORD_PREFIX)) + ssid).c_str(), true, cancelled);
            if (cancelled) break; // zurueck zur Netzwerkliste

            String ip;
            if (testConnection(tft, ssid, password, ip)) {
                cfg.wifiSsid = ssid;
                cfg.wifiPassword = password;
                return;
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
    runWifiFlow(tft, cfg);
    if (cfg.macIp.length() == 0) cfg.macIp = "";
    cfg.valid = true;
    Storage::save(cfg);
}

void runWifiOnly(TFT_eSPI& tft, Storage::DeviceConfig& cfg) {
    runWifiFlow(tft, cfg);
    cfg.valid = true;
    Storage::save(cfg);
}

void runCalibrationOnly(TFT_eSPI& tft, Storage::DeviceConfig& cfg) {
    TouchCalib::Calibration cal = runCalibrationFlow(tft);
    cfg.touchCal = cal;
    cfg.valid = true;
    Storage::save(cfg);
}

}
