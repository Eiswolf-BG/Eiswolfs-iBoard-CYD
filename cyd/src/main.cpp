// Eiswolfs iBoard - CYD-Anzeige fuer Mac-Systemwerte (CPU/GPU/RAM/Temp/
// Power/Fan). Empfaengt eine JSON-Zeile pro Sekunde per UDP-Broadcast
// (Port 4210) ODER per USB-Serial (siehe ../mac/sender.py und
// ../mac/usb_link.py). WLAN-Zugangsdaten und Touch-Kalibrierung liegen
// in /iBoard/config.json auf der SD-Karte (SD-Ordnername bewusst NICHT
// umbenannt, siehe CLAUDE.md "Bewusst unveraendert") - Fallback: NVS,
// siehe storage.h.
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <esp_system.h>
#include "config.h"
#include "touch_calib.h"
#include "storage.h"
#include "i18n.h"
#include "secrets.h"
#include "setup_flow.h"
#include "fonts/PixelFont20.h"
#include "apple_rgb565.h"
// FreeSansBold9/18/24pt7b werden von TFT_eSPIs eigenem gfxfont.h bei
// LOAD_GFXFF bereits automatisch eingebunden (siehe gfxfont.h in der
// TFT_eSPI-Bibliothek) - ein eigenes #include hier wuerde zu
// "redefinition"-Fehlern fuehren, die Typen/Objekte sind schon da.

namespace {

TFT_eSPI tft = TFT_eSPI();
WiFiUDP udp;
Storage::DeviceConfig deviceCfg;

struct Rect {
    int16_t x, y, w, h;
    bool contains(int16_t px, int16_t py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

// ---------------------------------------------------------------------
// Schriften - "Pixel" (Press Start 2P, OFL) und "Klar" (FreeSansBold,
// bereits Teil der TFT_eSPI-Bibliothek). Siehe cyd/fonts/LICENSES.md.
// pickFont() waehlt je Text/Platzangebot die groesste Variante aus der
// jeweiligen Kandidatenliste, die sowohl in die Breite als auch in die
// Hoehe passt (tft.textWidth()/tft.fontHeight() nach setFreeFont()+
// setTextSize() - funktioniert fuer GFXFF-Fonts genauso wie fuer den
// eingebauten Font).
// ---------------------------------------------------------------------

enum class FontFamily { Pixel, Clear };
FontFamily currentFamily = FontFamily::Pixel;

struct FontChoice {
    const GFXfont* font;
    uint8_t size;
};

const FontChoice PIXEL_CANDIDATES[] = {
    { &PixelFont20, 2 },
    { &PixelFont20, 1 },
};
const FontChoice CLEAR_CANDIDATES[] = {
    { &FreeSansBold24pt7b, 1 },
    { &FreeSansBold18pt7b, 1 },
    { &FreeSansBold9pt7b, 1 },
};

void applyFont(const FontChoice& fc) {
    tft.setFreeFont(fc.font);
    tft.setTextSize(fc.size);
}

// Setzt die groesstmoegliche passende Schriftgroesse und gibt sie
// zurueck (bereits aktiv, kein erneuter Aufruf von applyFont() nötig).
FontChoice pickFont(const String& text, int16_t maxW, int16_t maxH) {
    const FontChoice* candidates = (currentFamily == FontFamily::Pixel) ? PIXEL_CANDIDATES : CLEAR_CANDIDATES;
    int count = (currentFamily == FontFamily::Pixel)
        ? (int)(sizeof(PIXEL_CANDIDATES) / sizeof(PIXEL_CANDIDATES[0]))
        : (int)(sizeof(CLEAR_CANDIDATES) / sizeof(CLEAR_CANDIDATES[0]));

    for (int i = 0; i < count; i++) {
        applyFont(candidates[i]);
        int16_t w = tft.textWidth(text);
        int16_t h = tft.fontHeight();
        if (w <= maxW && h <= maxH) return candidates[i];
    }
    applyFont(candidates[count - 1]);
    return candidates[count - 1];
}

void useBuiltinFont() {
    tft.setTextFont(1);
    tft.setTextSize(1);
}

// Waehlt die Schriftgroesse IMMER aus der klaren Schrift (FreeSansBold),
// unabhaengig von der im Menue gewaehlten Schriftfamilie - fuer
// Bootscreen-Texte und (seit Auftrag "Kopfzeile ueberarbeiten") den
// Kopfzeilen-Namen/die Quellenanzeige, die laut Auftrag immer in der
// klaren Schrift bleiben.
FontChoice pickClearFont(const String& text, int16_t maxW, int16_t maxH) {
    int count = (int)(sizeof(CLEAR_CANDIDATES) / sizeof(CLEAR_CANDIDATES[0]));
    for (int i = 0; i < count; i++) {
        applyFont(CLEAR_CANDIDATES[i]);
        if (tft.textWidth(text) <= maxW && tft.fontHeight() <= maxH) return CLEAR_CANDIDATES[i];
    }
    applyFont(CLEAR_CANDIDATES[count - 1]);
    return CLEAR_CANDIDATES[count - 1];
}

// ---------------------------------------------------------------------
// Layout: Kopfzeile + 2x3 Kacheln (Querformat 320x240). Jede Kachel hat
// oben einen farbigen Streifen mit Label (einmalig gezeichnet, aendert
// sich nie zur Laufzeit) und darunter den eigentlichen Wertbereich, der
// bei jeder Aenderung NUR dort geloescht und neu gezeichnet wird.
// ---------------------------------------------------------------------

constexpr int16_t HEADER_H = Config::USE_OLD_HEADER ? 20 : 34;
constexpr int16_t HEADER_LEFT_W = Config::SCREEN_WIDTH / 3;
constexpr int16_t TILE_GAP = 3;
constexpr int16_t OUTER_MARGIN = 2;
constexpr int16_t TILE_W = (Config::SCREEN_WIDTH - 2 * OUTER_MARGIN - 2 * TILE_GAP) / 3;
constexpr int16_t TILE_H = (Config::SCREEN_HEIGHT - HEADER_H - 2 * OUTER_MARGIN - TILE_GAP) / 2;
constexpr int16_t STRIPE_H = 29;

// Watt-Kachel in der Kopfzeile (neues Layout) - Rects muessen vor
// computeRowFonts() stehen, das pwrFont daraus berechnet.
constexpr int16_t PWR_STRIPE_W = 58;
Rect pwrTileRect = { HEADER_LEFT_W, 0, (int16_t)(Config::SCREEN_WIDTH - HEADER_LEFT_W), HEADER_H };
Rect pwrStripeRect = { HEADER_LEFT_W, 0, PWR_STRIPE_W, HEADER_H };
Rect pwrValueRect = { (int16_t)(HEADER_LEFT_W + PWR_STRIPE_W), 0, (int16_t)(Config::SCREEN_WIDTH - HEADER_LEFT_W - PWR_STRIPE_W), HEADER_H };
FontChoice pwrFont; // einmalig berechnet, siehe computeRowFonts()

enum TileIndex { TILE_CPU = 0, TILE_GPU, TILE_RAM, TILE_CPU_TEMP, TILE_GPU_TEMP, TILE_FAN, TILE_COUNT };

struct TileDef {
    const char* label;
    uint16_t stripeColor;
};

// Beschriftung bewusst ASCII-sicher ("TEMP" statt "CPU°"/"GPU°") - weder
// unser Pixel-Font (OFL, nur ASCII 0x20-0x7E) noch FreeSansBold (ebenfalls
// nur 0x20-0x7E, siehe gfxfont.h) stellen das Grad-Zeichen dar, die
// Werte selbst zeigen deshalb "61C" statt "61°". Die beiden TEMP-Kacheln
// tragen die Farbe der Last-Kachel direkt darueber (Cyan/Lila), damit
// klar ist, welche Temperatur zu welcher Last gehoert - die Zahl selbst
// bleibt in ihrer eigenen Warnfarbe (gruen/gelb/rot), siehe colorForTemp().
const TileDef TILES[TILE_COUNT] = {
    { "CPU", TFT_CYAN },
    { "GPU", TFT_PURPLE },
    { "RAM", TFT_ORANGE },
    { "TEMP", TFT_CYAN },
    { "TEMP", TFT_PURPLE },
    { "FAN / RPM", TFT_GREEN },
};

Rect tileRect(int index) {
    int row = index / 3;
    int col = index % 3;
    int16_t x = OUTER_MARGIN + col * (TILE_W + TILE_GAP);
    int16_t y = HEADER_H + OUTER_MARGIN + row * (TILE_H + TILE_GAP);
    return { x, y, TILE_W, TILE_H };
}

Rect tileBody(const Rect& outer) {
    return { outer.x, (int16_t)(outer.y + STRIPE_H), outer.w, (int16_t)(outer.h - STRIPE_H) };
}

// BUGFIX (Auftrag "Zahl blitzt bei Aenderung kurz gross/klein auf"):
// Ursache war, dass die Schriftgroesse bei JEDER Werteaenderung neu
// anhand der GENAUEN aktuellen Zeichenkette gemessen wurde (pickFont())
// - "5%" und "17%" landeten dadurch je nach Tick bei unterschiedlichen
// Groessen, und ein kurzzeitig abweichender Wert (z.B. durch einen
// einzelnen, einen Tick alten USB/WLAN-Wert waehrend des Quellenwechsels
// im Auto-Modus) liess die Schrift fuer einen Frame springen. Fix: die
// Schriftgroesse wird jetzt EINMALIG pro Zeilentyp (Reihe 1: CPU/GPU/RAM,
// Reihe 2: TMP/TMP/FAN) beim Layoutaufbau anhand der laengsten jemals
// moeglichen Werte berechnet (siehe computeRowFonts()) und danach nicht
// mehr veraendert - drawTileValue() wendet nur noch diese fixen
// row1Font/row2Font-Werte an, ohne erneut zu messen.
FontChoice row1Font;
FontChoice row2Font;

FontChoice fitAllStrings(const String strs[], int count, int16_t maxW, int16_t maxH) {
    const FontChoice* candidates = (currentFamily == FontFamily::Pixel) ? PIXEL_CANDIDATES : CLEAR_CANDIDATES;
    int ccount = (currentFamily == FontFamily::Pixel)
        ? (int)(sizeof(PIXEL_CANDIDATES) / sizeof(PIXEL_CANDIDATES[0]))
        : (int)(sizeof(CLEAR_CANDIDATES) / sizeof(CLEAR_CANDIDATES[0]));

    for (int i = 0; i < ccount; i++) {
        applyFont(candidates[i]);
        bool allFit = true;
        for (int j = 0; j < count; j++) {
            if (tft.textWidth(strs[j]) > maxW) { allFit = false; break; }
        }
        if (allFit && tft.fontHeight() <= maxH) return candidates[i];
    }
    applyFont(candidates[ccount - 1]);
    return candidates[ccount - 1];
}

void computeRowFonts() {
    Rect outer = tileRect(0);
    Rect body = tileBody(outer);
    Rect inner = { (int16_t)(body.x + 3), (int16_t)(body.y + 2), (int16_t)(body.w - 6), (int16_t)(body.h - 4) };

    // Reihe 1 (CPU/GPU/RAM): immer Balken reserviert, laengster Wert "100%".
    int16_t row1H = inner.h - 10;
    String row1Strs[] = { "100%", "--" };
    row1Font = fitAllStrings(row1Strs, 2, inner.w, row1H);

    // Reihe 2 (TEMP/TEMP/FAN): Hoehen-Worst-Case ist FAN MIT Balken (auf
    // gleicher Hoehe wie die Balken von CPU/GPU/RAM, siehe drawTileValue())
    // - die Einheit "RPM" steht seit dem Umbau im Streifen-Label
    // ("FAN / RPM"), nicht mehr als eigene Zeile unter der Zahl. TEMP-
    // Kacheln haben mehr Platz, bekommen aber dieselbe Groesse wie FAN,
    // damit die Reihe optisch einheitlich bleibt.
    int16_t row2H = inner.h - 10;
    String row2Strs[] = { "110C", "4900", "AUS", "--" };
    row2Font = fitAllStrings(row2Strs, 4, inner.w, row2H);

    if (!Config::USE_OLD_HEADER) {
        // Watt-Kachel in der Kopfzeile - eigener, deutlich kleinerer
        // Platz als die Hauptkacheln, deshalb eigene Groesse statt
        // row1Font/row2Font.
        int16_t pwrW = pwrValueRect.w - 6;
        int16_t pwrH = pwrValueRect.h - 4;
        String pwrStrs[] = { "100.0 W", "99.9 W", "--" };
        pwrFont = fitAllStrings(pwrStrs, 3, pwrW, pwrH);
    }
}

uint16_t colorForTemp(float tempC) {
    if (tempC < 60.0f) return TFT_GREEN;
    if (tempC < 80.0f) return TFT_YELLOW;
    return TFT_RED;
}

void drawBar(int16_t x, int16_t y, int16_t w, int16_t h, float pct, uint16_t color) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    int16_t filled = (int16_t)(w * (pct / 100.0f));
    tft.fillRect(x, y, filled, h, color);
    tft.fillRect(x + filled, y, w - filled, h, TFT_BLACK);
}

void drawStripe(int index) {
    Rect outer = tileRect(index);
    const TileDef& def = TILES[index];
    tft.fillRect(outer.x, outer.y, outer.w, STRIPE_H, def.stripeColor);

    String label = def.label;

    tft.setTextColor(TFT_BLACK, def.stripeColor);
    tft.setTextDatum(MC_DATUM);
    pickFont(label, (int16_t)(outer.w - 6), (int16_t)(STRIPE_H - 4));
    tft.drawString(label, outer.x + outer.w / 2, outer.y + STRIPE_H / 2);
    tft.setTextDatum(TL_DATUM);
}

// Luefter-Boost-Overlay auf dem FAN-Streifen (Auftrag "Luefter-Boost") -
// normalerweise "FAN / RPM" (gruen, wie TILES[] es vorgibt), waehrend
// eines von der Mac-App bestaetigten Boosts orange mit "BOOST" +
// Restzeit. Wird aus updateDisplay() JEDE Sekunde aufgerufen, zeichnet
// aber nur neu, wenn sich Text oder Boost-Zustand tatsaechlich
// geaendert haben (kein Aufblitzen) - die Startwerte entsprechen genau
// dem, was drawStaticLayout()/drawStripe() beim Layoutaufbau bereits
// gezeichnet haben.
String lastFanStripeText = "FAN / RPM";
bool lastFanStripeBoost = false;

String formatFanBoostLeft(int leftSec) {
    if (leftSec < 0) return "BOOST AN";
    int mm = leftSec / 60;
    int ss = leftSec % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "BOOST %d:%02d", mm, ss);
    return String(buf);
}

void updateFanStripe(bool boostActive, int leftSec) {
    String text = boostActive ? formatFanBoostLeft(leftSec) : "FAN / RPM";
    if (text == lastFanStripeText && boostActive == lastFanStripeBoost) return;

    Rect outer = tileRect(TILE_FAN);
    uint16_t color = boostActive ? TFT_ORANGE : TILES[TILE_FAN].stripeColor;
    tft.fillRect(outer.x, outer.y, outer.w, STRIPE_H, color);
    tft.setTextColor(TFT_BLACK, color);
    tft.setTextDatum(MC_DATUM);
    pickFont(text, (int16_t)(outer.w - 6), (int16_t)(STRIPE_H - 4));
    tft.drawString(text, outer.x + outer.w / 2, outer.y + STRIPE_H / 2);
    tft.setTextDatum(TL_DATUM);

    lastFanStripeText = text;
    lastFanStripeBoost = boostActive;
}

// Kurze "n/v"-Anzeige auf dem FAN-Streifen, wenn ein gesendeter Boost-/
// Auto-Befehl innerhalb von Config::FAN_CMD_CONFIRM_TIMEOUT_MS nicht
// bestaetigt wurde (Auftrag) - danach kehrt die Kachel von selbst zur
// normalen Darstellung zurueck (naechster updateFanStripe()-Aufruf
// erkennt den erzwungenen Zustandsunterschied und zeichnet neu).
void showFanStripeNotAvailable() {
    Rect outer = tileRect(TILE_FAN);
    tft.fillRect(outer.x, outer.y, outer.w, STRIPE_H, TFT_DARKGREY);
    tft.setTextColor(TFT_BLACK, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    pickFont(I18n::t(StringId::PWR_NOT_AVAILABLE), (int16_t)(outer.w - 6), (int16_t)(STRIPE_H - 4));
    tft.drawString(I18n::t(StringId::PWR_NOT_AVAILABLE), outer.x + outer.w / 2, outer.y + STRIPE_H / 2);
    tft.setTextDatum(TL_DATUM);
    delay(700);
    lastFanStripeText = ""; // erzwingt beim naechsten updateFanStripe() ein Neuzeichnen
}

// Zeichnet NUR den Wertbereich einer Kachel neu. "value" ist die fertig
// formatierte grosse Zahl inkl. direkt angehaengter Einheit (z.B.
// "20%", "61C") - die Luefter-Einheit "RPM" steht seit dem Umbau im
// Streifen-Label ("FAN / RPM"), nicht mehr als eigene Zeile hier unten.
// smallUnit bleibt als allgemeiner Mechanismus bestehen, wird aktuell
// aber von keiner Kachel mehr benutzt.
void drawTileValue(int index, const String& value, uint16_t valueColor,
                    bool hasBar, float barPct, const String& smallUnit = "") {
    Rect outer = tileRect(index);
    Rect body = tileBody(outer);
    Rect inner = { (int16_t)(body.x + 3), (int16_t)(body.y + 2), (int16_t)(body.w - 6), (int16_t)(body.h - 4) };

    tft.fillRect(body.x, body.y, body.w, body.h, TFT_BLACK);

    int16_t barAreaH = hasBar ? 10 : 0;
    int16_t unitAreaH = smallUnit.length() > 0 ? 12 : 0;
    int16_t valueAreaH = inner.h - barAreaH - unitAreaH;
    if (valueAreaH < 10) valueAreaH = 10;

    applyFont(index < 3 ? row1Font : row2Font);
    tft.setTextColor(valueColor, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(value, inner.x + inner.w / 2, inner.y + valueAreaH / 2);

    if (unitAreaH > 0) {
        useBuiltinFont();
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString(smallUnit, inner.x + inner.w / 2, inner.y + valueAreaH + unitAreaH / 2);
    }
    tft.setTextDatum(TL_DATUM);

    if (hasBar) {
        int16_t barY = body.y + body.h - barAreaH - 1;
        drawBar(inner.x, barY, inner.w, barAreaH - 1, barPct, valueColor);
    }
}

// ---------------------------------------------------------------------
// Empfangene Werte + Anzeigezustand
// ---------------------------------------------------------------------

struct Metrics {
    bool valid = false;
    float cpuPct = -1;
    float ramPct = -1;
    bool hasCpuTemp = false;
    float cpuTempC = 0;
    bool hasGpuTemp = false;
    float gpuTempC = 0;
    bool hasGpuPct = false;
    float gpuPct = 0;
    bool hasPower = false;
    float powerW = 0;
    bool hasFan = false;
    int fanRpm = 0;
    int fanMaxRpm = 0;
    // Luefter-Boost-Status, wie von der Mac-App gemeldet (Auftrag
    // "Luefter-Boost") - zeigt den WAHREN Zustand, nicht nur das, was
    // das CYD selbst zuletzt befohlen hat.
    bool fanBoost = false;
    int fanLeft = -1; // Sekunden, -1 = bis Stopp/unbekannt
    bool fanOk = false; // Hilfsdienst auf dem Mac verfuegbar?
    // Auftrag "Ruhemodus" - von der Mac-App bei jedem Paket mitgeschickter
    // Zaehler (sender.py::SenderThread.sleep_ack_counter), steigt EINMALIG
    // pro verarbeitetem SLEEP-Befehl. Neues, abwaertskompatibles Feld -
    // fehlt es (alte App-Version), bleibt der Wert einfach 0.
    int sleepAck = 0;
};

Metrics current;
Metrics lastDrawn;
bool lastDrawnValid = false;
bool showingNoConnection = false;
uint32_t lastUdpPacketMs = 0;
uint32_t lastUsbPacketMs = 0;
uint32_t lastWifiRetryMs = 0;
uint32_t packetCount = 0;
String usbLineBuf;

// "auto" (USB hat Vorrang, wenn frisch) / "wifi" (nur UDP) / "usb" (nur
// USB-Zeilen) - von Alex per Menue gewaehlt, siehe runMainMenu(). Das
// WLAN kann dabei weiterhin verbunden bleiben, die Anzeige haengt im
// Modus "usb" aber nicht davon ab (siehe parsePacket()).
String connectionMode = "auto";

// Luefter-Boost (Auftrag "Luefter-Boost") - IP der Mac-App, aus dem
// Absender des zuletzt empfangenen WLAN-Datenpakets gelernt (dieselbe
// Adresse, an die auch die eigene IP-Antwort geht, siehe loop()).
IPAddress lastMacIp;
bool haveMacIp = false;

// Wartet nach einem gesendeten FAN-Befehl auf Bestaetigung im naechsten
// Datenpaket (Feld "fan_boost") - siehe sendFanCommand()/loop().
bool fanCmdPending = false;
uint32_t fanCmdSentMs = 0;
bool fanCmdWantBoost = false;

// Ruhemodus (Auftrag "Ruhemodus"): PWR-Kachel antippen sendet "SLEEP
// <token>", genau wie sendFanCommand() oben auf demselben Weg/Port.
// sleepCmdBaselineAck merkt sich current.sleepAck im Moment des Sendens -
// jede Aenderung davon (egal in welche Richtung, der Zaehler steigt nur)
// gilt als Bestaetigung, siehe loop().
enum class PwrFeedback { None, Ruhe, NotAvailable };
PwrFeedback pwrFeedbackState = PwrFeedback::None;
uint32_t pwrFeedbackUntilMs = 0;
bool sleepCmdPending = false;
uint32_t sleepCmdSentMs = 0;
int sleepCmdBaselineAck = 0;
// 3s-Sperre nach einem gesendeten Befehl (Auftrag) - verhindert, dass ein
// prellender Touch zwei SLEEP-Befehle ausloest, unabhaengig von
// TouchCalib::wasTapped()s eigener Entprellung.
uint32_t pwrTapLockedUntilMs = 0;
constexpr uint32_t PWR_TAP_LOCKOUT_MS = 3000;
constexpr uint32_t PWR_FEEDBACK_RUHE_MS = 2000;
constexpr uint32_t PWR_FEEDBACK_NA_MS = 1500;

// Wake-on-LAN-Rueckmeldung auf der CPU-Temp-Kachel (Auftrag) - eigener,
// von PwrFeedback komplett unabhaengiger Zustand/eigene Taste (Alex'
// ausdruecklicher Wunsch: NICHT dieselbe Kachel wie der Ruhemodus, falls
// ein Touch-Ereignis mal falsch erkannt wird). Waehrend aktiv blendet
// updateDisplay() die normale Temperaturanzeige dieser einen Kachel aus,
// damit "WOL..." nicht sofort von einem neuen Messwert ueberschrieben wird.
bool wolFeedbackActive = false;
uint32_t wolFeedbackUntilMs = 0;
uint32_t wolTapLockedUntilMs = 0;
constexpr uint32_t WOL_TAP_LOCKOUT_MS = 3000;
constexpr uint32_t WOL_FEEDBACK_MS = 1500;

// Windows-Shutdown auf derselben Kachel wie WoL, aber langes Halten
// (Alex' ausdruecklicher Wunsch, "selbe wie an nur longpress"). Datei-
// weiter Scope (NICHT static innerhalb des Lang-Antippen-Blocks unten),
// weil der kurze-Tipp-Handler weiter oben dieses Flag mitlesen muss: ohne
// das wuerde TouchCalib::wasTapped() beim Loslassen NACH einem langen
// Halten zusaetzlich faelschlich auch noch den WoL-Kurztipp ausloesen
// (wasTapped() kennt selbst keine Zeitgrenze, siehe touch_calib.cpp).
bool cpuTempLongFired = false;

Rect setupButtonRect = { 10, 200, 130, 28 }; // Button "Menue" auf dem "Keine Verbindung"-Screen
// Mindestens ca. 80x40px (Auftrag) - reicht bewusst etwas in den oberen
// Rand der ersten Kachelreihe hinein (die Streifen dort sind nicht
// interaktiv, das stoert also nicht).
Rect headerLeftRect = { 0, 0, Config::USE_OLD_HEADER ? (int16_t)80 : HEADER_LEFT_W, 40 };
Rect headerRightRect = { 150, 0, 168, HEADER_H }; // nur im alten Layout benutzt

bool usbActiveNow() {
    return (millis() - lastUsbPacketMs) <= 3000;
}

// Tatsaechlich aktiver Weg unter Beruecksichtigung des gewaehlten Modus
// (nicht nur der reinen Zeitfrische wie usbActiveNow()).
bool usbIsActiveSource() {
    if (connectionMode == "usb") return true;
    if (connectionMode == "wifi") return false;
    return usbActiveNow();
}

// Luefter-Boost (Auftrag "Luefter-Boost") - schickt "FAN BOOST <sek>
// <stufe> <token>" bzw. "FAN AUTO <token>" auf dem gerade aktiven Weg
// (USB-Zeile oder UDP-Unicast an die gelernte Mac-IP, Port
// Config::FAN_CMD_UDP_PORT - NICHT der Datenport). Merkt sich danach,
// welchen Zustand es erwartet, damit loop() auf die Bestaetigung im
// naechsten Datenpaket warten kann (siehe Config::FAN_CMD_CONFIRM_TIMEOUT_MS).
void sendFanCommand(bool wantBoost) {
    char line[96];
    if (wantBoost) {
        snprintf(line, sizeof(line), "FAN BOOST %d %s %s",
                 deviceCfg.fanBoostSec, deviceCfg.fanBoostRpm.c_str(), FAN_BOOST_TOKEN);
    } else {
        snprintf(line, sizeof(line), "FAN AUTO %s", FAN_BOOST_TOKEN);
    }

    if (usbIsActiveSource()) {
        Serial.print(line);
        Serial.print("\n");
    } else if (WiFi.status() == WL_CONNECTED && haveMacIp) {
        udp.beginPacket(lastMacIp, Config::FAN_CMD_UDP_PORT);
        udp.print(line);
        udp.endPacket();
    }

    fanCmdPending = true;
    fanCmdSentMs = millis();
    fanCmdWantBoost = wantBoost;
}

// Ruhemodus (Auftrag "Ruhemodus") - schickt "SLEEP <token>" auf demselben
// Weg/Port wie sendFanCommand() oben (Config::FAN_CMD_UDP_PORT, NICHT der
// Datenport - der Port ist jetzt fuer beide Befehlsarten zustaendig, siehe
// CydCommandListener auf der Mac-Seite).
void sendSleepCommand() {
    char line[80];
    snprintf(line, sizeof(line), "SLEEP %s", FAN_BOOST_TOKEN);

    if (usbIsActiveSource()) {
        Serial.print(line);
        Serial.print("\n");
    } else if (WiFi.status() == WL_CONNECTED && haveMacIp) {
        udp.beginPacket(lastMacIp, Config::FAN_CMD_UDP_PORT);
        udp.print(line);
        udp.endPacket();
    }
}

// Wake-on-LAN fuer Alex' Windows-Laptop (Auftrag) - das CYD verschickt das
// Magic Packet SELBST per WLAN-Broadcast, braucht dafuer NICHT den Umweg
// ueber die Mac-App (anders als Ruhemodus/Luefter-Boost). Die MAC-Adresse
// steht NIE im Quellcode (oeffentliches Repo), sondern nur in
// deviceCfg.wolMacAddress (SD/NVS, am Geraet eingegeben).
bool parseMacAddress(const String& text, uint8_t out[6]) {
    int idx = 0;
    int pos = 0;
    while (idx < 6) {
        int sep = text.indexOf(':', pos);
        String byteStr = (sep == -1) ? text.substring(pos) : text.substring(pos, sep);
        byteStr.trim();
        if (byteStr.length() != 2) return false;
        char* endPtr = nullptr;
        long val = strtol(byteStr.c_str(), &endPtr, 16);
        if (endPtr == byteStr.c_str() || *endPtr != '\0' || val < 0 || val > 255) return false;
        out[idx] = (uint8_t)val;
        idx++;
        if (sep == -1) break;
        pos = sep + 1;
    }
    return idx == 6;
}

void sendWolPacket() {
    uint8_t mac[6];
    if (!parseMacAddress(deviceCfg.wolMacAddress, mac)) return;
    if (WiFi.status() != WL_CONNECTED) return;

    // Magic Packet: 6x 0xFF, danach die Ziel-MAC 16x wiederholt (102 Byte
    // insgesamt) - Standardformat, von jeder WoL-faehigen Netzwerkkarte
    // erkannt. Broadcast auf Port 9 (Standard-WoL-Port), erreicht jedes
    // Geraet im selben L2-Netz (WLAN und Ethernet am selben Router zaehlen
    // normalerweise als ein gemeinsames Netz).
    uint8_t packet[102];
    for (int i = 0; i < 6; i++) packet[i] = 0xFF;
    for (int i = 0; i < 16; i++) {
        memcpy(packet + 6 + i * 6, mac, 6);
    }
    udp.beginPacket(IPAddress(255, 255, 255, 255), 9);
    udp.write(packet, sizeof(packet));
    udp.endPacket();
}

// Windows-Laptop herunterfahren (Auftrag) - eigener Befehl an den
// Hintergrund-Listener (windows/), der auf WIN_SHUTDOWN_UDP_PORT lauscht
// und bei gueltigem Token "shutdown /s" ausfuehrt. Nutzt denselben Token
// wie Luefter-Boost/Ruhemodus (FAN_BOOST_TOKEN) - kein zweites Geheimnis.
void sendShutdownCommand() {
    if (WiFi.status() != WL_CONNECTED) return;
    char line[80];
    snprintf(line, sizeof(line), "SHUTDOWN %s", FAN_BOOST_TOKEN);
    udp.beginPacket(IPAddress(255, 255, 255, 255), Config::WIN_SHUTDOWN_UDP_PORT);
    udp.print(line);
    udp.endPacket();
}

void drawButton(const Rect& r, const char* label, uint16_t color, bool filled = false) {
    useBuiltinFont();
    if (filled) {
        tft.fillRect(r.x, r.y, r.w, r.h, color);
        tft.setTextColor(TFT_BLACK, color);
    } else {
        tft.fillRect(r.x, r.y, r.w, r.h, TFT_BLACK);
        tft.drawRect(r.x, r.y, r.w, r.h, color);
        tft.setTextColor(color, TFT_BLACK);
    }
    tft.setTextDatum(MC_DATUM);
    tft.drawString(label, r.x + r.w / 2, r.y + r.h / 2);
    tft.setTextDatum(TL_DATUM);
}

// Helligkeitsregler (Auftrag "Helligkeitsregler fuers Display") - TFT_eSPI
// selbst schaltet TFT_BL beim init() nur digital HIGH/LOW (siehe
// TFT_BACKLIGHT_ON in platformio.ini), nicht per PWM. analogWrite() ist
// der ESP32-Arduino-Core-Kompatibilitaets-Shim ueber ledc und funktioniert
// identisch auf alten (2.x) und neuen (3.x) Core-Versionen, ohne dass wir
// uns selbst um ledcSetup/ledcAttach kuemmern muessen. 10-100% statt 0-100%,
// damit das Menue bei der niedrigsten Stufe nicht unsichtbar/unbedienbar wird.
void applyBrightness() {
    int pct = constrain(deviceCfg.brightness, 10, 100);
    analogWrite(TFT_BL, map(pct, 0, 100, 0, 255));
}

void drawNoConnectionScreen() {
    tft.fillScreen(TFT_BLACK);
    useBuiltinFont();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(4, 2);
    tft.print(I18n::t(StringId::APP_NAME));

    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(I18n::t(StringId::NO_CONNECTION_LINE1), tft.width() / 2, 40);
    tft.drawString(I18n::t(StringId::NO_CONNECTION_LINE2), tft.width() / 2, 64);
    tft.setTextDatum(TL_DATUM);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 100);
    tft.print(I18n::t(StringId::WIFI_LABEL_PREFIX));
    tft.print(WiFi.status() == WL_CONNECTED ? WiFi.SSID() : deviceCfg.wifiSsid);
    tft.setCursor(10, 115);
    tft.print(I18n::t(StringId::IP_LABEL_PREFIX));
    tft.print(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("-"));
    tft.setCursor(10, 130);
    tft.print(I18n::t(StringId::STATUS_LABEL_PREFIX));
    tft.print(WiFi.status() == WL_CONNECTED ? I18n::t(StringId::STATUS_CONNECTED) : I18n::t(StringId::STATUS_NOT_CONNECTED));

    drawButton(setupButtonRect, I18n::t(StringId::MENU_BUTTON), TFT_YELLOW);
}

// ALTES Kopfzeilen-Layout (siehe Config::USE_OLD_HEADER): Watt +
// aktiver Uebertragungsweg (USB/WLAN) klein oben rechts, ein
// gemeinsamer, fest reservierter Bereich, der bei jeder Aenderung
// KOMPLETT (beide Teile zusammen) geloescht und neu geschrieben wird.
// Das war die Ursache der halben Linie unter der Kopfzeile (siehe
// CLAUDE.md): eine fruehere Version loeschte hier nur ein
// Teilrechteck, das zufaellig die Trennlinie unter der Kopfzeile mit
// uebermalte. Die Trennlinie selbst wurde komplett entfernt, nicht nur
// reparatur-gezeichnet.
void drawHeaderRightOld(bool force) {
    static bool hasLast = false;
    static float lastW = -1;
    static bool lastUsb = false;
    static bool lastForced = false;

    bool usbNow = usbIsActiveSource();
    bool forced = connectionMode != "auto";
    bool powerChanged = current.hasPower && (!hasLast || (int)(current.powerW * 10) != (int)(lastW * 10));
    bool powerGone = !current.hasPower && hasLast;
    bool sourceChanged = usbNow != lastUsb || forced != lastForced;

    if (!force && !powerChanged && !powerGone && !sourceChanged) return;

    tft.fillRect(headerRightRect.x, headerRightRect.y, headerRightRect.w, headerRightRect.h, TFT_BLACK);
    useBuiltinFont();
    tft.setTextDatum(TR_DATUM);

    int16_t rightEdge = Config::SCREEN_WIDTH - 4;

    // Sternchen hinter dem Wort zeigt eine fest gewaehlte (nicht
    // automatisch erkannte) Quelle an - siehe Auftrag.
    String sourceLabel = (usbNow ? "USB" : "WLAN") + String(forced ? "*" : "");
    tft.setTextColor(usbNow ? TFT_GREEN : TFT_DARKGREY, TFT_BLACK);
    tft.drawString(sourceLabel, rightEdge, 6);

    int16_t sourceLabelW = tft.textWidth(sourceLabel);

    if (current.hasPower) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString(String(current.powerW, 1) + " W", (int16_t)(rightEdge - sourceLabelW - 10), 6);
    }

    tft.setTextDatum(TL_DATUM);
    hasLast = current.hasPower;
    lastW = current.powerW;
    lastUsb = usbNow;
    lastForced = forced;
}

// NEUES Kopfzeilen-Layout (Normalbetrieb): links der Name (gross, klare
// Schrift, statisch, siehe drawStaticLayout()), rechts oben in der Ecke
// das Quellenkuerzel (Auftrag "Kopfzeile ueberarbeiten", 29.10. - war
// vorher unter dem Namen links), ganz rechts die Watt-Kachel im selben
// Stil wie die sechs Hauptkacheln (Streifen + Wertbereich).
constexpr uint16_t HEADER_NAME_COLOR = 0x7EBF; // #7FD6FF (naechster RGB565-Wert), siehe Boot::ICE_BLUE
// BUGFIX ("USB-Anzeige verschwindet"/"flackert"): die Watt-Kachel
// loeschte bei jeder Aenderung ihren KOMPLETTEN Wertbereich (volle
// Kopfzeilenhoehe) schwarz - das ueberlappte die Ecke der Quellen-
// anzeige, loeschte sie wieder und (als Notloesung) wurde sie danach
// jeden Tick neu gezeichnet, was das Flackern verursachte. Echter Fix:
// die beiden Zeichenbereiche ueberschneiden sich jetzt gar nicht mehr -
// die Quellenanzeige bekommt oben einen eigenen, schmalen Streifen
// (HEADER_SOURCE_H), die Watt-Kachel zeichnet nur noch darunter.
// Zusaetzlich: eingebaute (kompakte) Schrift statt der grossen "Klar"-
// Schrift fuer dieses eine Element (Auftrag: "etwas kleiner").
constexpr int16_t HEADER_SOURCE_H = 10;
Rect headerSourceRect = { (int16_t)(Config::SCREEN_WIDTH - 80), 0, 76, HEADER_SOURCE_H };

void drawHeaderSourceNew(bool force) {
    static bool lastUsb = false;
    static bool lastForced = false;
    static bool hasLast = false;

    bool usbNow = usbIsActiveSource();
    bool forced = connectionMode != "auto";
    if (!force && hasLast && usbNow == lastUsb && forced == lastForced) return;

    tft.fillRect(headerSourceRect.x, headerSourceRect.y, headerSourceRect.w, headerSourceRect.h, TFT_BLACK);
    String sourceLabel = (usbNow ? "USB" : "WLAN") + String(forced ? "*" : "");
    useBuiltinFont();
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(usbNow ? TFT_GREEN : TFT_DARKGREY, TFT_BLACK);
    tft.drawString(sourceLabel, Config::SCREEN_WIDTH - 4, 1);
    tft.setTextDatum(TL_DATUM);

    lastUsb = usbNow;
    lastForced = forced;
    hasLast = true;
}

// Ruhemodus-Rueckmeldung (Auftrag "Ruhemodus") - zeichnet GENAU in
// dieselbe Flaeche wie der Wattwert (pwrValueRect, nur der Bereich
// unterhalb des Quellenanzeige-Streifens), in der Farbe des PWR-
// Streifens (TFT_YELLOW). Einmaliger Aufruf beim Auftreten des Zustands,
// kein Dauer-Neuzeichnen - verhindert Aufblitzen/Flackern.
void drawPwrFeedbackText(const char* text) {
    int16_t clearY = headerSourceRect.y + headerSourceRect.h;
    int16_t clearH = pwrValueRect.h - clearY;
    tft.fillRect(pwrValueRect.x, clearY, pwrValueRect.w, clearH, TFT_BLACK);
    applyFont(pwrFont);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(text, pwrValueRect.x + pwrValueRect.w / 2, pwrValueRect.y + pwrValueRect.h / 2);
    tft.setTextDatum(TL_DATUM);
}

void drawHeaderPwrValueNew(bool force) {
    // Ruhemodus-Rueckmeldung hat Vorrang - solange sie steht, nie
    // ueberschreiben (siehe loop(), das sie selbst wieder aufraeumt).
    if (pwrFeedbackState != PwrFeedback::None) return;

    static bool hasLast = false;
    static float lastW = -1;

    bool powerChanged = current.hasPower && (!hasLast || (int)(current.powerW * 10) != (int)(lastW * 10));
    bool powerGone = !current.hasPower && hasLast;
    if (!force && !powerChanged && !powerGone) return;

    // Nur UNTERHALB des Quellenanzeige-Streifens loeschen/zeichnen,
    // siehe Kommentar oben - verhindert die Ueberschneidung an der
    // Wurzel, statt sie nur durch Dauer-Neuzeichnen zu kaschieren.
    int16_t clearY = headerSourceRect.y + headerSourceRect.h;
    int16_t clearH = pwrValueRect.h - clearY;
    tft.fillRect(pwrValueRect.x, clearY, pwrValueRect.w, clearH, TFT_BLACK);
    applyFont(pwrFont);
    tft.setTextColor(current.hasPower ? TFT_YELLOW : TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    String valueStr = current.hasPower ? (String(current.powerW, 1) + " W") : String("--");
    tft.drawString(valueStr, pwrValueRect.x + pwrValueRect.w / 2, pwrValueRect.y + pwrValueRect.h / 2);
    tft.setTextDatum(TL_DATUM);

    hasLast = current.hasPower;
    lastW = current.powerW;
}

void drawHeaderDynamic(bool force) {
    if (Config::USE_OLD_HEADER) {
        drawHeaderRightOld(force);
    } else {
        // Beide Bereiche ueberschneiden sich jetzt nicht mehr (siehe
        // Kommentare bei drawHeaderPwrValueNew()/headerSourceRect), die
        // Quellenanzeige kann deshalb wieder nur bei eigener Aenderung
        // neu gezeichnet werden (kein Dauer-Neuzeichnen mehr noetig).
        drawHeaderPwrValueNew(force);
        drawHeaderSourceNew(force);
    }
}

void drawStaticLayout() {
    tft.fillScreen(TFT_BLACK);
    useBuiltinFont();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    if (Config::USE_OLD_HEADER) {
        tft.setCursor(4, 6);
        tft.print("Eiswolfs iBoard");
    } else {
        // Name zweizeilig, groesser und in der klaren Schrift (Auftrag
        // "Kopfzeile ueberarbeiten", 29.10.) - IMMER klare Schrift,
        // auch wenn im Menue "Pixel" gewaehlt ist. Breite vor dem
        // Zeichnen geprueft, damit nichts den PWR-Streifen beruehrt.
        int16_t nameMaxW = (int16_t)(HEADER_LEFT_W - 8);
        tft.setTextColor(HEADER_NAME_COLOR, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        pickClearFont("Eiswolfs", nameMaxW, 16);
        tft.drawString("Eiswolfs", 4, 1);
        pickClearFont("iBoard", nameMaxW, 16);
        tft.drawString("iBoard", 4, 17);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        // Watt-Kachel: Streifen einmalig statisch wie bei den anderen
        // Kacheln, Wertbereich folgt dynamisch (siehe drawHeaderDynamic()).
        tft.fillRect(pwrStripeRect.x, pwrStripeRect.y, pwrStripeRect.w, pwrStripeRect.h, TFT_YELLOW);
        tft.setTextColor(TFT_BLACK, TFT_YELLOW);
        tft.setTextDatum(MC_DATUM);
        pickFont("PWR", (int16_t)(pwrStripeRect.w - 4), (int16_t)(pwrStripeRect.h - 4));
        tft.drawString("PWR", pwrStripeRect.x + pwrStripeRect.w / 2, pwrStripeRect.y + pwrStripeRect.h / 2);
        tft.setTextDatum(TL_DATUM);
    }

    for (int i = 0; i < TILE_COUNT; i++) {
        drawStripe(i);
    }

    computeRowFonts();
}

void updateDisplay() {
    bool full = !lastDrawnValid;

    if (full || (int)current.cpuPct != (int)lastDrawn.cpuPct) {
        drawTileValue(TILE_CPU, String((int)current.cpuPct) + "%", TFT_CYAN, true, current.cpuPct);
    }
    if (full || (int)current.gpuPct != (int)lastDrawn.gpuPct || current.hasGpuPct != lastDrawn.hasGpuPct) {
        if (current.hasGpuPct) {
            drawTileValue(TILE_GPU, String((int)current.gpuPct) + "%", TFT_PURPLE, true, current.gpuPct);
        } else {
            drawTileValue(TILE_GPU, "--", TFT_DARKGREY, false, 0);
        }
    }
    if (full || (int)current.ramPct != (int)lastDrawn.ramPct) {
        drawTileValue(TILE_RAM, String((int)current.ramPct) + "%", TFT_ORANGE, true, current.ramPct);
    }
    if (!wolFeedbackActive && (full || (int)current.cpuTempC != (int)lastDrawn.cpuTempC || current.hasCpuTemp != lastDrawn.hasCpuTemp)) {
        if (current.hasCpuTemp) {
            drawTileValue(TILE_CPU_TEMP, String((int)current.cpuTempC) + "C", colorForTemp(current.cpuTempC), false, 0);
        } else {
            drawTileValue(TILE_CPU_TEMP, "--", TFT_DARKGREY, false, 0);
        }
    }
    if (full || (int)current.gpuTempC != (int)lastDrawn.gpuTempC || current.hasGpuTemp != lastDrawn.hasGpuTemp) {
        if (current.hasGpuTemp) {
            drawTileValue(TILE_GPU_TEMP, String((int)current.gpuTempC) + "C", colorForTemp(current.gpuTempC), false, 0);
        } else {
            drawTileValue(TILE_GPU_TEMP, "--", TFT_DARKGREY, false, 0);
        }
    }
    if (full || current.fanRpm != lastDrawn.fanRpm || current.hasFan != lastDrawn.hasFan) {
        if (current.hasFan) {
            if (current.fanRpm <= 0) {
                drawTileValue(TILE_FAN, "AUS", TFT_DARKGREY, false, 0);
            } else {
                float fanPct = current.fanMaxRpm > 0 ? (100.0f * current.fanRpm / current.fanMaxRpm) : 0;
                drawTileValue(TILE_FAN, String(current.fanRpm), TFT_GREEN, true, fanPct);
            }
        } else {
            drawTileValue(TILE_FAN, "--", TFT_DARKGREY, false, 0);
        }
    }

    updateFanStripe(current.fanBoost, current.fanLeft);

    drawHeaderDynamic(full);

    lastDrawn = current;
    lastDrawnValid = true;
}


// ---------------------------------------------------------------------
// Netzwerk / USB
// ---------------------------------------------------------------------

void parsePacket(const char* json, size_t len, bool isUsb) {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, json, len);
    if (err) {
        // Kaputte/unvollstaendige Pakete duerfen den "verbunden"-Zaehler
        // NICHT zuruecksetzen (sonst waere ein einzelnes defektes Paket
        // ausreichend, um faelschlich auf "Keine Verbindung" zu
        // springen) - die jeweilige lastXxxPacketMs bleibt unberuehrt.
        if (Config::DEBUG_LOG) {
            Serial.printf("[Eiswolfs iBoard] JSON-Fehler (%s): %s\n", isUsb ? "USB" : "WLAN", err.c_str());
        }
        return;
    }

    // Verbindungsmodus (siehe runMainMenu()): in "wifi"/"usb" wird die
    // jeweils andere Quelle komplett ignoriert - zaehlt auch nicht fuer
    // den Verbindungsstatus. Der Handshake ("IBOARD?"/"IBOARD_OK") ist
    // davon unberuehrt, der liegt in pollUsbSerial() VOR diesem Aufruf.
    if (connectionMode == "wifi" && isUsb) return;
    if (connectionMode == "usb" && !isUsb) return;

    Metrics m;
    m.valid = true;
    m.cpuPct = doc["cpu_pct"] | 0.0f;
    m.ramPct = doc["ram_pct"] | 0.0f;

    if (!doc["cpu_temp_c"].isNull()) {
        m.hasCpuTemp = true;
        m.cpuTempC = doc["cpu_temp_c"];
    }
    if (!doc["gpu_temp_c"].isNull()) {
        m.hasGpuTemp = true;
        m.gpuTempC = doc["gpu_temp_c"];
    }
    if (!doc["gpu_pct"].isNull()) {
        m.hasGpuPct = true;
        m.gpuPct = doc["gpu_pct"];
    }
    if (!doc["power_w"].isNull()) {
        m.hasPower = true;
        m.powerW = doc["power_w"];
    }
    if (!doc["fan_rpm"].isNull()) {
        m.hasFan = true;
        m.fanRpm = doc["fan_rpm"];
        m.fanMaxRpm = doc["fan_max_rpm"] | 0;
    }
    m.fanBoost = (doc["fan_boost"] | 0) != 0;
    m.fanLeft = doc["fan_left"] | -1;
    m.fanOk = (doc["fan_ok"] | 0) != 0;
    m.sleepAck = doc["sleep_ack"] | 0;

    uint32_t now = millis();

    // Nur im Modus "auto": USB hat Vorrang, wenn frisch - WLAN-Pakete
    // duerfen die Anzeige trotzdem aktualisieren, solange gerade kein
    // aktuelles USB-Paket vorliegt.
    if (connectionMode == "auto" && !isUsb && usbActiveNow()) {
        lastUdpPacketMs = now; // zaehlt fuer den reinen Verbindungsstatus mit
        return;
    }

    current = m;
    if (isUsb) lastUsbPacketMs = now; else lastUdpPacketMs = now;

    if (Config::DEBUG_LOG) {
        packetCount++;
        Serial.printf("[Eiswolfs iBoard] Paket #%lu Quelle=%s t=%lu cpu=%.1f\n",
                       (unsigned long)packetCount, isUsb ? "USB" : "WLAN", (unsigned long)now, m.cpuPct);
    }
}

void pollUsbSerial() {
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n') {
            String line = usbLineBuf;
            usbLineBuf = "";
            line.trim();
            if (line == "IBOARD?") {
                Serial.print("IBOARD_OK\n");
            } else if (line.length() > 0 && line[0] == '{') {
                parsePacket(line.c_str(), line.length(), true);
            }
            // alles andere (Boot-Ausgaben etc.) wird bewusst ignoriert.
        } else if (c != '\r') {
            usbLineBuf += c;
            if (usbLineBuf.length() > 600) usbLineBuf = ""; // Schutz gegen Pufferueberlauf
        }
    }
}

// ---------------------------------------------------------------------
// Bootscreen (Auftrag "Bootscreen mit Apfel", 29.10.) - siehe CLAUDE.md
// "Bootscreen" fuer Masse/Farben/Positionen. Zeigt 7s lang Name + Apfel
// + Puls-Linie + Ladebalken, waehrenddessen laeuft der WLAN-Verbindungs-
// aufbau NICHT-BLOCKIERD im Hintergrund (eigene Uhr, kein
// WiFi.begin()-Warteloop wie frueher in connectWifi()). Kurzes Antippen
// ueberspringt sofort; danach eine kurze Tipp-Sperre, damit derselbe
// Tap nicht gleichzeitig einen Button der naechsten Anzeige ausloest.
// Nur beim normalen Boot/Reset aufgerufen - NICHT bei WLAN-Wechseln,
// Verbindungsabbruechen oder beim Schliessen des Menues.
namespace Boot {
    // Auftrag "Bootscreen im Dashboard-Stil" (02.11.): AUSSCHLIESSLICH
    // die bereits im Dashboard verwendeten Farbkonstanten - kein eigenes
    // Eisblau mehr (vorher HEADER_NAME_COLOR/ICE_BLUE 0x7EBF, kam im
    // Dashboard nirgends vor). "Das eine Gruen" ist TFT_GREEN (FAN-
    // Streifen, siehe TILES[]), PWR-Streifen ist TFT_YELLOW, GPU-Streifen
    // TFT_PURPLE, RAM-Streifen TFT_ORANGE, CPU-Streifen TFT_CYAN.
    constexpr uint16_t BG = 0x0000;         // #000000 - identisch zum Dashboard-Hintergrund (siehe CLAUDE.md)
    constexpr uint16_t BAR_TRACK = 0x2124;  // #262626 (Ladebalken-Spur)
    constexpr uint16_t STATUS_GREY = 0xCE59; // #C8C8C8 (Statuszeile)
    constexpr uint16_t VERSION_GREY = 0x7C77; // unveraendert gegenueber vorher ("bleibt in Dunkelgrau")

    constexpr int16_t TITLE_BASELINE_Y = 32;
    constexpr int16_t APPLE_X = (Config::SCREEN_WIDTH - APPLE_W) / 2; // zentriert
    constexpr int16_t APPLE_Y = 51; // Stiel-Oberkante landet bei ca. y=52

    constexpr int16_t PULSE_BASE_Y = 122;
    constexpr int16_t BAR_X = 30, BAR_Y = 204, BAR_W = 260, BAR_H = 4;
    constexpr int16_t STATUS_Y = 228;
    constexpr int16_t VERSION_X = Config::SCREEN_WIDTH - 6;
    constexpr int16_t VERSION_Y_LOW = STATUS_Y;   // normale Position, neben der Statuszeile
    constexpr int16_t VERSION_Y_HIGH = BAR_Y - 6; // Ausweichposition ueber dem Balken (lange IP)
    constexpr int16_t VERSION_GAP_MIN = 8;        // Mindestabstand Statuszeile <-> Version

    // Punkte der Puls-Linie relativ zu PULSE_BASE_Y (siehe Auftrag,
    // Werte 1:1 uebernommen).
    struct PulsePoint { int16_t x, dy; };
    const PulsePoint PULSE_POINTS[] = {
        { 70, 0 }, { 135, 0 }, { 145, -6 }, { 153, 26 },
        { 166, -32 }, { 176, 18 }, { 184, 0 }, { 250, 0 },
    };
    constexpr int PULSE_POINT_COUNT = sizeof(PULSE_POINTS) / sizeof(PULSE_POINTS[0]);
    constexpr int16_t PULSE_X_START = 70;
    constexpr int16_t PULSE_X_END = 250;
}


// Zeichnet die Puls-Linie bis zu einer bestimmten X-Position (fuer die
// Aufbau-Animation) - zeichnet NUR das neu hinzugekommene Teilstueck
// (von lastX bis targetX), nicht die ganze Linie neu, damit nichts
// flackert. lastX/lastPointIdx sind Zustand des Aufrufers.
void drawPulseSegment(int16_t fromX, int16_t toX) {
    if (toX <= fromX) return;
    using namespace Boot;
    for (int i = 0; i < PULSE_POINT_COUNT - 1; i++) {
        int16_t x0 = PULSE_POINTS[i].x, x1 = PULSE_POINTS[i + 1].x;
        if (x1 <= fromX || x0 >= toX) continue;
        int16_t y0 = PULSE_BASE_Y + PULSE_POINTS[i].dy;
        int16_t y1 = PULSE_BASE_Y + PULSE_POINTS[i + 1].dy;
        int16_t segFromX = max(x0, fromX);
        int16_t segToX = min(x1, toX);
        float t0 = (float)(segFromX - x0) / (x1 - x0);
        float t1 = (float)(segToX - x0) / (x1 - x0);
        int16_t sy0 = y0 + (int16_t)((y1 - y0) * t0);
        int16_t sy1 = y0 + (int16_t)((y1 - y0) * t1);
        // BUGFIX (Auftrag "gelbliche Linienenden"): der zweite Parameter
        // ist die Hintergrundfarbe, die drawWideLine() beim Anti-
        // Aliasing an den (abgerundeten) Kanten einblendet - das war
        // faelschlich TFT_WHITE statt der tatsaechlichen Hintergrund-
        // farbe, wodurch die Kanten/Enden der Linie falsch verblendet
        // wurden (sichtbar als Farbsaum). Jetzt korrekt Boot::BG.
        tft.drawWideLine(segFromX, sy0, segToX, sy1, 3.5f, TFT_WHITE, BG);
    }
}

void runBootScreen() {
    using namespace Boot;

    tft.fillScreen(BG);

    // Titel mehrfarbig (Auftrag): "Eiswolfs " in Gruen, "iBoard"
    // buchstabenweise in Dashboard-Farben (i=Gelb, B=Lila, o=Orange,
    // a=Hellblau, r=Gruen, d=Lila). Schriftgroesse einmalig anhand der
    // GESAMTEN Zeile bestimmen (wie bisher ca. 22px), danach bleibt sie
    // fuer alle Teilzeichenketten gleich. Jede Position wird aus der
    // gemessenen Breite der Teil-Zeichenkette DAVOR berechnet (nicht aus
    // der Summe der Einzelbreiten), damit der Abstand exakt stimmt.
    String title = "Eiswolfs iBoard";
    pickClearFont(title, Config::SCREEN_WIDTH - 20, 24);
    int16_t totalW = tft.textWidth(title);
    int16_t startX = Config::SCREEN_WIDTH / 2 - totalW / 2;

    tft.setTextDatum(BL_DATUM);
    String prefix = "Eiswolfs ";
    tft.setTextColor(TFT_GREEN, BG);
    tft.drawString(prefix, startX, TITLE_BASELINE_Y);

    constexpr char LETTERS[] = "iBoard";
    constexpr uint16_t LETTER_COLORS[] = { TFT_YELLOW, TFT_PURPLE, TFT_ORANGE, TFT_CYAN, TFT_GREEN, TFT_PURPLE };
    for (uint8_t i = 0; i < 6; i++) {
        String before = prefix + String(LETTERS).substring(0, i);
        int16_t x = startX + tft.textWidth(before);
        tft.setTextColor(LETTER_COLORS[i], BG);
        tft.drawString(String(LETTERS[i]), x, TITLE_BASELINE_Y);
    }
    tft.setTextDatum(TL_DATUM);

    // Apfel (fertiges Bild, Hintergrund bereits eingebacken). BUGFIX
    // (Auftrag "Apfel wird nicht richtig dargestellt"): die RGB565-
    // Werte liegen im C-Array in normaler (R5G6B5) Bit-Reihenfolge,
    // aber als little-endian uint16_t im Speicher - TFT_eSPI erwartet
    // bei pushImage() ohne setSwapBytes() big-endian Pixeldaten, sonst
    // werden die Bytes jedes Pixels vertauscht (sichtbar als bunte
    // Querstreifen statt des glatten Verlaufs).
    tft.setSwapBytes(true);
    tft.pushImage(APPLE_X, APPLE_Y, APPLE_W, APPLE_H, APPLE_RGB565);
    tft.setSwapBytes(false);

    // Ladebalken-Hintergrund (abgerundet, einmalig) - filigran (4px statt
    // vorher 8px), Spur in Dunkelgrau, Fuellung in der CPU-Farbe
    // (Hellblau/TFT_CYAN, siehe Schleife unten).
    tft.fillRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, BAR_H / 2, BAR_TRACK);

    // Version unten rechts, kleine/schmale Bibliotheksschrift (Auftrag) -
    // Startposition "niedrig" (neben der Statuszeile), weicht bei Bedarf
    // nach oben aus (siehe updateVersionPosition() in der Schleife).
    bool versionHigh = false;
    useBuiltinFont();
    tft.setTextColor(VERSION_GREY, BG);
    tft.setTextDatum(BR_DATUM);
    tft.drawString(Config::APP_VERSION, VERSION_X, VERSION_Y_LOW);
    tft.setTextDatum(TL_DATUM);

    // WLAN-Verbindungsaufbau NICHT-BLOCKIEREND starten.
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("Eiswolfs-iBoard");
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);
    WiFi.begin(deviceCfg.wifiSsid.c_str(), deviceCfg.wifiPassword.c_str());

    uint32_t startMs = millis();
    int16_t pulseDrawnX = PULSE_X_START;
    int16_t barDrawnW = 0;
    String lastStatusText = "";
    bool skipped = false;

    while (!skipped && millis() - startMs < Config::BOOT_SCREEN_MS) {
        uint32_t elapsed = millis() - startMs;

        // Ladebalken: nur das neue Stueck fuellen.
        float frac = (float)elapsed / (float)Config::BOOT_SCREEN_MS;
        if (frac > 1.0f) frac = 1.0f;
        int16_t targetW = (int16_t)(BAR_W * frac);
        if (targetW > barDrawnW) {
            tft.fillRect(BAR_X + barDrawnW, BAR_Y, targetW - barDrawnW, BAR_H, TFT_CYAN);
            barDrawnW = targetW;
        }

        // Puls-Linie: in den ersten BOOT_PULSE_MS von links nach rechts.
        if (pulseDrawnX < PULSE_X_END) {
            float pfrac = (float)elapsed / (float)Config::BOOT_PULSE_MS;
            if (pfrac > 1.0f) pfrac = 1.0f;
            int16_t targetX = PULSE_X_START + (int16_t)((PULSE_X_END - PULSE_X_START) * pfrac);
            if (targetX > pulseDrawnX) {
                drawPulseSegment(pulseDrawnX, targetX);
                pulseDrawnX = targetX;
            }
        }

        // WLAN-Statuszeile unten links - nur bei Aenderung neu zeichnen.
        String statusText;
        if (WiFi.status() == WL_CONNECTED) {
            statusText = "WLAN: " + WiFi.localIP().toString();
        } else if (elapsed >= Config::BOOT_SCREEN_MS - 300) {
            statusText = "WLAN: nicht verbunden";
        } else {
            statusText = "WLAN: verbinde...";
        }
        if (statusText != lastStatusText) {
            // Zentriert statt linksbuendig (Auftrag), schmale/kompakte
            // Bibliotheksschrift statt der grossen "klaren" Titelschrift.
            tft.fillRect(40, STATUS_Y - 10, 240, 12, BG);
            useBuiltinFont();
            int16_t statusW = tft.textWidth(statusText);
            tft.setTextColor(STATUS_GREY, BG);
            tft.setTextDatum(BC_DATUM);
            tft.drawString(statusText, Config::SCREEN_WIDTH / 2, STATUS_Y);
            tft.setTextDatum(TL_DATUM);
            lastStatusText = statusText;

            // Version weicht aus, wenn die jetzt zentrierte Statuszeile
            // (z.B. bei einer langen IP) zu nah an sie heranreicht
            // (Auftrag: mind. 8px Abstand, sonst eine Zeile hoeher ueber
            // den Balken).
            useBuiltinFont();
            int16_t versionW = tft.textWidth(Config::APP_VERSION);
            int16_t statusRightEdge = (int16_t)(Config::SCREEN_WIDTH / 2 + statusW / 2);
            int16_t versionLeftEdgeLow = (int16_t)(VERSION_X - versionW);
            bool needHigh = (statusRightEdge + VERSION_GAP_MIN) > versionLeftEdgeLow;
            if (needHigh != versionHigh) {
                int16_t oldY = versionHigh ? VERSION_Y_HIGH : VERSION_Y_LOW;
                tft.fillRect((int16_t)(VERSION_X - versionW - 2), (int16_t)(oldY - 12), (int16_t)(versionW + 4), 14, BG);
                versionHigh = needHigh;
                int16_t newY = versionHigh ? VERSION_Y_HIGH : VERSION_Y_LOW;
                tft.setTextColor(VERSION_GREY, BG);
                tft.setTextDatum(BR_DATUM);
                tft.drawString(Config::APP_VERSION, VERSION_X, newY);
                tft.setTextDatum(TL_DATUM);
            }
        }

        // Ueberspringen per kurzem Antippen.
        TouchCalib::Point tap;
        if (TouchCalib::wasTapped(tap)) {
            skipped = true;
        }

        delay(20);
    }

    // Endzustand sauber vervollstaendigen (Balken voll, Puls-Linie
    // komplett, finaler WLAN-Status), egal ob durch Zeitablauf oder
    // Ueberspringen beendet.
    if (barDrawnW < BAR_W) {
        tft.fillRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, BAR_H / 2, TFT_CYAN);
    }
    if (pulseDrawnX < PULSE_X_END) {
        drawPulseSegment(pulseDrawnX, PULSE_X_END);
    }

    // Tipp-Sperre: ca. 300ms lang alle Beruehrungen ignorieren, damit
    // der Skip-Tap nicht gleichzeitig einen Button der naechsten
    // Anzeige ausloest.
    uint32_t lockUntil = millis() + Config::BOOT_TAP_LOCKOUT_MS;
    while (millis() < lockUntil) {
        TouchCalib::Point p;
        TouchCalib::wasTapped(p); // Ergebnis bewusst verworfen
        delay(5);
    }
}


// Helligkeitsreglerseite (Auftrag "Helligkeitsregler fuers Display") -
// +/- Stepper statt Dreh-/Schiebe-Geste, da das Projekt bisher keinen
// Dragging-Slider-Widget-Code hat und +/- robust mit dem bestehenden
// Tap-Erkennungsmuster (TouchCalib::wasTapped()) funktioniert. Schritte
// von 10%, Untergrenze 10% (siehe applyBrightness()), damit das Menue nie
// unsichtbar/unbedienbar wird.
void runBrightnessMenu() {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t barX = 20, barY = 110, barW = (int16_t)(W - 40), barH = 24;
    Rect minusBtn = { 20, 160, 90, 44 };
    Rect plusBtn = { (int16_t)(W - 110), 160, 90, 44 };
    Rect backBtn = { 20, 212, (int16_t)(W - 40), 24 };

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        useBuiltinFont();
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(I18n::t(StringId::SETUP_BRIGHTNESS), W / 2, 30);
        String pctLabel = String(deviceCfg.brightness) + "%";
        tft.drawString(pctLabel, W / 2, 70);

        tft.drawRect(barX, barY, barW, barH, TFT_WHITE);
        int16_t filledW = (int16_t)((barW - 4) * (deviceCfg.brightness / 100.0f));
        tft.fillRect(barX + 2, barY + 2, filledW, barH - 4, TFT_YELLOW);

        tft.setTextDatum(TL_DATUM);
        drawButton(minusBtn, "-", TFT_CYAN);
        drawButton(plusBtn, "+", TFT_CYAN);
        drawButton(backBtn, I18n::t(StringId::MENU_BACK), TFT_GREEN);
    };

    render();

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            delay(10);
            continue;
        }
        if (minusBtn.contains(tap.x, tap.y) && deviceCfg.brightness > 10) {
            deviceCfg.brightness = max(10, deviceCfg.brightness - 10);
            Storage::save(deviceCfg);
            applyBrightness();
            render();
        } else if (plusBtn.contains(tap.x, tap.y) && deviceCfg.brightness < 100) {
            deviceCfg.brightness = min(100, deviceCfg.brightness + 10);
            Storage::save(deviceCfg);
            applyBrightness();
            render();
        } else if (backBtn.contains(tap.x, tap.y)) {
            return;
        }
    }
}

// Kleines Erklaer-Fenster (Auftrag "erklaere die Menues mit kleinen '?'")
// - Titel + mehrzeiliger Text (manuell per "\n" umgebrochen, siehe
// i18n.cpp-Kommentar), OK-Knopf unten. Blockierend, kein Rueckgabewert -
// der Aufrufer zeichnet seinen eigenen Screen danach selbst neu.
void showHelpPopup(const char* title, const char* body) {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;
    Rect okBtn = { (int16_t)(W / 2 - 60), (int16_t)(H - 44), 120, 32 };

    tft.fillScreen(TFT_BLACK);
    useBuiltinFont();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(title, W / 2, 24);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    int16_t y = 56;
    String bodyStr(body);
    int start = 0;
    while (start <= (int)bodyStr.length()) {
        int nl = bodyStr.indexOf('\n', start);
        String line = (nl == -1) ? bodyStr.substring(start) : bodyStr.substring(start, nl);
        tft.setCursor(10, y);
        tft.print(line);
        y += 18;
        if (nl == -1) break;
        start = nl + 1;
    }

    drawButton(okBtn, I18n::t(StringId::HELP_OK), TFT_GREEN);

    while (true) {
        TouchCalib::Point tap;
        if (TouchCalib::wasTapped(tap) && okBtn.contains(tap.x, tap.y)) {
            return;
        }
        delay(10);
    }
}

void runSetupMenu() {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t mainW = (int16_t)(W - 40 - 28 - 4);
    const int16_t helpX = (int16_t)(20 + mainW + 4);
    Rect wifiBtn = { 20, 50, mainW, 32 };
    Rect wifiHelp = { helpX, 50, 28, 32 };
    Rect calBtn = { 20, 86, mainW, 32 };
    Rect calHelp = { helpX, 86, 28, 32 };
    Rect brightBtn = { 20, 122, mainW, 32 };
    Rect brightHelp = { helpX, 122, 28, 32 };
    Rect wolBtn = { 20, 158, mainW, 32 };
    Rect wolHelp = { helpX, 158, 28, 32 };
    Rect backBtn = { 20, 194, (int16_t)(W - 40), 32 };

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        useBuiltinFont();
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(I18n::t(StringId::SETUP_TITLE), W / 2, 30);
        tft.setTextDatum(TL_DATUM);
        drawButton(wifiBtn, I18n::t(StringId::SETUP_WIFI_ONLY), TFT_CYAN);
        drawButton(wifiHelp, "?", TFT_YELLOW);
        drawButton(calBtn, I18n::t(StringId::SETUP_RECALIBRATE), TFT_CYAN);
        drawButton(calHelp, "?", TFT_YELLOW);
        drawButton(brightBtn, I18n::t(StringId::SETUP_BRIGHTNESS), TFT_CYAN);
        drawButton(brightHelp, "?", TFT_YELLOW);
        drawButton(wolBtn, I18n::t(StringId::SETUP_WOL_MAC), TFT_CYAN);
        drawButton(wolHelp, "?", TFT_YELLOW);
        drawButton(backBtn, I18n::t(StringId::MENU_BACK), TFT_ORANGE);
    };

    render();

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            delay(10);
            continue;
        }
        if (wifiHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::SETUP_WIFI_ONLY), I18n::t(StringId::SETUP_WIFI_HELP));
            render();
            continue;
        }
        if (calHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::SETUP_RECALIBRATE), I18n::t(StringId::SETUP_CAL_HELP));
            render();
            continue;
        }
        if (brightHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::SETUP_BRIGHTNESS), I18n::t(StringId::SETUP_BRIGHTNESS_HELP));
            render();
            continue;
        }
        if (wolHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::SETUP_WOL_MAC), I18n::t(StringId::SETUP_WOL_HELP));
            render();
            continue;
        }
        if (wifiBtn.contains(tap.x, tap.y)) {
            SetupFlow::runWifiOnly(tft, deviceCfg);
            return;
        }
        if (calBtn.contains(tap.x, tap.y)) {
            SetupFlow::runCalibrationOnly(tft, deviceCfg);
            return;
        }
        if (brightBtn.contains(tap.x, tap.y)) {
            runBrightnessMenu();
            render();
            continue;
        }
        if (wolBtn.contains(tap.x, tap.y)) {
            // MAC-Adresse fuer Wake-on-LAN (Auftrag) - NIE im Quellcode,
            // nur hier am Geraet eingegeben und auf SD/NVS gespeichert
            // (oeffentliches Repo). Ungueltige Eingabe wird einfach
            // verworfen (gleiches, bewusst schlichtes Muster wie bei den
            // anderen Eingabefeldern in diesem Projekt).
            bool cancelled = false;
            String text = SetupFlow::runTextEntry(tft, I18n::t(StringId::KB_WOL_MAC_PROMPT), cancelled);
            if (!cancelled) {
                text.trim();
                text.toUpperCase();
                uint8_t probe[6];
                if (text.length() == 0 || parseMacAddress(text, probe)) {
                    deviceCfg.wolMacAddress = text;
                    Storage::save(deviceCfg);
                }
            }
            render();
            continue;
        }
        if (backBtn.contains(tap.x, tap.y)) {
            return;
        }
    }
}

// Luefter-Boost-Menueseite (Auftrag "Luefter-Boost") - Dauer- und
// Drehzahl-Stufe fuer einen vom CYD gestarteten Boost (langes Antippen
// der FAN-Kachel, siehe sendFanCommand()). Unabhaengig von der
// Dauer-Einstellung im Mac-App-Menue, siehe CLAUDE.md.
void runFanSettingsMenu() {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t GAP = 6;
    const int16_t MARGIN = 10;
    const int16_t BTN_H = 40;

    const int16_t durW = (W - 2 * MARGIN - 2 * GAP) / 3;
    Rect dur30  = { MARGIN, 30, durW, BTN_H };
    Rect dur60  = { (int16_t)(MARGIN + durW + GAP), 30, durW, BTN_H };
    Rect dur120 = { (int16_t)(MARGIN + 2 * (durW + GAP)), 30, durW, BTN_H };
    Rect dur300 = { MARGIN, (int16_t)(30 + BTN_H + GAP), durW, BTN_H };
    Rect dur600 = { (int16_t)(MARGIN + durW + GAP), (int16_t)(30 + BTN_H + GAP), durW, BTN_H };
    Rect durTap = { (int16_t)(MARGIN + 2 * (durW + GAP)), (int16_t)(30 + BTN_H + GAP), durW, BTN_H };

    const int16_t speedY = 30 + 2 * (BTN_H + GAP) + 10;
    const int16_t spdW = (W - 2 * MARGIN - 3 * GAP) / 4;
    Rect spd2000 = { MARGIN, speedY, spdW, BTN_H };
    Rect spd3000 = { (int16_t)(MARGIN + (spdW + GAP)), speedY, spdW, BTN_H };
    Rect spd4000 = { (int16_t)(MARGIN + 2 * (spdW + GAP)), speedY, spdW, BTN_H };
    Rect spdMax  = { (int16_t)(MARGIN + 3 * (spdW + GAP)), speedY, spdW, BTN_H };

    Rect backBtn = { MARGIN, (int16_t)(speedY + BTN_H + 10), (int16_t)(W - 2 * MARGIN), 36 };

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        useBuiltinFont();
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(I18n::t(StringId::FAN_BOOST_TITLE), W / 2, 14);
        tft.setTextDatum(TL_DATUM);

        drawButton(dur30, I18n::t(StringId::FAN_BOOST_30SEC), TFT_CYAN, deviceCfg.fanBoostSec == 30);
        drawButton(dur60, I18n::t(StringId::FAN_BOOST_1MIN), TFT_CYAN, deviceCfg.fanBoostSec == 60);
        drawButton(dur120, I18n::t(StringId::FAN_BOOST_2MIN), TFT_CYAN, deviceCfg.fanBoostSec == 120);
        drawButton(dur300, I18n::t(StringId::FAN_BOOST_5MIN), TFT_CYAN, deviceCfg.fanBoostSec == 300);
        drawButton(dur600, I18n::t(StringId::FAN_BOOST_10MIN), TFT_CYAN, deviceCfg.fanBoostSec == 600);
        drawButton(durTap, I18n::t(StringId::FAN_BOOST_UNTIL_TAP), TFT_CYAN, deviceCfg.fanBoostSec == 0);

        drawButton(spd2000, "2000", TFT_YELLOW, deviceCfg.fanBoostRpm == "2000");
        drawButton(spd3000, "3000", TFT_YELLOW, deviceCfg.fanBoostRpm == "3000");
        drawButton(spd4000, "4000", TFT_YELLOW, deviceCfg.fanBoostRpm == "4000");
        drawButton(spdMax, I18n::t(StringId::FAN_BOOST_MAX_SPEED), TFT_YELLOW, deviceCfg.fanBoostRpm == "MAX");

        drawButton(backBtn, I18n::t(StringId::MENU_BACK), TFT_GREEN);
    };

    render();

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            delay(10);
            continue;
        }

        int newSec = deviceCfg.fanBoostSec;
        if (dur30.contains(tap.x, tap.y)) newSec = 30;
        else if (dur60.contains(tap.x, tap.y)) newSec = 60;
        else if (dur120.contains(tap.x, tap.y)) newSec = 120;
        else if (dur300.contains(tap.x, tap.y)) newSec = 300;
        else if (dur600.contains(tap.x, tap.y)) newSec = 600;
        else if (durTap.contains(tap.x, tap.y)) newSec = 0;

        String newRpm = deviceCfg.fanBoostRpm;
        if (spd2000.contains(tap.x, tap.y)) newRpm = "2000";
        else if (spd3000.contains(tap.x, tap.y)) newRpm = "3000";
        else if (spd4000.contains(tap.x, tap.y)) newRpm = "4000";
        else if (spdMax.contains(tap.x, tap.y)) newRpm = "MAX";

        if (newSec != deviceCfg.fanBoostSec || newRpm != deviceCfg.fanBoostRpm) {
            deviceCfg.fanBoostSec = newSec;
            deviceCfg.fanBoostRpm = newRpm;
            Storage::save(deviceCfg);
            render();
        } else if (backBtn.contains(tap.x, tap.y)) {
            return;
        }
    }
}

// Hauptmenue (Auftrag "Menue beim Antippen von Eiswolfs iBoard") - loest das
// fruehere reine Schrift-Umschalten per Kopfzeilen-Tap ab. Grosse,
// fingerfreundliche Tasten (>=40px hoch). Empfang/Timeout laufen im
// Hintergrund normal weiter, waehrend das Menue offen ist - der
// Aufrufer baut die Live-Anzeige nach dem Schliessen komplett neu auf.
void runMainMenu() {
    Rect autoBtn = { 10, 32, 96, 40 };
    Rect wifiBtn = { 111, 32, 96, 40 };
    Rect usbBtn = { 212, 32, 96, 40 };
    Rect pixelBtn = { 10, 90, 145, 40 };
    Rect clearBtn = { 165, 90, 145, 40 };
    // "Luefter" und "Einrichtung" teilen sich eine Reihe (je halbe
    // Breite, Auftrag "Luefter-Boost") - vorher nahm "Einrichtung" die
    // volle Breite ein.
    Rect fanBtn = { 10, 136, 145, 40 };
    Rect setupBtn = { 165, 136, 145, 40 };
    // Sprachauswahl (Auftrag "Sprache DE/EN") teilt sich die letzte Reihe
    // mit "Zurueck" (je halbe Breite) statt einer eigenen Reihe - keine
    // weitere Zeile mehr frei (320x240, Querformat).
    Rect langBtn = { 10, 182, 145, 40 };
    Rect backBtn = { 165, 182, 145, 40 };

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        useBuiltinFont();
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setCursor(10, 4);
        tft.print(I18n::t(StringId::MENU_HEADER));
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 20);
        tft.print(I18n::t(StringId::MENU_CONNECTION_LABEL));

        drawButton(autoBtn, I18n::t(StringId::MENU_CONNECTION_AUTO), TFT_CYAN, connectionMode == "auto");
        drawButton(wifiBtn, I18n::t(StringId::MENU_CONNECTION_WIFI), TFT_CYAN, connectionMode == "wifi");
        drawButton(usbBtn, I18n::t(StringId::MENU_CONNECTION_USB), TFT_CYAN, connectionMode == "usb");

        // BUGFIX (Auftrag "Zeichenrest hinter Schrift:"): drawButton()
        // setzt bei einer GEFUELLTEN Taste (z.B. aktives "USB") die
        // Textfarbe inkl. HINTERGRUND auf die Akzentfarbe - das blieb
        // bis zum naechsten print() aktiv und erzeugte einen farbigen
        // Kasten hinter "Schrift:", aber nur wenn die zuletzt gezeichnete
        // Verbindungs-Taste gerade die aktive (gefuellte) war. Deshalb
        // hier explizit zuruecksetzen statt den Zustand vom letzten
        // drawButton()-Aufruf zu erben.
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 78);
        tft.print(I18n::t(StringId::MENU_FONT_LABEL));
        drawButton(pixelBtn, I18n::t(StringId::MENU_FONT_PIXEL), TFT_YELLOW, currentFamily == FontFamily::Pixel);
        drawButton(clearBtn, I18n::t(StringId::MENU_FONT_CLEAR), TFT_YELLOW, currentFamily == FontFamily::Clear);

        drawButton(fanBtn, I18n::t(StringId::MENU_FAN_BUTTON), TFT_ORANGE);
        drawButton(setupBtn, I18n::t(StringId::MENU_SETUP_BUTTON), TFT_ORANGE);
        String langCode = I18n::currentLanguage();
        langCode.toUpperCase();
        String langLabel = String(I18n::t(StringId::MENU_LANGUAGE_LABEL)) + langCode;
        drawButton(langBtn, langLabel.c_str(), TFT_PURPLE);
        drawButton(backBtn, I18n::t(StringId::MENU_BACK), TFT_GREEN);
    };

    render();

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            delay(10);
            continue;
        }

        if (autoBtn.contains(tap.x, tap.y) && connectionMode != "auto") {
            connectionMode = "auto";
            deviceCfg.source = "auto";
            Storage::save(deviceCfg);
            render();
        } else if (wifiBtn.contains(tap.x, tap.y) && connectionMode != "wifi") {
            connectionMode = "wifi";
            deviceCfg.source = "wifi";
            Storage::save(deviceCfg);
            render();
        } else if (usbBtn.contains(tap.x, tap.y) && connectionMode != "usb") {
            connectionMode = "usb";
            deviceCfg.source = "usb";
            Storage::save(deviceCfg);
            render();
        } else if (pixelBtn.contains(tap.x, tap.y) && currentFamily != FontFamily::Pixel) {
            currentFamily = FontFamily::Pixel;
            deviceCfg.font = "pixel";
            Storage::save(deviceCfg);
            computeRowFonts();
            render();
        } else if (clearBtn.contains(tap.x, tap.y) && currentFamily != FontFamily::Clear) {
            currentFamily = FontFamily::Clear;
            deviceCfg.font = "clear";
            Storage::save(deviceCfg);
            computeRowFonts();
            render();
        } else if (fanBtn.contains(tap.x, tap.y)) {
            runFanSettingsMenu();
            render();
        } else if (setupBtn.contains(tap.x, tap.y)) {
            runSetupMenu();
            render();
        } else if (langBtn.contains(tap.x, tap.y)) {
            deviceCfg.language = (I18n::currentLanguage() == "de") ? "en" : "de";
            I18n::setLanguage(deviceCfg.language);
            Storage::save(deviceCfg);
            render();
        } else if (backBtn.contains(tap.x, tap.y)) {
            return;
        }
    }
}

} // namespace

void setup() {
    Serial.begin(115200);

    if (Config::DEBUG_LOG) {
        esp_reset_reason_t reason = esp_reset_reason();
        Serial.printf("[Eiswolfs iBoard] Boot, Reset-Grund=%d\n", (int)reason);
    }

    tft.init();
    tft.setRotation(1); // Querformat
    tft.invertDisplay(true); // dieser CYD-Displaytreiber (ILI9341_2) braucht das fuer korrekte Farben
    tft.fillScreen(TFT_BLACK);
    pinMode(TFT_BL, OUTPUT);

    TouchCalib::begin();
    Storage::begin();
    Serial.printf("[Eiswolfs iBoard] SD-Karte: %s\n", Storage::sdAvailable() ? "gefunden" : "NICHT gefunden");

    bool hasCfg = Storage::load(deviceCfg);
    if (hasCfg) {
        TouchCalib::setCalibration(deviceCfg.touchCal);
    }
    currentFamily = (deviceCfg.font == "clear") ? FontFamily::Clear : FontFamily::Pixel;
    connectionMode = deviceCfg.source;
    if (connectionMode != "wifi" && connectionMode != "usb") connectionMode = "auto";
    I18n::setLanguage(deviceCfg.language);
    applyBrightness();

    // Notausgang: Touch beim Einschalten ca. 3s gehalten -> Einrichtung
    // erzwingen, auch wenn eine (ggf. schlechte) Kalibrierung vorhanden ist.
    bool forceSetup = false;
    if (TouchCalib::rawPoint().touched) {
        uint32_t holdStart = millis();
        bool stillHeld = true;
        while (millis() - holdStart < Config::BOOT_HOLD_SETUP_MS) {
            if (!TouchCalib::rawPoint().touched) {
                stillHeld = false;
                break;
            }
            delay(20);
        }
        forceSetup = stillHeld;
    }

    if (!hasCfg || forceSetup) {
        // Keine gueltige Konfiguration (oder Notausgang gehalten) ->
        // direkt die Ersteinrichtung, OHNE Bootscreen (siehe Auftrag).
        SetupFlow::runFullSetup(tft, deviceCfg);
        TouchCalib::setCalibration(deviceCfg.touchCal);
        I18n::setLanguage(deviceCfg.language);
        WiFi.setSleep(false);
    } else {
        runBootScreen();
    }

    // "Keine SD-Karte"-Hinweis erscheint bewusst ERST NACH dem
    // Bootscreen (bzw. nach der Ersteinrichtung) - der Bootscreen
    // selbst erscheint unabhaengig vom SD-Status immer, siehe Auftrag.
    if (!Storage::sdAvailable()) {
        useBuiltinFont();
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(10, 10);
        tft.println("Keine SD-Karte gefunden.");
        tft.println("Einstellungen werden nur");
        tft.println("intern (NVS) gespeichert.");
        delay(2000);
    }

    udp.begin(Config::UDP_PORT);
    drawStaticLayout();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED && millis() - lastWifiRetryMs > Config::WIFI_RETRY_INTERVAL_MS) {
        lastWifiRetryMs = millis();
        WiFi.disconnect();
        WiFi.begin(deviceCfg.wifiSsid.c_str(), deviceCfg.wifiPassword.c_str());
        WiFi.setSleep(false);
    }

    pollUsbSerial();

    // Alle gerade wartenden UDP-Pakete in dieser Schleife komplett
    // leeren, nicht nur eines - nur der zuletzt eingetroffene Inhalt
    // zaehlt (siehe CLAUDE.md "Stolperfallen").
    int packetSize;
    while ((packetSize = udp.parsePacket()) > 0) {
        static char buf[512];
        IPAddress remoteIp = udp.remoteIP();
        uint16_t remotePort = udp.remotePort();
        int len = udp.read(buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            parsePacket(buf, len, false);
        }
        // Fuer den Luefter-Boost merken, wohin FAN-Befehle gehen sollen
        // (siehe sendFanCommand()) - derselbe Absender wie das Datenpaket.
        lastMacIp = remoteIp;
        haveMacIp = true;
        // Antwortet dem Absender mit der eigenen IP (unabhaengig vom
        // Parse-Erfolg/Modus) - Messung ergab, dass UDP-BROADCAST in
        // Alex' Netz fast komplett verschluckt wird (1 von 90 Paketen
        // kam an), UNICAST an diese Adresse aber zuverlaessig 100%
        // ankommt (siehe CLAUDE.md "Stolperfallen"). Die Mac-App lernt
        // daraus automatisch die CYD-IP und sendet danach zusaetzlich
        // per Unicast, ohne dass Alex etwas eintragen muss.
        if (WiFi.status() == WL_CONNECTED) {
            udp.beginPacket(remoteIp, remotePort);
            udp.print(WiFi.localIP().toString());
            udp.endPacket();
        }
    }

    // Welche Pakete fuer den Verbindungsstatus zaehlen, haengt vom
    // gewaehlten Modus ab - im Modus "usb" soll z.B. eine tote WLAN-
    // Verbindung nicht "verbunden" vortaeuschen, und umgekehrt.
    uint32_t lastAnyPacketMs;
    if (connectionMode == "wifi") lastAnyPacketMs = lastUdpPacketMs;
    else if (connectionMode == "usb") lastAnyPacketMs = lastUsbPacketMs;
    else lastAnyPacketMs = max(lastUdpPacketMs, lastUsbPacketMs);

    bool noData = (millis() - lastAnyPacketMs > Config::NO_DATA_TIMEOUT_MS) || lastAnyPacketMs == 0;

    if (noData != showingNoConnection) {
        showingNoConnection = noData;
        if (showingNoConnection) {
            drawNoConnectionScreen();
        } else {
            drawStaticLayout();
            lastDrawnValid = false;
        }
    }

    if (showingNoConnection) {
        TouchCalib::Point tap;
        if (TouchCalib::wasTapped(tap)) {
            if (setupButtonRect.contains(tap.x, tap.y)) {
                runMainMenu();
                drawNoConnectionScreen();
            }
        }
    } else {
        TouchCalib::Point tap;
        bool tapped = TouchCalib::wasTapped(tap);
        if (tapped && headerLeftRect.contains(tap.x, tap.y)) {
            runMainMenu();
            drawStaticLayout();
            lastDrawnValid = false;
        } else if (tapped && pwrTileRect.contains(tap.x, tap.y) && millis() >= pwrTapLockedUntilMs) {
            // Ruhemodus (Auftrag): einfaches Antippen der PWR-Kachel, kein
            // langes Halten. 3s-Sperre ab jetzt gegen prellende Touches.
            sendSleepCommand();
            pwrTapLockedUntilMs = millis() + PWR_TAP_LOCKOUT_MS;
            sleepCmdPending = true;
            sleepCmdSentMs = millis();
            sleepCmdBaselineAck = current.sleepAck;
            pwrFeedbackState = PwrFeedback::Ruhe;
            pwrFeedbackUntilMs = millis() + PWR_FEEDBACK_RUHE_MS;
            drawPwrFeedbackText(I18n::t(StringId::PWR_SLEEP_FEEDBACK));
        } else if (tapped && deviceCfg.wolMacAddress.length() > 0 &&
                   tileRect(TILE_CPU_TEMP).contains(tap.x, tap.y) && millis() >= wolTapLockedUntilMs &&
                   !cpuTempLongFired) {
            // Wake-on-LAN (Auftrag) - eigene Kachel/eigener Code-Pfad,
            // komplett unabhaengig von der PWR-Kachel (Alex' Wunsch: ein
            // falsch erkannter Touch darf nie den Mac schlafen schicken,
            // wenn eigentlich der Laptop gemeint war, und umgekehrt).
            // !cpuTempLongFired: unterdrueckt den Kurztipp, wenn GERADE
            // ein langes Halten (Shutdown) auf derselben Kachel feuerte -
            // sonst wuerde das Loslassen danach zusaetzlich WoL ausloesen.
            sendWolPacket();
            wolTapLockedUntilMs = millis() + WOL_TAP_LOCKOUT_MS;
            wolFeedbackActive = true;
            wolFeedbackUntilMs = millis() + WOL_FEEDBACK_MS;
            drawTileValue(TILE_CPU_TEMP, I18n::t(StringId::WOL_SENT_FEEDBACK), TFT_YELLOW, false, 0);
        }

        // Luefter-Boost: langes Antippen (Config::FAN_LONGPRESS_MS) der
        // FAN-Kachel startet/beendet ihn - kurzes Antippen tut bewusst
        // nichts (Auftrag), deshalb unabhaengig von wasTapped() ueber
        // den rohen, kontinuierlichen Touch-Zustand erkannt.
        {
            static bool fanPressActive = false;
            static bool fanLongFired = false;
            static uint32_t fanPressStartMs = 0;
            static bool fanReleasePending = false;
            static uint32_t fanReleaseStartMs = 0;
            TouchCalib::Point raw = TouchCalib::rawPoint();
            if (raw.touched) {
                fanReleasePending = false;
                TouchCalib::Point mp = TouchCalib::mappedPoint();
                Rect fanOuter = tileRect(TILE_FAN);
                if (!fanPressActive) {
                    if (fanOuter.contains(mp.x, mp.y)) {
                        fanPressActive = true;
                        fanLongFired = false;
                        fanPressStartMs = millis();
                    }
                } else if (!fanLongFired && millis() - fanPressStartMs >= Config::FAN_LONGPRESS_MS) {
                    fanLongFired = true;
                    sendFanCommand(!current.fanBoost);
                }
            } else if (fanPressActive) {
                // Entprellung (Bugfix): ein einzelner kurzer Ausssetzer
                // des resistiven Touch-Sensors mitten in einer langen
                // Beruehrung wurde vorher sofort als "losgelassen"
                // gewertet - dadurch konnte EIN Halten zwei Lang-
                // Antippen-Ereignisse ausloesen (Boost startete und
                // endete sofort wieder). Ein "touched=false" muss jetzt
                // 150ms am Stueck anhalten, bevor die Beruehrung
                // tatsaechlich als beendet gilt.
                if (!fanReleasePending) {
                    fanReleasePending = true;
                    fanReleaseStartMs = millis();
                } else if (millis() - fanReleaseStartMs >= 150) {
                    fanPressActive = false;
                    fanLongFired = false;
                    fanReleasePending = false;
                }
            }
        }

        // Windows-Laptop herunterfahren: langes Antippen derselben
        // CPU-Temp-Kachel wie WoL (Auftrag "selbe wie an nur longpress") -
        // exakt dasselbe Muster wie beim FAN-Block oben, inkl. 150ms-
        // Entprellung. cpuTempLongFired ist dateiweit (siehe oben), damit
        // der WoL-Kurztipp-Handler weiter oben ihn mitlesen kann.
        {
            static bool pressActive = false;
            static uint32_t pressStartMs = 0;
            static bool releasePending = false;
            static uint32_t releaseStartMs = 0;
            TouchCalib::Point raw = TouchCalib::rawPoint();
            if (raw.touched) {
                releasePending = false;
                TouchCalib::Point mp = TouchCalib::mappedPoint();
                Rect cpuTempOuter = tileRect(TILE_CPU_TEMP);
                if (!pressActive) {
                    if (cpuTempOuter.contains(mp.x, mp.y) && deviceCfg.wolMacAddress.length() > 0) {
                        pressActive = true;
                        cpuTempLongFired = false;
                        pressStartMs = millis();
                    }
                } else if (!cpuTempLongFired && millis() - pressStartMs >= Config::FAN_LONGPRESS_MS) {
                    cpuTempLongFired = true;
                    sendShutdownCommand();
                    wolFeedbackActive = true;
                    wolFeedbackUntilMs = millis() + WOL_FEEDBACK_MS;
                    drawTileValue(TILE_CPU_TEMP, I18n::t(StringId::SHUTDOWN_SENT_FEEDBACK), TFT_RED, false, 0);
                }
            } else if (pressActive) {
                if (!releasePending) {
                    releasePending = true;
                    releaseStartMs = millis();
                } else if (millis() - releaseStartMs >= 150) {
                    pressActive = false;
                    releasePending = false;
                    // cpuTempLongFired bleibt bewusst bis hierhin stehen -
                    // erst JETZT, nachdem auch der kurze-Tipp-Handler oben
                    // (derselbe Loslass-Zyklus) laengst durchgelaufen ist,
                    // wird es zurueckgesetzt, bereit fuer die naechste
                    // Beruehrung.
                    cpuTempLongFired = false;
                }
            }
        }

        // Bestaetigung eines gesendeten FAN-Befehls abwarten (siehe
        // sendFanCommand()) - kommt binnen Config::FAN_CMD_CONFIRM_TIMEOUT_MS
        // keine passende Aenderung von current.fanBoost, kurz "n/v" zeigen.
        if (fanCmdPending) {
            if (current.fanBoost == fanCmdWantBoost) {
                fanCmdPending = false;
            } else if (millis() - fanCmdSentMs > Config::FAN_CMD_CONFIRM_TIMEOUT_MS) {
                fanCmdPending = false;
                showFanStripeNotAvailable();
            }
        }

        // Ruhemodus (Auftrag): RUHE-Anzeigefenster lief ab -> zurueck zur
        // normalen Wattzahl (unabhaengig davon, ob schon bestaetigt).
        if (pwrFeedbackState == PwrFeedback::Ruhe && millis() >= pwrFeedbackUntilMs) {
            pwrFeedbackState = PwrFeedback::None;
            drawHeaderPwrValueNew(true);
        }
        // Bestaetigung/Timeout eines gesendeten SLEEP-Befehls - unabhaengig
        // vom RUHE-Anzeigefenster oben, gleiches Muster wie fanCmdPending.
        // Jede Aenderung von current.sleepAck (egal welcher neue Wert)
        // zaehlt als Bestaetigung.
        if (sleepCmdPending) {
            if (current.sleepAck != sleepCmdBaselineAck) {
                sleepCmdPending = false;
            } else if (millis() - sleepCmdSentMs > Config::FAN_CMD_CONFIRM_TIMEOUT_MS) {
                sleepCmdPending = false;
                pwrFeedbackState = PwrFeedback::NotAvailable;
                pwrFeedbackUntilMs = millis() + PWR_FEEDBACK_NA_MS;
                drawPwrFeedbackText(I18n::t(StringId::PWR_NOT_AVAILABLE));
            }
        }
        // "n/v"-Anzeigefenster lief ab -> zurueck zur normalen Wattzahl.
        if (pwrFeedbackState == PwrFeedback::NotAvailable && millis() >= pwrFeedbackUntilMs) {
            pwrFeedbackState = PwrFeedback::None;
            drawHeaderPwrValueNew(true);
        }

        // Wake-on-LAN-Anzeigefenster lief ab -> zurueck zur normalen
        // CPU-Temp-Anzeige. Direktes Neuzeichnen aus current (nicht ueber
        // updateDisplay()s Aenderungs-Check), da lastDrawn.cpuTempC trotz
        // ausgeblendeter Anzeige schon auf dem aktuellen Stand war und sich
        // seitdem evtl. gar nicht veraendert hat.
        if (wolFeedbackActive && millis() >= wolFeedbackUntilMs) {
            wolFeedbackActive = false;
            if (current.hasCpuTemp) {
                drawTileValue(TILE_CPU_TEMP, String((int)current.cpuTempC) + "C", colorForTemp(current.cpuTempC), false, 0);
            } else {
                drawTileValue(TILE_CPU_TEMP, "--", TFT_DARKGREY, false, 0);
            }
        }

        if (current.valid) {
            updateDisplay();
        }
    }

    delay(20);
}
