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
#include "cmd_auth.h"
#include "fonts/PixelFont20.h"
#include "apple_rgb565.h"
#include "icon_fan.h"
#include "ui_text.h"
#include "fonts/ui_bold_font.h"
// FreeSansBold9/18/24pt7b werden von TFT_eSPIs eigenem gfxfont.h bei
// LOAD_GFXFF bereits automatisch eingebunden (siehe gfxfont.h in der
// TFT_eSPI-Bibliothek) - ein eigenes #include hier wuerde zu
// "redefinition"-Fehlern fuehren, die Typen/Objekte sind schon da.
// Alexa (Release v1.2.0): immer einkompiliert, aber nur aktiv, wenn im Menue
// Einrichtung > Alexa eingeschaltet (Standard AUS, siehe alexaActive). Die
// internen [FAUXMO]-Debug-Logs (DEBUG_FAUXMO) sind im oeffentlichen Build aus.
#include <fauxmoESP.h>

namespace {

TFT_eSPI tft = TFT_eSPI();
WiFiUDP udp;
Storage::DeviceConfig deviceCfg;

// Alexa-Emulation (Auftraege "Alexa-Test 1/2", "Alexa Aus", Release v1.2.0),
// siehe setup()/loop() weiter unten. Geraete-ID/Seriennummer leitet
// fauxmoESP intern bereits aus WiFi.macAddress() ab (siehe
// fauxmoESP::addDevice()) - bleibt dadurch nach einem Neustart stabil.
// alexaActive wird in setup() EINMAL aus deviceCfg.alexaEnabled gesetzt und
// aendert sich bis zum naechsten Neustart nicht (Aenderung im Alexa-Menue
// loest einen Neustart aus). false = kein Server, kein SSDP, kein Port 80.
fauxmoESP fauxmo;
bool alexaActive = false;
bool alexaStarted = false;
String alexaNameActive;
// Auftrag "Alexa-Test 2" - der onSetState-Callback laeuft im AsyncTCP-
// Kontext, NICHT im Haupt-loop()-Task. Er setzt hier nur ein einfaches
// Merker-Flag (keine eigene Synchronisierung noetig, da genau EIN
// Schreiber/EIN Leser je Flag, dasselbe einfache Muster, das der
// restliche Code hier ueberall sonst mit millis()-Vergleichen statt
// Mutexen verwendet) - der eigentliche WoL-Versand UND das Zuruecksetzen
// auf "aus" (Taster-Verhalten) passieren erst in loop(), siehe dort.
// So bekommt Alexa ihre HTTP-Antwort sofort, unabhaengig davon, wie
// lange sendWolPacket()/UDP brauchen wuerden.
volatile bool alexaWolRequested = false;
uint32_t alexaOffRevertAtMs = 0; // 0 = kein Taster-Ruecksetzen ausstehend
// Auftrag "Alexa Aus" - gleiches Merker-Flag-Muster wie alexaWolRequested.
volatile bool alexaShutdownRequested = false;

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
// Auftrag "Umlaute und Umrandung", Schritt A2: eigene, umlautfaehige
// Schrift statt FreeSansBold9/18/24pt7b (reines ASCII) - gleiche
// Pixelgroessen (M-Hoehe 13/26/34px empirisch kalibriert, siehe
// tools/generate_ui_font.py), dadurch bleibt die optische Groesse am
// Hauptscreen unveraendert. FreeSansBold*pt7b bleiben als Includes
// bestehen (kommen automatisch mit LOAD_GFXFF/gfxfont.h), werden aber
// nirgends mehr referenziert.
const FontChoice CLEAR_CANDIDATES[] = {
    { &UiBold24, 1 },
    { &UiBold18, 1 },
    { &UiBold9, 1 },
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

// Frueher der eingebaute GLCD-Font (Font 1, reines ASCII) - seit
// Auftrag "Umlaute und Umrandung", Schritt A3 die eigene, umlautfaehige
// GFXFF-Schrift (UiBoldSmall, siehe tools/generate_ui_font.py). Name
// bewusst beibehalten (geringstmoegliche Aenderung an allen
// Aufrufstellen), nicht mehr woertlich "eingebaut".
void useBuiltinFont() {
    applyFont({ &UiBoldSmall, 1 });
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
    // Auftrag "Monitor-Umschaltung" - additiv wie sleepAck oben
    // (sender.py::SenderThread.monitor_ack_counter), fehlt es (alte
    // Mac-App-Version), bleibt der Wert einfach 0.
    int monitorAck = 0;
};

Metrics current;
Metrics lastDrawn;
bool lastDrawnValid = false;
bool showingNoConnection = false;

// Zwei Seiten (Auftrag "Controls-Screen"): Hauptscreen (reine Anzeige)
// und Controls (alle Bedienfunktionen). Wischen wechselt zwischen
// beiden, siehe loop(). Startet immer auf Main (nach dem Bootscreen).
// Devices (Auftrag "WoL-Geraete") - dritte Seite, nur erreichbar per
// Wischen von Controls aus, nur vorhanden wenn wolDeviceCount>=2 (siehe
// loop()). Kein automatischer Sprung dorthin vom Menue aus.
enum class Page : uint8_t { Main, Controls, Devices };
Page currentPage = Page::Main;
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

// Ruhemodus (Auftrag "Ruhemodus"): die Mac-Kachel auf Controls sendet
// "SLEEP <Zaehler> <HMAC>", genau wie sendFanCommand() oben auf
// demselben Weg/Port. sleepCmdBaselineAck merkt sich current.sleepAck
// im Moment des Sendens - jede Aenderung davon (egal in welche
// Richtung, der Zaehler steigt nur) gilt als Bestaetigung, siehe
// handleControlsPage(). Variablen bewusst dateiweit (nicht mehr an
// eine feste Hauptscreen-Kachel gebunden, seit Auftrag "Controls-
// Screen" - die eigentliche Flash-/Sperr-Logik je Kachel liegt jetzt
// bei den Ctrl*-Variablen dort).
bool sleepCmdPending = false;
uint32_t sleepCmdSentMs = 0;
int sleepCmdBaselineAck = 0;
// Dauer der kurzen Rueckmeldefenster (Auftrag "Controls-Screen") -
// Namen/Werte aus der fruehreren PWR-/WoL-Rueckmeldung uebernommen.
constexpr uint32_t PWR_FEEDBACK_RUHE_MS = 2000;
constexpr uint32_t PWR_FEEDBACK_NA_MS = 1500;
constexpr uint32_t WOL_FEEDBACK_MS = 1500;

// "Mac schlaeft absichtlich" (Auftrag "Display aus im Ruhezustand",
// Schritt 4 des Folgeauftrags "Ruhezustand-Fix" ueberarbeitet auf
// Bestaetigungs-Basis statt "10s keine Daten") - nur im RAM, NIE auf
// SD/NVS geschrieben. Ablauf: Requested (bis zu SLEEP_ACK_TIMEOUT_MS auf
// die sleep_ack-Bestaetigung der Mac-App warten) -> Confirmed (Bestaetigung
// da, SLEEP_DISPLAY_OFF_DELAY_MS Verzoegerung) -> Active (Beleuchtung aus) ->
// optional kurz durch Beruehrung geweckt (macSleepWokenAtMs!=0, Beleuchtung
// an, zeigt "Keine Verbindung zum Mac") -> zurueck zu Active (dunkel) ODER
// Off (Daten kommen ANHALTEND - siehe WAKE_SUSTAIN_MS - normales
// Dashboard). NICHT zu verwechseln mit sleepCmdPending oben, das nur die
// gleiche Befehls-BESTAETIGUNG (sleep_ack) fuer das kurze Controls-Feedback
// (Kachel gruen/rot) prueft - beide lesen denselben sleepCmdBaselineAck.
enum class MacSleepState : uint8_t { Off, Requested, Confirmed, Active };
MacSleepState macSleepState = MacSleepState::Off;
uint32_t macSleepRequestedAtMs = 0;
uint32_t macSleepConfirmedAtMs = 0; // Zeitpunkt der sleep_ack-Bestaetigung (Requested -> Confirmed)
// Momentaufnahme von lastAnyPacketMs (siehe loop()) beim Eintritt in
// Active - jede Abweichung davon gilt als "neue Daten angekommen",
// unabhaengig vom absoluten Millis()-Wert (ueberlaufsicher, gleiches
// Subtraktions-Prinzip wie ueberall sonst in dieser Datei).
uint32_t macSleepBaselineLastPacketMs = 0;
// "Anhaltende Daten"-Erkennung in Active (Schritt 4 Ruhezustand-Fix): ein
// einzelnes, isoliertes Paket (z.B. aus einem DarkWake) soll die
// Beleuchtung NICHT wieder einschalten. macSleepWakeCandidateSinceMs==0
// heisst "kein laufender Daten-Streak"; sobald seit Streak-Beginn
// WAKE_SUSTAIN_MS vergangen sind, gilt der Mac als wirklich wach.
uint32_t macSleepWakeCandidateSinceMs = 0;
uint32_t macSleepLastNewPacketMs = 0;
uint32_t macSleepWokenAtMs = 0; // 0 = nicht per Beruehrung geweckt (dauerhaft dunkel)
constexpr uint32_t SLEEP_ACK_TIMEOUT_MS = 3000;
constexpr uint32_t SLEEP_DISPLAY_OFF_DELAY_MS = 3000;
constexpr uint32_t WAKE_SUSTAIN_MS = 5000;
constexpr uint32_t TOUCH_WAKE_MS = 15000;
// Nicht vom Auftrag benannt, aber fuer WAKE_SUSTAIN_MS technisch noetig:
// laengste erlaubte Luecke zwischen zwei "neuen" Paketen, bevor ein
// laufender Daten-Streak als abgebrochen gilt (Mac war nur kurz im
// DarkWake aktiv und schlaeft gleich wieder) - deutlich groesser als das
// normale 1s-Sendeintervall der Mac-App, aber klar unter WAKE_SUSTAIN_MS.
constexpr uint32_t WAKE_GAP_RESET_MS = 2000;

Rect setupButtonRect = { 10, 200, 130, 28 }; // Button "Menue" auf dem "Keine Verbindung"-Screen
Rect headerRightRect = { 150, 0, 168, HEADER_H }; // nur im alten Layout benutzt

// WoL/Shutdown-Buttonspalte auf "Keine Verbindung zum Mac" (Auftrag
// "WoL-Geraete", vorher nur fuer den einen Acer-Button) - rechte
// Spalte, spiegelbildlich zum Menue-Button (320-10-130=180), Eintrag
// Nr. 1 ganz unten (heutige/bisherige Position y=200), weitere darueber
// mit gleichem Abstand. slot 0 = unten (Geraet 0), slot 3 = oben
// (Geraet 3).
Rect noConnDeviceButtonRect(uint8_t slot) {
    constexpr int16_t bottomY = 200, h = 28, gap = 6, stepY = h + gap;
    return Rect{ 180, (int16_t)(bottomY - slot * stepY), 130, h };
}

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

// Welche Pakete fuer den Verbindungsstatus zaehlen, haengt vom
// gewaehlten Modus ab (siehe loop()) - eigene Funktion statt Duplikat
// (Auftrag "Display aus im Ruhezustand" braucht denselben Wert auch
// ausserhalb von loop(), siehe handleControlsPage()/updateMacSleepState()).
uint32_t computeLastAnyPacketMs() {
    if (connectionMode == "wifi") return lastUdpPacketMs;
    if (connectionMode == "usb") return lastUsbPacketMs;
    return max(lastUdpPacketMs, lastUsbPacketMs);
}

// Luefter-Boost (Auftrag "Luefter-Boost") - schickt "FAN BOOST <sek>
// <stufe> <token>" bzw. "FAN AUTO <token>" auf dem gerade aktiven Weg
// (USB-Zeile oder UDP-Unicast an die gelernte Mac-IP, Port
// Config::FAN_CMD_UDP_PORT - NICHT der Datenport). Merkt sich danach,
// welchen Zustand es erwartet, damit loop() auf die Bestaetigung im
// naechsten Datenpaket warten kann (siehe Config::FAN_CMD_CONFIRM_TIMEOUT_MS).
void sendFanCommand(bool wantBoost) {
    String base;
    if (wantBoost) {
        base = "FAN BOOST " + String(deviceCfg.fanBoostSec) + " " + deviceCfg.fanBoostRpm;
    } else {
        base = "FAN AUTO";
    }
    String line = CmdAuth::signCommand(base, deviceCfg);

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

// Ruhemodus (Auftrag "Ruhemodus") - schickt "SLEEP <Zaehler> <HMAC>" auf
// demselben Weg/Port wie sendFanCommand() oben (Config::FAN_CMD_UDP_PORT,
// NICHT der Datenport). Siehe cmd_auth.h/.cpp fuer das Format (Auftrag
// "Sicherheit und Lizenzen", Teil 2).
void sendSleepCommand() {
    String line = CmdAuth::signCommand("SLEEP", deviceCfg);

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

// Seit Auftrag "WoL-Geraete" parametrisiert (vorher immer
// deviceCfg.wolMacAddress) - jedes Geraet hat seine eigene MAC.
void sendWolPacket(const String& macStr) {
    uint8_t mac[6];
    if (!parseMacAddress(macStr, mac)) return;
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

// Auftrag "Monitor-Umschaltung" - nicht-blockierender Zustandsautomat,
// der VOR dem eigentlichen WoL-Paket (bei konfiguriertem
// monitorInputOnPowerOn UND erreichbarem Mac) einen MONITOR-Befehl
// schickt und bis zu MONITOR_ACK_TIMEOUT_MS auf die Quittung
// (current.monitorAck) wartet - siehe triggerWolForDevice() unten und
// die Abfrage in loop(). KEIN delay() - der Zustand lebt ausschliesslich
// zwischen zwei loop()-Durchlaeufen.
enum class MonitorPreWolState { Idle, WaitingForAck };
MonitorPreWolState monitorPreWolState = MonitorPreWolState::Idle;
uint32_t monitorCmdSentMs = 0;
int monitorAckBaselineForWol = 0;
uint8_t monitorPendingWolDeviceIdx = 0;

// Sendet den "MONITOR n"-Befehl auf demselben Weg/Port wie SLEEP/FAN
// (siehe sendSleepCommand() weiter unten als Vorbild) - eigene kleine
// Funktion, da sowohl triggerWolForDevice() (frueh im Code) als auch
// loop() (spaet im Code) das Senden selbst NICHT nochmal braucht, nur
// das Warten auf die Quittung - der Befehl wird nur HIER, einmalig
// beim Ausloesen, verschickt.
void sendMonitorCommand(uint8_t logicalInput) {
    String line = CmdAuth::signCommand("MONITOR " + String(logicalInput), deviceCfg);
    if (usbIsActiveSource()) {
        Serial.print(line);
        Serial.print("\n");
    } else if (WiFi.status() == WL_CONNECTED && haveMacIp) {
        udp.beginPacket(lastMacIp, Config::FAN_CMD_UDP_PORT);
        udp.print(line);
        udp.endPacket();
    }
}

// Zentrale Stelle fuer JEDEN Weg, der ein WoL-Geraet einschaltet
// (Controls-Kachel/"Keine Verbindung"-Button ueber handleDeviceWidgets(),
// UND der Alexa-Flow in loop(), siehe Auftrag "Alexa-Test 2") - ersetzt
// die bisherigen direkten sendWolPacket()-Aufrufe an beiden Stellen.
// Ist kein Monitor-Umschalten konfiguriert ODER der Mac nicht erreichbar
// (schlaeft/App aus - computeLastAnyPacketMs() deckt beides gleich ab),
// geht das WoL-Paket SOFORT, exakt wie bisher. Sonst wird zuerst der
// MONITOR-Befehl gesendet und das WoL-Paket ueber den obigen
// Zustandsautomaten (siehe loop()) bis zu 2s verzoegert.
void triggerWolForDevice(uint8_t deviceIdx) {
    const Storage::WolDevice& d = deviceCfg.wolDevices[deviceIdx];
    uint32_t lastPacketMs = computeLastAnyPacketMs();
    bool macReachable = (lastPacketMs != 0) && (millis() - lastPacketMs <= Config::NO_DATA_TIMEOUT_MS);

    if (d.monitorInputOnPowerOn == 0 || !macReachable || monitorPreWolState != MonitorPreWolState::Idle) {
        if (d.monitorInputOnPowerOn != 0 && !macReachable) {
            Serial.println("[Eiswolfs iBoard] Monitor: Mac nicht erreichbar, nur WoL");
        }
        sendWolPacket(d.mac);
        return;
    }

    sendMonitorCommand(d.monitorInputOnPowerOn);
    monitorAckBaselineForWol = current.monitorAck;
    monitorCmdSentMs = millis();
    monitorPendingWolDeviceIdx = deviceIdx;
    monitorPreWolState = MonitorPreWolState::WaitingForAck;
    Serial.printf("[Eiswolfs iBoard] Monitor: Befehl %u gesendet, warte auf Quittung\n",
                  (unsigned)d.monitorInputOnPowerOn);
}

// Windows-Laptop herunterfahren (Auftrag) - eigener Befehl an den
// Hintergrund-Listener (windows/), der auf WIN_SHUTDOWN_UDP_PORT lauscht
// und bei gueltigem Token "shutdown /s" ausfuehrt. Nutzt denselben Token
// wie Luefter-Boost/Ruhemodus (FAN_BOOST_TOKEN) - kein zweites Geheimnis.
void sendShutdownCommand() {
    if (WiFi.status() != WL_CONNECTED) return;
    String line = CmdAuth::signCommand("SHUTDOWN", deviceCfg);
    udp.beginPacket(IPAddress(255, 255, 255, 255), Config::WIN_SHUTDOWN_UDP_PORT);
    udp.print(line);
    udp.endPacket();
}

// Resync (Auftrag "Sicherheit und Lizenzen", Teil 2b) - fuer den seltenen
// Fall, dass der eigene Zaehler verloren ging (z.B. Werksreset). Sendet
// einen ganz normal signierten "RESYNC"-Befehl an BEIDE Empfaenger
// (Mac-Port UND Windows-Port) - die eigentliche Sonderbehandlung
// (neuen, nicht zwingend hoeheren Zaehler trotzdem akzeptieren) passiert
// NUR empfaengerseitig, hinter einem dortigen, zeitlich begrenzten
// manuellen Schalter (siehe windows/listener/main.go, mac/fan_boost.py).
void sendResyncCommand() {
    String line = CmdAuth::signCommand("RESYNC", deviceCfg);
    if (usbIsActiveSource()) {
        Serial.print(line);
        Serial.print("\n");
    }
    if (WiFi.status() == WL_CONNECTED) {
        if (haveMacIp) {
            udp.beginPacket(lastMacIp, Config::FAN_CMD_UDP_PORT);
            udp.print(line);
            udp.endPacket();
        }
        udp.beginPacket(IPAddress(255, 255, 255, 255), Config::WIN_SHUTDOWN_UDP_PORT);
        udp.print(line);
        udp.endPacket();
    }
}

// Ueber UiText::draw() (Auftrag "Ueberschriften und WLAN-Zurueck") -
// jede Tastenbeschriftung bekommt dadurch automatisch die Breitenmessung
// mit Schrumpf-/Kuerz-Sicherheitsnetz (vorher: feste UiBoldSmall-Groesse
// ohne jede Pruefung, ein zu langes Label - z.B. eine laengere
// englische Uebersetzung - haette unbemerkt ueberlaufen koennen).
void drawButton(const Rect& r, const char* label, uint16_t color, bool filled = false) {
    uint16_t bg = filled ? color : TFT_BLACK;
    uint16_t fg = filled ? TFT_BLACK : color;
    tft.fillRect(r.x, r.y, r.w, r.h, bg);
    if (!filled) tft.drawRect(r.x, r.y, r.w, r.h, color);
    UiText::Box box = { (int16_t)(r.x + 2), (int16_t)(r.y + 2), (int16_t)(r.w - 4), (int16_t)(r.h - 4) };
    UiText::draw(tft, box, label, fg, bg, UiText::Align::Center, 2);
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

// Ueberarbeitet (Auftrag "Ueberschriften und WLAN-Zurueck"): die
// zweizeilige rote Ueberschrift wurde vorher per tft.setTextSize(2) auf
// der bereits 24px hohen UiBoldSmall-Schrift hochskaliert (verboten,
// siehe Auftrag - erzeugt sichtbar gezackte/verpixelte Kanten UND lief
// bei festen Y=40/64 oben an und rechts aus dem Bild) - jetzt ueber
// UiText::draw(), das selbst die groesstmoegliche ERZEUGTE Groesse
// waehlt, die in die Box passt, nie skaliert.
//
// Vorab-Deklaration (Auftrag "WoL-Geraete"): die eigentliche Definition
// von drawDeviceWidget() liegt weiter unten (braucht drawPowerIcon()/
// fillRoundRectTop() etc.), wird aber hier schon fuer die Geraete-
// Buttonspalte gebraucht - das Enum selbst hat keine Abhaengigkeiten,
// steht deshalb direkt hier (keine zweite Definition weiter unten).
enum class DeviceWidgetStyle : uint8_t { Tile, Button };
void drawDeviceWidget(DeviceWidgetStyle style, const Rect& r, uint8_t deviceIdx);

void drawNoConnectionScreen() {
    tft.fillScreen(TFT_BLACK);
#ifdef LAYOUT_DEBUG
    UiText::beginScreen("KeineVerbindung");
#endif
    useBuiltinFont();
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(I18n::t(StringId::APP_NAME), 4, 2);

    UiText::draw(tft, { 8, 28, (int16_t)(Config::SCREEN_WIDTH - 16), 26 },
                 I18n::t(StringId::NO_CONNECTION_LINE1), TFT_RED, TFT_BLACK);
    UiText::draw(tft, { 8, 54, (int16_t)(Config::SCREEN_WIDTH - 16), 26 },
                 I18n::t(StringId::NO_CONNECTION_LINE2), TFT_RED, TFT_BLACK);

    // Textspalte links bleibt schmaler als frueher, sobald mindestens
    // EIN Geraete-Button existiert (Auftrag "WoL-Geraete", Schritt 5) -
    // die Buttonspalte beginnt bei x=180, also bis x=172 Luft lassen
    // (reicht fuer bis zu 4 Buttons uebereinander, siehe
    // noConnDeviceButtonRect()). Ohne jedes Geraet bleibt die volle
    // Breite wie vor diesem Auftrag erhalten.
    bool hasDevices = deviceCfg.wolDeviceCount > 0;
    int16_t textW = hasDevices ? (int16_t)(172 - 8) : (int16_t)(Config::SCREEN_WIDTH - 16);
    String ssidLine = String(I18n::t(StringId::WIFI_LABEL_PREFIX)) +
                       (WiFi.status() == WL_CONNECTED ? WiFi.SSID() : deviceCfg.wifiSsid);
    String ipLine = String(I18n::t(StringId::IP_LABEL_PREFIX)) +
                     (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("-"));
    String statusLine = String(I18n::t(StringId::STATUS_LABEL_PREFIX)) +
                         (WiFi.status() == WL_CONNECTED ? I18n::t(StringId::STATUS_CONNECTED) : I18n::t(StringId::STATUS_NOT_CONNECTED));
    UiText::draw(tft, { 8, 88, textW, 17 }, ssidLine, TFT_WHITE, TFT_BLACK, UiText::Align::Left);
    UiText::draw(tft, { 8, 107, textW, 17 }, ipLine, TFT_WHITE, TFT_BLACK, UiText::Align::Left);
    UiText::draw(tft, { 8, 126, textW, 17 }, statusLine, TFT_WHITE, TFT_BLACK, UiText::Align::Left);

    drawButton(setupButtonRect, I18n::t(StringId::MENU_BUTTON), TFT_YELLOW);
    // Geraete-Buttonspalte (Auftrag "WoL-Geraete") - Eintrag 0 unten,
    // weitere darueber, kein Button wenn die Liste leer ist.
    for (uint8_t i = 0; i < deviceCfg.wolDeviceCount; i++) {
        drawDeviceWidget(DeviceWidgetStyle::Button, noConnDeviceButtonRect(i), i);
    }
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
    // BUGFIX (Auftrag "Umlaute und Umrandung", Alex' Live-Feedback am
    // Geraet - "USB oben rechts" zeigte Buchstabenmatsch): useBuiltinFont()
    // zeichnet seit diesem Auftrag mit der neuen, deutlich hoeheren
    // GFXFF-Schrift - der extra-knappe Loeschbereich hier (10px) war noch
    // auf den winzigen alten Font 1 zugeschnitten, die neue Schrift ragte
    // heraus und wurde nur teilweise geloescht ("Geisterpixel"). "USB"/
    // "WLAN" sind reines ASCII (keine Umlaute noetig) - bleiben deshalb
    // bewusst auf dem echten, winzigen eingebauten Font 1, NICHT ueber
    // useBuiltinFont() (das ist jetzt die groessere Menue-Schrift).
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(usbNow ? TFT_GREEN : TFT_DARKGREY, TFT_BLACK);
    tft.drawString(sourceLabel, Config::SCREEN_WIDTH - 4, 1);
    tft.setTextDatum(TL_DATUM);

    lastUsb = usbNow;
    lastForced = forced;
    hasLast = true;
}

// Watt-Kachel in der Kopfzeile - seit Auftrag "Controls-Screen" reine
// Anzeige (keine Ruhemodus-Rueckmeldung mehr hier, das Antippen sitzt
// jetzt auf der Mac-Kachel auf Controls, siehe handleControlsPage()).
void drawHeaderPwrValueNew(bool force) {
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
    if (full || (int)current.cpuTempC != (int)lastDrawn.cpuTempC || current.hasCpuTemp != lastDrawn.hasCpuTemp) {
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
    m.monitorAck = doc["monitor_ack"] | 0;

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

        // Auftrag "Controls-Optik und USB-Sync", Teil B: das CYD muss
        // "IBOARD?" auch waehrend des 7s-Bootscreens beantworten, sonst
        // landet eine Anfrage genau in diesem Fenster ins Leere (live
        // nachgemessen - siehe CLAUDE.md).
        pollUsbSerial();

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
        pollUsbSerial();
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
        UiText::drawHeader(tft, I18n::t(StringId::SETUP_BRIGHTNESS), false);
        useBuiltinFont();
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
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
            // Auftrag "Controls-Optik und USB-Sync", Teil B: "IBOARD?"
            // auch waehrend offener Menues beantworten (siehe Boot-
            // screen-Kommentar weiter oben fuer den Hintergrund).
            pollUsbSerial();
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

// Vorab-Deklaration (Auftrag "WoL-Geraete") - Definition weiter unten,
// nach pickRandomDeviceColor()/drawDeviceWidget() (braucht beide),
// wird aber hier schon im Einrichtung-Untermenue aufgerufen.
void runWolDevicesScreen();

// Kleines Erklaer-Fenster (Auftrag "erklaere die Menues mit kleinen '?'")
// - Titel + mehrzeiliger Text (manuell per "\n" umgebrochen, siehe
// i18n.cpp-Kommentar), OK-Knopf unten. Blockierend, kein Rueckgabewert -
// der Aufrufer zeichnet seinen eigenen Screen danach selbst neu.
void showHelpPopup(const char* title, const char* body) {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t H = Config::SCREEN_HEIGHT;
    Rect okBtn = { (int16_t)(W / 2 - 60), (int16_t)(H - 44), 120, 32 };

    tft.fillScreen(TFT_BLACK);
    UiText::drawHeader(tft, title, false);

    int16_t y = 42;
    String bodyStr(body);
    int start = 0;
    while (start <= (int)bodyStr.length()) {
        int nl = bodyStr.indexOf('\n', start);
        String line = (nl == -1) ? bodyStr.substring(start) : bodyStr.substring(start, nl);
        UiText::draw(tft, { 8, y, (int16_t)(W - 16), 18 }, line, TFT_CYAN, TFT_BLACK, UiText::Align::Left);
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
        pollUsbSerial();
        delay(10);
    }
}

// Alexa-Name bereinigen (Release v1.2.0): nur Buchstaben A-Z/a-z, Ziffern,
// Leerzeichen und Bindestrich (Alexa- und XML-sicher - fauxmoESP setzt den
// Namen unmaskiert in die Hue-JSON-/XML-Antworten ein, "&", "<", Anfuehrungszeichen und
// Umlaute wuerden dort Probleme machen). Mehrfach-Leerzeichen werden
// zusammengefasst, Anfang/Ende getrimmt, auf ALEXA_NAME_MAX_LEN gekuerzt.
String sanitizeAlexaName(const String& in) {
    String out;
    bool lastSpace = true; // fuehrende Leerzeichen verwerfen
    for (unsigned i = 0; i < in.length() && out.length() < Config::ALEXA_NAME_MAX_LEN; i++) {
        char c = in[i];
        bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
        if (c == ' ') {
            if (!lastSpace) { out += ' '; lastSpace = true; }
        } else if (ok) {
            out += c;
            lastSpace = false;
        }
    }
    out.trim();
    return out;
}

// Alexa-Menue (Release v1.2.0, unter Einrichtung): Ein/Aus (Standard AUS),
// Name des ersten WoL-Geraets fuer Alexa, der bestehende Schalter "Alexa:
// Ausschalten" (hierher verschoben) und ein Hinweis. Aenderungen werden
// sofort auf SD/NVS gespeichert. Wirksam werden Ein/Aus und Name erst nach
// einem Neustart (der Dienst wird in setup() entweder gar nicht oder
// komplett gestartet) - beim Verlassen mit Aenderung kommt eine Meldung
// und das Geraet startet neu. "Alexa: Ausschalten" wirkt sofort.
void runAlexaMenu() {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t mainW = (int16_t)(W - 40 - 28 - 4);
    const int16_t helpX = (int16_t)(20 + mainW + 4);
    constexpr int16_t ROW_H = 28;
    constexpr int16_t ROW_STEP = 34;
    constexpr int16_t ROW_TOP = 40;
    Rect enBtn = { 20, (int16_t)(ROW_TOP + 0 * ROW_STEP), mainW, ROW_H };
    Rect enHelp = { helpX, (int16_t)(ROW_TOP + 0 * ROW_STEP), 28, ROW_H };
    Rect nameBtn = { 20, (int16_t)(ROW_TOP + 1 * ROW_STEP), mainW, ROW_H };
    Rect nameHelp = { helpX, (int16_t)(ROW_TOP + 1 * ROW_STEP), 28, ROW_H };
    Rect sdnBtn = { 20, (int16_t)(ROW_TOP + 2 * ROW_STEP), mainW, ROW_H };
    Rect sdnHelp = { helpX, (int16_t)(ROW_TOP + 2 * ROW_STEP), 28, ROW_H };
    const int16_t hintY = (int16_t)(ROW_TOP + 3 * ROW_STEP + 6);
    Rect backBtn = { 20, 200, (int16_t)(W - 40), 28 };

    const bool enabledAtStart = deviceCfg.alexaEnabled;
    const String nameAtStart = deviceCfg.alexaName;

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        UiText::drawHeader(tft, I18n::t(StringId::ALEXA_MENU_BUTTON), false);
        drawButton(enBtn, I18n::t(StringId::ALEXA_ENABLE_LABEL), TFT_CYAN, deviceCfg.alexaEnabled);
        drawButton(enHelp, "?", TFT_YELLOW);
        String nameLabel = String(I18n::t(StringId::ALEXA_NAME_LABEL)) + deviceCfg.alexaName;
        drawButton(nameBtn, nameLabel.c_str(), TFT_CYAN);
        drawButton(nameHelp, "?", TFT_YELLOW);
        drawButton(sdnBtn, I18n::t(StringId::ALEXA_SHUTDOWN_MENU), TFT_CYAN, deviceCfg.alexaShutdownEnabled);
        drawButton(sdnHelp, "?", TFT_YELLOW);
        // Hinweis (zwei Zeilen, "\n" im Text), mit UiText::draw gezeichnet
        String hint(I18n::t(StringId::ALEXA_HINT));
        int nl = hint.indexOf('\n');
        String l1 = (nl < 0) ? hint : hint.substring(0, nl);
        String l2 = (nl < 0) ? String("") : hint.substring(nl + 1);
        UiText::draw(tft, { 20, hintY, (int16_t)(W - 40), 18 }, l1, TFT_LIGHTGREY, TFT_BLACK, UiText::Align::Left);
        UiText::draw(tft, { 20, (int16_t)(hintY + 18), (int16_t)(W - 40), 18 }, l2, TFT_LIGHTGREY, TFT_BLACK, UiText::Align::Left);
        drawButton(backBtn, I18n::t(StringId::MENU_BACK), TFT_ORANGE);
    };

    render();

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            pollUsbSerial();
            delay(10);
            continue;
        }
        if (enHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::ALEXA_ENABLE_LABEL), I18n::t(StringId::ALEXA_ENABLE_HELP));
            render();
            continue;
        }
        if (nameHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::ALEXA_NAME_PROMPT), I18n::t(StringId::ALEXA_NAME_HELP));
            render();
            continue;
        }
        if (sdnHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::ALEXA_SHUTDOWN_MENU), I18n::t(StringId::ALEXA_SHUTDOWN_HELP));
            render();
            continue;
        }
        if (enBtn.contains(tap.x, tap.y)) {
            deviceCfg.alexaEnabled = !deviceCfg.alexaEnabled;
            Storage::save(deviceCfg);
            render();
            continue;
        }
        if (nameBtn.contains(tap.x, tap.y)) {
            String draft = deviceCfg.alexaName;
            while (true) {
                bool cancelled = false;
                String n = SetupFlow::runTextEntry(tft, I18n::t(StringId::ALEXA_NAME_PROMPT), cancelled, draft.c_str());
                if (cancelled) break;
                String trimmed = n;
                trimmed.trim();
                String clean = sanitizeAlexaName(n);
                if (clean != trimmed) {
                    showHelpPopup(I18n::t(StringId::ALEXA_NAME_PROMPT), I18n::t(StringId::ALEXA_NAME_FIXED));
                }
                if (clean.length() == 0) {
                    showHelpPopup(I18n::t(StringId::ALEXA_NAME_PROMPT), I18n::t(StringId::WOL_EMPTY_NAME));
                    draft = deviceCfg.alexaName;
                    continue;
                }
                deviceCfg.alexaName = clean;
                Storage::save(deviceCfg);
                break;
            }
            render();
            continue;
        }
        if (sdnBtn.contains(tap.x, tap.y)) {
            // Bisheriger Schalter "Alexa: Ausschalten" (Auftrag "Alexa Aus"),
            // sofort gespeichert, wirkt ohne Neustart.
            deviceCfg.alexaShutdownEnabled = !deviceCfg.alexaShutdownEnabled;
            Storage::save(deviceCfg);
            render();
            continue;
        }
        if (backBtn.contains(tap.x, tap.y)) {
            if (deviceCfg.alexaEnabled != enabledAtStart || deviceCfg.alexaName != nameAtStart) {
                showHelpPopup(I18n::t(StringId::ALEXA_MENU_BUTTON), I18n::t(StringId::ALEXA_RESTART_TEXT));
                tft.fillScreen(TFT_BLACK);
                delay(200);
                ESP.restart();
            }
            return;
        }
    }
}

void runSetupMenu() {
    const int16_t W = Config::SCREEN_WIDTH;
    const int16_t mainW = (int16_t)(W - 40 - 28 - 4);
    const int16_t helpX = (int16_t)(20 + mainW + 4);
    // Auftrag "Alexa Aus": 6. Zeile dazugekommen - Zeilenhoehe/-abstand
    // etwas verkleinert (28->24px, Schritt 32->28px), damit weiterhin
    // alles inkl. Zurueck-Taste auf den 240px Bildschirm passt.
    constexpr int16_t ROW_H = 24;
    constexpr int16_t ROW_STEP = 28;
    constexpr int16_t ROW_TOP = 38;
    Rect wifiBtn = { 20, (int16_t)(ROW_TOP + 0 * ROW_STEP), mainW, ROW_H };
    Rect wifiHelp = { helpX, (int16_t)(ROW_TOP + 0 * ROW_STEP), 28, ROW_H };
    Rect calBtn = { 20, (int16_t)(ROW_TOP + 1 * ROW_STEP), mainW, ROW_H };
    Rect calHelp = { helpX, (int16_t)(ROW_TOP + 1 * ROW_STEP), 28, ROW_H };
    Rect brightBtn = { 20, (int16_t)(ROW_TOP + 2 * ROW_STEP), mainW, ROW_H };
    Rect brightHelp = { helpX, (int16_t)(ROW_TOP + 2 * ROW_STEP), 28, ROW_H };
    Rect wolBtn = { 20, (int16_t)(ROW_TOP + 3 * ROW_STEP), mainW, ROW_H };
    Rect wolHelp = { helpX, (int16_t)(ROW_TOP + 3 * ROW_STEP), 28, ROW_H };
    Rect resyncBtn = { 20, (int16_t)(ROW_TOP + 4 * ROW_STEP), mainW, ROW_H };
    Rect resyncHelp = { helpX, (int16_t)(ROW_TOP + 4 * ROW_STEP), 28, ROW_H };
    Rect alexaBtn = { 20, (int16_t)(ROW_TOP + 5 * ROW_STEP), mainW, ROW_H };
    Rect alexaHelp = { helpX, (int16_t)(ROW_TOP + 5 * ROW_STEP), 28, ROW_H };
    Rect backBtn = { 20, (int16_t)(ROW_TOP + 6 * ROW_STEP + 2), (int16_t)(W - 40), 26 };

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        UiText::drawHeader(tft, I18n::t(StringId::SETUP_TITLE), false);
        drawButton(wifiBtn, I18n::t(StringId::SETUP_WIFI_ONLY), TFT_CYAN);
        drawButton(wifiHelp, "?", TFT_YELLOW);
        drawButton(calBtn, I18n::t(StringId::SETUP_RECALIBRATE), TFT_CYAN);
        drawButton(calHelp, "?", TFT_YELLOW);
        drawButton(brightBtn, I18n::t(StringId::SETUP_BRIGHTNESS), TFT_CYAN);
        drawButton(brightHelp, "?", TFT_YELLOW);
        drawButton(wolBtn, I18n::t(StringId::SETUP_WOL_MAC), TFT_CYAN);
        drawButton(wolHelp, "?", TFT_YELLOW);
        drawButton(resyncBtn, I18n::t(StringId::SETUP_RESYNC), TFT_CYAN);
        drawButton(resyncHelp, "?", TFT_YELLOW);
        // Release v1.2.0: eigener Menuepunkt "Alexa" (oeffnet runAlexaMenu()),
        // gefuellt = Alexa eingeschaltet (Standard aus). Der fruehere Schalter
        // "Alexa: Ausschalten" ist in dieses Untermenue verschoben.
        drawButton(alexaBtn, I18n::t(StringId::ALEXA_MENU_BUTTON), TFT_CYAN, deviceCfg.alexaEnabled);
        drawButton(alexaHelp, "?", TFT_YELLOW);
        drawButton(backBtn, I18n::t(StringId::MENU_BACK), TFT_ORANGE);
    };

    render();

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            // Auftrag "Controls-Optik und USB-Sync", Teil B: "IBOARD?"
            // auch waehrend offener Menues beantworten (siehe Boot-
            // screen-Kommentar weiter oben fuer den Hintergrund).
            pollUsbSerial();
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
        if (resyncHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::SETUP_RESYNC), I18n::t(StringId::SETUP_RESYNC_HELP));
            render();
            continue;
        }
        if (alexaHelp.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::ALEXA_MENU_BUTTON), I18n::t(StringId::ALEXA_MENU_HELP));
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
            // Seit Auftrag "WoL-Geraete": Liste statt Einzel-MAC-Eingabe
            // - MAC-Adressen NIE im Quellcode, nur hier am Geraet
            // eingegeben und auf SD/NVS gespeichert (oeffentliches Repo).
            runWolDevicesScreen();
            render();
            continue;
        }
        if (resyncBtn.contains(tap.x, tap.y)) {
            sendResyncCommand();
            showHelpPopup(I18n::t(StringId::SETUP_RESYNC), I18n::t(StringId::RESYNC_SENT_FEEDBACK));
            render();
            continue;
        }
        if (alexaBtn.contains(tap.x, tap.y)) {
            runAlexaMenu();
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

    // ROW0_Y = UiText::CONTENT_Y (Auftrag "Ueberschriften und WLAN-
    // Zurueck", Schritt 2: einheitliche 34px-Kopfzeile) statt vorher 30 -
    // die erste Tastenreihe begann sonst 4px VOR dem Ende der neuen
    // Kopfzeile.
    const int16_t ROW0_Y = UiText::CONTENT_Y;
    const int16_t durW = (W - 2 * MARGIN - 2 * GAP) / 3;
    Rect dur30  = { MARGIN, ROW0_Y, durW, BTN_H };
    Rect dur60  = { (int16_t)(MARGIN + durW + GAP), ROW0_Y, durW, BTN_H };
    Rect dur120 = { (int16_t)(MARGIN + 2 * (durW + GAP)), ROW0_Y, durW, BTN_H };
    Rect dur300 = { MARGIN, (int16_t)(ROW0_Y + BTN_H + GAP), durW, BTN_H };
    Rect dur600 = { (int16_t)(MARGIN + durW + GAP), (int16_t)(ROW0_Y + BTN_H + GAP), durW, BTN_H };
    Rect durTap = { (int16_t)(MARGIN + 2 * (durW + GAP)), (int16_t)(ROW0_Y + BTN_H + GAP), durW, BTN_H };

    const int16_t speedY = ROW0_Y + 2 * (BTN_H + GAP) + 10;
    const int16_t spdW = (W - 2 * MARGIN - 3 * GAP) / 4;
    Rect spd2000 = { MARGIN, speedY, spdW, BTN_H };
    Rect spd3000 = { (int16_t)(MARGIN + (spdW + GAP)), speedY, spdW, BTN_H };
    Rect spd4000 = { (int16_t)(MARGIN + 2 * (spdW + GAP)), speedY, spdW, BTN_H };
    Rect spdMax  = { (int16_t)(MARGIN + 3 * (spdW + GAP)), speedY, spdW, BTN_H };

    Rect backBtn = { MARGIN, (int16_t)(speedY + BTN_H + 10), (int16_t)(W - 2 * MARGIN), 36 };

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
        UiText::drawHeader(tft, I18n::t(StringId::FAN_BOOST_TITLE), false);

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
            // Auftrag "Controls-Optik und USB-Sync", Teil B: "IBOARD?"
            // auch waehrend offener Menues beantworten (siehe Boot-
            // screen-Kommentar weiter oben fuer den Hintergrund).
            pollUsbSerial();
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
// Ueberarbeitet (Auftrag "Ueberschriften und WLAN-Zurueck"): Titel lief
// vorher oben an und wurde unten von "Verbindung:" ueberdeckt (beide
// UiBoldSmall-hoch [24px], Titel bei y=4..28, Label bei y=20..44 -
// echte Ueberlappung, nicht nur optisch knapp). "Schrift:" war aus
// demselben Grund fast vollstaendig von den Tasten darunter verdeckt
// (Label y=78..102, Tasten y=90..130). Jetzt: einheitliche 34px-
// Kopfzeile (UiText::drawHeader) + eigene, kleinere Boxen (ca. 13px)
// fuer die beiden Labels, mit garantiertem Abstand zu Titel/Tasten.
void runMainMenu() {
    Rect autoBtn = { 10, 55, 96, 36 };
    Rect wifiBtn = { 111, 55, 96, 36 };
    Rect usbBtn = { 212, 55, 96, 36 };
    Rect pixelBtn = { 10, 110, 145, 36 };
    Rect clearBtn = { 165, 110, 145, 36 };
    // "Luefter" und "Einrichtung" teilen sich eine Reihe (je halbe
    // Breite, Auftrag "Luefter-Boost") - vorher nahm "Einrichtung" die
    // volle Breite ein.
    Rect fanBtn = { 10, 148, 145, 36 };
    Rect setupBtn = { 165, 148, 145, 36 };
    // Sprachauswahl (Auftrag "Sprache DE/EN") teilt sich die letzte Reihe
    // mit "Zurueck" (je halbe Breite) statt einer eigenen Reihe - keine
    // weitere Zeile mehr frei (320x240, Querformat).
    Rect langBtn = { 10, 186, 145, 36 };
    Rect backBtn = { 165, 186, 145, 36 };

    auto render = [&]() {
        tft.fillScreen(TFT_BLACK);
#ifdef LAYOUT_DEBUG
        UiText::beginScreen("Hauptmenue");
#endif
        UiText::drawHeader(tft, I18n::t(StringId::MENU_HEADER), false);
        UiText::draw(tft, { 10, UiText::CONTENT_Y, 300, 15 },
                     I18n::t(StringId::MENU_CONNECTION_LABEL), TFT_WHITE, TFT_BLACK, UiText::Align::Left);

        drawButton(autoBtn, I18n::t(StringId::MENU_CONNECTION_AUTO), TFT_CYAN, connectionMode == "auto");
        drawButton(wifiBtn, I18n::t(StringId::MENU_CONNECTION_WIFI), TFT_CYAN, connectionMode == "wifi");
        drawButton(usbBtn, I18n::t(StringId::MENU_CONNECTION_USB), TFT_CYAN, connectionMode == "usb");

        UiText::draw(tft, { 10, 93, 300, 15 },
                     I18n::t(StringId::MENU_FONT_LABEL), TFT_WHITE, TFT_BLACK, UiText::Align::Left);
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
            // Auftrag "Controls-Optik und USB-Sync", Teil B: "IBOARD?"
            // auch waehrend offener Menues beantworten (siehe Boot-
            // screen-Kommentar weiter oben fuer den Hintergrund).
            pollUsbSerial();
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

// ---------------------------------------------------------------------
// Controls-Screen (Auftrag "Controls-Screen") - zweite Seite, Wischen
// nach links vom Hauptscreen (siehe loop()). Vier Kacheln, alle
// bisherigen Bedienfunktionen des Hauptscreens wandern hierher. Nutzt
// bewusst DIESELBEN Zustandsvariablen wie vorher (pwrFeedbackState/
// sleepCmdPending/wolFeedbackActive/cpuTempLongFired/fanCmdPending usw.,
// siehe oben) - nur WOHER der Touch kommt und WOHIN gezeichnet wird,
// aendert sich. Befehle/Protokoll selbst (sendSleepCommand() usw.)
// bleiben unveraendert.
// ---------------------------------------------------------------------

constexpr int16_t CTRL_TILE_W = 146, CTRL_TILE_H = 80, CTRL_STRIPE_H = 26, CTRL_RADIUS = 7;

// Einzige Stelle, die RGB in RGB565 umrechnet (Auftrag "Controls-Optik
// und USB-Sync", Teil A1) - exakt dieselbe Formel wie TFT_eSPI::
// color565() (R 5 Bit/G 6 Bit/B 5 Bit, nicht gleichmaessig!), hier als
// eigene constexpr-Funktion dupliziert, weil die Mitgliedsfunktion der
// TFT_eSPI-Instanz nicht zur Compile-Zeit auswertbar ist. Verhindert
// genau den Fehler, der zum falschen Seitenpunkt-Grau fuehrte (siehe
// CLAUDE.md Farbtabelle): ein auf den ersten Blick "neutral aussehender"
// Hex-Wert wie 0x5555 ist bei RGB565 NICHT automatisch Grau, weil die
// drei Kanaele unterschiedlich viele Bits haben.
constexpr uint16_t ctrlColor(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

// BUGFIX (Alex' Korrektur anhand des bestaetigten Entwurfs, 03.11.):
// der Kachelkoerper ist im Entwurf reines Schwarz, nicht #1A1A1A -
// verschmilzt dadurch optisch mit dem Bildschirmhintergrund, sichtbar
// bleibt nur der farbige Streifen oben. TFT_BLACK direkt statt einer
// eigenen ctrlColor()-Konstante, da identisch mit dem Seitenhintergrund.
constexpr uint16_t CTRL_BODY_BG = TFT_BLACK;
constexpr uint16_t CTRL_DOT_INACTIVE = ctrlColor(85, 85, 85); // #555555 - war faelschlich 0x5555 (siehe oben)

enum CtrlTileIdx { CTRL_ACER = 0, CTRL_MAC, CTRL_FAN, CTRL_MENU, CTRL_TILE_COUNT };

Rect ctrlTileRects[CTRL_TILE_COUNT] = {
    { 10, 40, CTRL_TILE_W, CTRL_TILE_H },
    { 164, 40, CTRL_TILE_W, CTRL_TILE_H },
    { 10, 128, CTRL_TILE_W, CTRL_TILE_H },
    { 164, 128, CTRL_TILE_W, CTRL_TILE_H },
};

// Rueckmeldezustand je interaktiver Kachel (Mac/Luefter) - "color"
// 0 = kein aktives Feedback, sonst die Flash-Farbe gerade auf der
// Kachel. Gemeinsame kleine Struktur statt doppelter Kopie, siehe
// startCtrlFeedback()/loop(). Die Acer-Kachel (jetzt WoL-Geraet Nr. 1)
// nutzt seit Auftrag "WoL-Geraete" das neue, geraete-indizierte
// deviceFbColor[]/deviceFbUntilMs[] weiter unten statt einer eigenen
// CtrlFeedback-Instanz - gemeinsam mit den Geraete-Seiten-Kacheln und
// den Buttons auf "Keine Verbindung".
struct CtrlFeedback {
    uint16_t color = 0;
    uint32_t untilMs = 0;
};
CtrlFeedback macFb, fanFb;
// 3s-Sperre je Kachel nach einem gesendeten Befehl (Auftrag) - eigene
// Variablen, damit ein prellender Touch auf einer Kachel nicht
// innerhalb dieser 3s einen zweiten Befehl derselben Kachel ausloest.
uint32_t macTapLockedUntilMs = 0, fanTapLockedUntilMs = 0;
constexpr uint32_t CTRL_TAP_LOCKOUT_MS = 3000;

// ---------------------------------------------------------------------
// WoL-Geraete (Auftrag "WoL-Geraete") - gemeinsame Tap/Halten-Logik fuer
// ALLE Geraete-Buttons auf allen drei Screens (Controls-Kachel 1,
// Geraete-Seiten-Kacheln 2-4, "Keine Verbindung"-Buttons), siehe
// handleDeviceWidgets() weiter unten. Ein einzelner gemeinsamer
// Touch-Zustand reicht, weil das Touch-Panel ohnehin nur EINE
// Beruehrung gleichzeitig kennt - genau wie vorher ein einzelnes
// acerLongFired fuer die einzige Acer-Kachel reichte.
// ---------------------------------------------------------------------
struct DeviceTouchState {
    bool pressActive = false;
    uint32_t pressStartMs = 0;
    TouchCalib::Point pressStartPoint;
    int16_t maxMove = 0;
    bool longFired = false;
    bool releasePending = false;
    uint32_t releaseStartMs = 0;
    int8_t pressedDeviceIdx = -1;
};
DeviceTouchState deviceTouch;
// Rueckmeldung/Sperre JE GERAET (nicht je Screen) - deviceCfg.wolDevices[]-
// Index, screen-unabhaengig (ein Geraet hat ueberall dieselbe Sperre/
// denselben Feedback-Zustand, auch wenn es nur auf EINEM Screen auf
// einmal sichtbar ist).
uint16_t deviceFbColor[Config::MAX_WOL_DEVICES] = {0, 0, 0, 0};
uint32_t deviceFbUntilMs[Config::MAX_WOL_DEVICES] = {0, 0, 0, 0};
uint32_t deviceTapLockedUntilMs[Config::MAX_WOL_DEVICES] = {0, 0, 0, 0};

// Kachel-Positionen auf der neuen Seite "Geraete" - exakt dieselben drei
// Positionen wie Controls' CTRL_ACER/CTRL_MAC/CTRL_FAN (Auftrag), aber
// als EIGENES Array (nicht ctrlTileRects wiederverwendet), damit die
// Icon-/Farblogik der Geraete-Kacheln nicht an die idx-abhaengigen
// Controls-Funktionen (drawCtrlIcon() unterscheidet nach Kachel-Index)
// gebunden ist - sonst haette z.B. Geraet 3 an CTRL_FAN-Position
// faelschlich das Luefter-Symbol bekommen.
Rect deviceTileRects[3] = {
    { 10, 40, CTRL_TILE_W, CTRL_TILE_H },
    { 164, 40, CTRL_TILE_W, CTRL_TILE_H },
    { 10, 128, CTRL_TILE_W, CTRL_TILE_H },
};

// Feste Palette fuer NEUE Geraete (Auftrag Schritt 2) - ohne die vier
// bereits vergebenen Kachelfarben (Gelb/Tuerkis/Gruen/Violett) und ohne
// Rot (Ausschalten-Rueckmeldefarbe) - alle gut mit schwarzer Schrift
// lesbar. Bei Erschoepfung ist laut Auftrag eine Wiederholung erlaubt.
constexpr uint16_t WOL_NEW_DEVICE_PALETTE[] = { TFT_ORANGE, TFT_PINK, TFT_SKYBLUE, TFT_MAGENTA };

uint16_t pickRandomDeviceColor() {
    constexpr uint8_t n = sizeof(WOL_NEW_DEVICE_PALETTE) / sizeof(WOL_NEW_DEVICE_PALETTE[0]);
    uint8_t startIdx = (uint8_t)random(n);
    for (uint8_t tries = 0; tries < n; tries++) {
        uint16_t candidate = WOL_NEW_DEVICE_PALETTE[(startIdx + tries) % n];
        bool used = false;
        for (uint8_t i = 0; i < deviceCfg.wolDeviceCount; i++) {
            if (deviceCfg.wolDevices[i].color == candidate) { used = true; break; }
        }
        if (!used) return candidate;
    }
    return WOL_NEW_DEVICE_PALETTE[random(n)]; // Palette erschoepft - Wiederholung erlaubt
}

// Zurueck-Button in der Controls-Kopfzeile (Auftrag "Controls-
// Bedienung", Schritt 1) - sichtbarer Button kleiner als der Tipp-
// Bereich (grosszuegige Trefferflaeche fuer Finger, ohne optisch zu
// gross zu wirken oder die "Controls"-Ueberschrift zu beruehren).
constexpr int16_t CTRL_BACK_VISIBLE_X = 8, CTRL_BACK_VISIBLE_Y = 6;
constexpr int16_t CTRL_BACK_VISIBLE_W = 44, CTRL_BACK_VISIBLE_H = 28;
Rect ctrlBackTapRect = { 0, 0, 80, 38 };

// BUGFIX (Auftrag "Controls-Optik", Teil A1/A2): TFT_eSPI::drawArc()
// zaehlt den Winkel NICHT wie ein Zifferblatt von oben (12 Uhr), sondern
// beginnt bei 0 Grad unten (6 Uhr) - direkt im Quellcode nachgeprueft
// (Kommentar "Arc sweeps through 6 o'clock" bei der Sonderbehandlung
// fuer endAngle<startAngle, sowie die Quadrant-Zuordnung der ersten
// 0-90 Grad zum unteren linken Quadranten). 180 Grad liegt damit (egal
// in welche Drehrichtung gezaehlt wird, da genau gegenueberliegend) bei
// 12 Uhr oben - die Luecke muss also um 180 Grad zentriert sein, nicht
// um 0. Vorher stand sie faelschlich unten (Auftrag-Beobachtung "Power-
// Symbol steht auf dem Kopf").
void drawPowerIcon(int16_t cx, int16_t cy, uint16_t color, uint16_t bg) {
    const int16_t r = 15, ir = 10;
    // Luecke ca. 70 Grad, mittig oben (145..215) - gezeichneter Bogen ist
    // der Rest (215..360 + 0..145), drawArc() uebernimmt das Aufteilen
    // selbst (endAngle < startAngle -> zwei Teile, siehe Kommentar oben).
    tft.drawArc(cx, cy, r, ir, 215, 145, color, bg, true);
    // Strich ragt von oben (12 Uhr) durch die Luecke in den Ring.
    tft.drawWideLine(cx, cy - r - 1, cx, cy - ir + 1, 5.0f, color, bg);
}

// Zeichnet eine geglaettete Graustufen-Alpha-Maske (0=unsichtbar,
// 255=volle Vordergrundfarbe, dazwischen weich ueberblendet) an eine
// Bildschirmposition - fuer das Luefterrad (siehe icon_fan.h), das mit
// TFT_eSPI-Primitiven allein nicht sauber darstellbar waere (echte
// Fluegel-Rotation, siehe Kommentar bei drawFanIcon()). Blendet direkt
// in den RGB565-Farbkanaelen (5/6/5 Bit), ausreichend genau fuer ein so
// kleines Symbol, ohne jeden Pixel erst in 8-Bit-RGB umrechnen zu
// muessen.
void drawAlphaIcon(const uint8_t* mask, int16_t size, int16_t cx, int16_t cy, uint16_t fg, uint16_t bg) {
    int16_t x0 = cx - size / 2;
    int16_t y0 = cy - size / 2;
    int16_t fgR = (fg >> 11) & 0x1F, fgG = (fg >> 5) & 0x3F, fgB = fg & 0x1F;
    int16_t bgR = (bg >> 11) & 0x1F, bgG = (bg >> 5) & 0x3F, bgB = bg & 0x1F;
    for (int16_t yy = 0; yy < size; yy++) {
        for (int16_t xx = 0; xx < size; xx++) {
            uint8_t a = pgm_read_byte(&mask[yy * size + xx]);
            if (a == 0) continue;
            uint16_t color;
            if (a >= 250) {
                color = fg;
            } else {
                int16_t r = bgR + ((fgR - bgR) * a) / 255;
                int16_t g = bgG + ((fgG - bgG) * a) / 255;
                int16_t b = bgB + ((fgB - bgB) * a) / 255;
                color = (uint16_t)((r << 11) | (g << 5) | b);
            }
            tft.drawPixel(x0 + xx, y0 + yy, color);
        }
    }
}

// BUGFIX (Auftrag "Controls-Optik", Teil A1/A2): vorher mit
// fillEllipse() drei IMMER gleich ausgerichtete, nur um die Nabe
// VERSCHOBENE Ellipsen - sah wie ein Klecks aus, weil TFT_eSPIs
// fillEllipse() nicht rotieren kann, obwohl der Auftrag echte, um
// 0/120/240 Grad GEDREHTE Fluegel verlangt. Jetzt eine vorab am
// Rechner mit voller Anti-Aliasing-Qualitaet erzeugte Bitmaske (siehe
// tools/generate_controls_icons.py, Ergebnis in icon_fan.h) - die Nabe
// selbst bleibt ein einfacher, zur Laufzeit in der Hintergrundfarbe
// gezeichneter Kreis (stanzt wie zuvor ein Loch in die Fluegelmitte).
void drawFanIcon(int16_t cx, int16_t cy, uint16_t bladeColor, uint16_t hubBg) {
    drawAlphaIcon(iconFanMask, ICONFAN_SIZE, cx, cy, bladeColor, hubBg);
    tft.fillCircle(cx, cy, 4, hubBg);
}

void drawMenuIcon(int16_t cx, int16_t cy, uint16_t color) {
    const int16_t barW = 22, barH = 4, gap = 7;
    tft.fillRect(cx - barW / 2, cy - gap - barH / 2, barW, barH, color);
    tft.fillRect(cx - barW / 2, cy - barH / 2, barW, barH, color);
    tft.fillRect(cx - barW / 2, cy + gap - barH / 2, barW, barH, color);
}

// Zeichnet das zur Kachel gehoerende Symbol an der festen Position
// (ca. 54px unter der Kachel-Oberkante, Auftrag), unabhaengig davon,
// ob gerade der normale Koerper oder ein Feedback-Flash dahinter steht.
void drawCtrlIcon(int idx, uint16_t color, uint16_t bg = CTRL_BODY_BG) {
    const Rect& r = ctrlTileRects[idx];
    int16_t cx = r.x + r.w / 2;
    int16_t cy = r.y + 54;
    switch (idx) {
        case CTRL_ACER:
        case CTRL_MAC:
            drawPowerIcon(cx, cy, color, bg);
            break;
        case CTRL_FAN:
            drawFanIcon(cx, cy, color, bg);
            break;
        case CTRL_MENU:
            drawMenuIcon(cx, cy, color);
            break;
    }
}

// Rundet NUR die oberen bzw. NUR die unteren beiden Ecken eines
// Rechtecks (Auftrag "Controls-Optik", Teil A3: Streifen oben rund/
// unten eckig, Koerper darunter umgekehrt - zusammen ein rund
// umrissenes Kachel-Rechteck). TFT_eSPI::fillRoundRect() selbst rundet
// IMMER alle vier Ecken; diese zwei Helfer bauen dieselbe Technik
// (fillRect fuer die gerade Haelfte + fillCircleHelper() fuer genau
// zwei Ecken) von Hand nach - fillCircleHelper() ist public, siehe
// TFT_eSPI::fillRoundRect()-Quelltext (cornername 2 = oben, 1 = unten).
void fillRoundRectTop(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    tft.fillRect(x, y + r, w, h - r, color);
    tft.fillCircleHelper(x + r, y + r, r, 2, w - r - r - 1, color);
}
void fillRoundRectBottom(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    tft.fillRect(x, y, w, h - r, color);
    tft.fillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
}

// Komplette Kachel-Silhouette (Streifen oben rund + Koerper unten rund,
// gemeinsam ein rund umrissenes 146x80-Rechteck) - gemeinsame Basis fuer
// Ruhezustand UND Feedback-Flash (dort stripeColor==bodyColor).
void drawCtrlTileShape(int idx, uint16_t stripeColor, uint16_t bodyColor) {
    const Rect& r = ctrlTileRects[idx];
    fillRoundRectTop(r.x, r.y, r.w, CTRL_STRIPE_H, CTRL_RADIUS, stripeColor);
    fillRoundRectBottom(r.x, (int16_t)(r.y + CTRL_STRIPE_H), r.w, (int16_t)(r.h - CTRL_STRIPE_H), CTRL_RADIUS, bodyColor);
}

// Feine Umrandung (Auftrag "Umlaute und Umrandung", Schritt A0) - wird
// IMMER als allerletztes gezeichnet (nach Streifen+Koerper+Symbol/Text),
// damit sie nie ueberdeckt wird und bei jedem Zustand (Ruhe/Feedback/
// Boost) luecken-frei bleibt. Staerke/Farbe aus Config, mehrere Ringe
// nach INNEN gestapelt falls CTRL_OUTLINE_PX > 1 (beruehrt dadurch nie
// die Nachbarkachel).
void drawCtrlOutline(int idx) {
    const Rect& r = ctrlTileRects[idx];
    for (int16_t i = 0; i < Config::CTRL_OUTLINE_PX; i++) {
        tft.drawRoundRect((int16_t)(r.x + i), (int16_t)(r.y + i), (int16_t)(r.w - 2 * i), (int16_t)(r.h - 2 * i),
                           CTRL_RADIUS, Config::CTRL_OUTLINE_COLOR);
    }
}

// Streifen+Koerper (siehe drawCtrlTileShape()) plus Label im Streifen -
// gemeinsame Basis fuer alle vier Kacheln im Ruhezustand.
void drawCtrlTileBase(int idx, uint16_t stripeColor, const String& label) {
    const Rect& r = ctrlTileRects[idx];
    drawCtrlTileShape(idx, stripeColor, CTRL_BODY_BG);
    tft.setTextColor(TFT_BLACK, stripeColor);
    tft.setTextDatum(MC_DATUM);
    pickClearFont(label, (int16_t)(r.w - 6), (int16_t)(CTRL_STRIPE_H - 4));
    tft.drawString(label, r.x + r.w / 2, r.y + CTRL_STRIPE_H / 2);
    tft.setTextDatum(TL_DATUM);
}

// Faerbt eine ganze Kachel (Streifen+Koerper) in der Rueckmeldefarbe,
// Symbol schwarz (Auftrag) - kein Label-Text waehrend des Flashs.
void ctrlFlashTile(int idx, uint16_t color) {
    drawCtrlTileShape(idx, color, color);
    drawCtrlIcon(idx, TFT_BLACK, color);
    drawCtrlOutline(idx);
}

void startCtrlFeedback(CtrlFeedback& fb, int idx, uint16_t color, uint32_t durationMs) {
    fb.color = color;
    fb.untilMs = millis() + durationMs;
    ctrlFlashTile(idx, color);
}

String formatFanLeftShort(int leftSec) {
    if (leftSec < 0) return I18n::t(StringId::FAN_BOOST_ON_INDEFINITE);
    int mm = leftSec / 60, ss = leftSec % 60;
    char buf[8];
    snprintf(buf, sizeof(buf), "%d:%02d", mm, ss);
    return String(buf);
}

// Geraete-Widget (Auftrag "WoL-Geraete") - EINE Implementierung fuer
// alle Geraete-Buttons auf allen drei Screens: "Tile" ist die 146x80-
// Kachel-Optik (Controls-Kachel 1 UND die drei Geraete-Seiten-Kacheln,
// identisch zur fruehren, jetzt entfernten drawCtrlAcerIdle()/
// ctrlFlashTile()-Kombination, nur ueber ein Rect statt einen Controls-
// Kachel-Index angesprochen - vermeidet den Icon-Mischmasch, der
// entstuende, wuerde man z.B. die CTRL_FAN-Position fuer ein Geraet
// wiederverwenden, siehe deviceTileRects[] oben), "Button" der
// schlichte Rahmen-Button auf "Keine Verbindung" (bisher
// drawNoConnAcerButton()/startNoConnAcerFeedback()). Das Enum selbst
// ist bereits weiter oben definiert (wird dort schon gebraucht).
void drawDeviceWidget(DeviceWidgetStyle style, const Rect& r, uint8_t deviceIdx) {
    const Storage::WolDevice& d = deviceCfg.wolDevices[deviceIdx];
    if (style == DeviceWidgetStyle::Tile) {
        fillRoundRectTop(r.x, r.y, r.w, CTRL_STRIPE_H, CTRL_RADIUS, d.color);
        fillRoundRectBottom(r.x, (int16_t)(r.y + CTRL_STRIPE_H), r.w, (int16_t)(r.h - CTRL_STRIPE_H), CTRL_RADIUS, CTRL_BODY_BG);
        tft.setTextColor(TFT_BLACK, d.color);
        tft.setTextDatum(MC_DATUM);
        pickClearFont(d.name, (int16_t)(r.w - 6), (int16_t)(CTRL_STRIPE_H - 4));
        tft.drawString(d.name, r.x + r.w / 2, r.y + CTRL_STRIPE_H / 2);
        tft.setTextDatum(TL_DATUM);
        drawPowerIcon(r.x + r.w / 2, (int16_t)(r.y + 54), TFT_WHITE, CTRL_BODY_BG);
        for (int16_t i = 0; i < Config::CTRL_OUTLINE_PX; i++) {
            tft.drawRoundRect((int16_t)(r.x + i), (int16_t)(r.y + i), (int16_t)(r.w - 2 * i),
                               (int16_t)(r.h - 2 * i), CTRL_RADIUS, Config::CTRL_OUTLINE_COLOR);
        }
    } else {
        drawButton(r, d.name.c_str(), TFT_WHITE);
    }
}

void flashDeviceWidget(DeviceWidgetStyle style, const Rect& r, uint8_t deviceIdx, uint16_t color) {
    if (style == DeviceWidgetStyle::Tile) {
        fillRoundRectTop(r.x, r.y, r.w, CTRL_STRIPE_H, CTRL_RADIUS, color);
        fillRoundRectBottom(r.x, (int16_t)(r.y + CTRL_STRIPE_H), r.w, (int16_t)(r.h - CTRL_STRIPE_H), CTRL_RADIUS, color);
        drawPowerIcon(r.x + r.w / 2, (int16_t)(r.y + 54), TFT_BLACK, color);
        for (int16_t i = 0; i < Config::CTRL_OUTLINE_PX; i++) {
            tft.drawRoundRect((int16_t)(r.x + i), (int16_t)(r.y + i), (int16_t)(r.w - 2 * i),
                               (int16_t)(r.h - 2 * i), CTRL_RADIUS, Config::CTRL_OUTLINE_COLOR);
        }
    } else {
        const Storage::WolDevice& d = deviceCfg.wolDevices[deviceIdx];
        drawButton(r, d.name.c_str(), color, true);
    }
}

void startDeviceFeedback(DeviceWidgetStyle style, const Rect& r, uint8_t deviceIdx, uint16_t color) {
    deviceFbColor[deviceIdx] = color;
    deviceFbUntilMs[deviceIdx] = millis() + WOL_FEEDBACK_MS;
    flashDeviceWidget(style, r, deviceIdx, color);
}

// Halten ohne eingerichtetes Ausschalten (Auftrag Schritt 6) - derselbe
// kurze Grau-Flash wie die "n/v"-Anzeige bei Luefter/Mac, mit eigenem
// Text statt Namen/Symbol darueber.
void showDeviceNotSetUp(DeviceWidgetStyle style, const Rect& r, uint8_t deviceIdx) {
    startDeviceFeedback(style, r, deviceIdx, TFT_DARKGREY);
    // UiText::draw() fuer BEIDE Stile (nicht pickClearFont()+drawString()
    // fuer die Kachel) - der Text "Ausschalten nicht eingerichtet" ist zu
    // lang, um bei pickClearFont() in jedem Fall wirklich innerhalb der
    // 146px-Kachelbreite zu bleiben (pickClearFont() hat keine echte
    // Kuerz-/Breitenpruefung wie UiText::draw() - lief dadurch sichtbar
    // links/rechts ueber die Kachel hinaus, von Alex gemeldet, 10.11.).
    UiText::Box box = { (int16_t)(r.x + 2), (int16_t)(r.y + 2), (int16_t)(r.w - 4), (int16_t)(r.h - 4) };
    UiText::draw(tft, box, I18n::t(StringId::WOL_SHUTDOWN_NOT_SET_UP), TFT_WHITE, TFT_DARKGREY, UiText::Align::Center, 2);
}

// Ausschalten fuer EIN Geraet (Auftrag Schritt 6) - legacyBroadcastShutdown
// (nur beim migrierten Acer-Eintrag gesetzt) sendet wie bisher per
// Broadcast (sendShutdownCommand(), unveraendert). Neue Geraete brauchen
// eine eingetragene IP (gezielter Unicast an Config::WIN_SHUTDOWN_UDP_PORT,
// GENAU dasselbe Protokoll/CmdAuth::signCommand() - der Windows-Listener
// unterscheidet beim Empfang nicht zwischen Broadcast/Unicast, siehe
// CLAUDE.md). Gibt false zurueck, wenn nichts gesendet wurde (Aufrufer
// zeigt dann showDeviceNotSetUp()).
bool sendDeviceShutdown(uint8_t deviceIdx) {
    const Storage::WolDevice& d = deviceCfg.wolDevices[deviceIdx];
    // IP hat IMMER Vorrang, sobald eingetragen (Auftrag "Broadcast-
    // Regression pruefen", 10.11.) - auch bei einem migrierten
    // Eintrag, der "legacyBroadcastShutdown" gesetzt hat, und auch
    // OHNE dass der Bearbeiten-Dialog vorher einmal bestaetigt wurde.
    // Gezielte Pakete sind zuverlässiger als Broadcast (siehe CLAUDE.md,
    // Alex' eigener Router liess Broadcast einmal nicht zuverlaessig
    // durch). Nur wenn KEINE IP eingetragen ist, bleibt der alte
    // Broadcast-Weg als Ersatz (fuer Geraete, die nie eine IP bekommen
    // haben).
    if (d.shutdownIp.length() > 0) {
        if (WiFi.status() != WL_CONNECTED) return false;
        IPAddress ip;
        if (!ip.fromString(d.shutdownIp)) return false;
        String line = CmdAuth::signCommand("SHUTDOWN", deviceCfg);
        udp.beginPacket(ip, Config::WIN_SHUTDOWN_UDP_PORT);
        udp.print(line);
        udp.endPacket();
        return true;
    }
    // Broadcast-Ersatzweg: unabhaengig vom (persistenten, aber durch
    // einen frueheren Bug einmal faelschlich dauerhaft geloeschten)
    // "legacyBroadcastShutdown"-Flag an den festen Platz 0 gebunden
    // (dort liegt immer der urspruenglich migrierte Eintrag) statt an
    // das Flag selbst - so bleibt der Ersatzweg auch dann erhalten,
    // wenn eine IP zwischenzeitlich eingetragen und wieder entfernt
    // wurde.
    if (d.legacyBroadcastShutdown || deviceIdx == 0) {
        sendShutdownCommand();
        return true;
    }
    return false;
}

// Gemeinsame Tap/Halten-Logik fuer Geraete-Widgets (Auftrag "WoL-
// Geraete") - "rects"/"deviceIdx" sind parallele Arrays der Laenge
// "count" (welches Rect gehoert zu welchem deviceCfg.wolDevices[]-
// Index), "style" waehlt Kachel- oder Button-Optik. g/tap kommen aus
// EINEM TouchCalib::pollGesture()-Aufruf des Aufrufers (fuer Tippen =
// WoL); Halten = Ausschalten laeuft zusaetzlich ueber
// TouchCalib::rawPoint(), exakt wie beim fruehren Acer-Halten-Block,
// nur jetzt fuer ein beliebiges Rect aus "rects"/"deviceIdx".
void handleDeviceWidgets(DeviceWidgetStyle style, const Rect* rects, const uint8_t* deviceIdx,
                          uint8_t count, TouchCalib::Gesture g, const TouchCalib::Point& tap) {
    if (g == TouchCalib::Gesture::Tap) {
        for (uint8_t i = 0; i < count; i++) {
            uint8_t di = deviceIdx[i];
            if (rects[i].contains(tap.x, tap.y) && millis() >= deviceTapLockedUntilMs[di] &&
                !(deviceTouch.longFired && deviceTouch.pressedDeviceIdx == (int8_t)di)) {
                triggerWolForDevice(di);
                deviceTapLockedUntilMs[di] = millis() + CTRL_TAP_LOCKOUT_MS;
                startDeviceFeedback(style, rects[i], di, TFT_GREEN);
                break;
            }
        }
    }

    TouchCalib::Point raw = TouchCalib::rawPoint();
    if (raw.touched) {
        deviceTouch.releasePending = false;
        TouchCalib::Point mp = TouchCalib::mappedPoint();
        if (!deviceTouch.pressActive) {
            for (uint8_t i = 0; i < count; i++) {
                if (rects[i].contains(mp.x, mp.y)) {
                    deviceTouch.pressActive = true;
                    deviceTouch.longFired = false;
                    deviceTouch.pressStartMs = millis();
                    deviceTouch.pressStartPoint = mp;
                    deviceTouch.maxMove = 0;
                    deviceTouch.pressedDeviceIdx = (int8_t)deviceIdx[i];
                    break;
                }
            }
        } else {
            int16_t dx = (int16_t)abs((int)mp.x - (int)deviceTouch.pressStartPoint.x);
            int16_t dy = (int16_t)abs((int)mp.y - (int)deviceTouch.pressStartPoint.y);
            int16_t moveNow = (dx > dy) ? dx : dy;
            if (moveNow > deviceTouch.maxMove) deviceTouch.maxMove = moveNow;
            if (!deviceTouch.longFired && deviceTouch.maxMove <= Config::TAP_MAX_MOVE_PX &&
                millis() - deviceTouch.pressStartMs >= Config::FAN_LONGPRESS_MS) {
                for (uint8_t i = 0; i < count; i++) {
                    if ((int8_t)deviceIdx[i] == deviceTouch.pressedDeviceIdx && rects[i].contains(mp.x, mp.y) &&
                        millis() >= deviceTapLockedUntilMs[deviceIdx[i]]) {
                        deviceTouch.longFired = true;
                        uint8_t di = deviceIdx[i];
                        bool sent = sendDeviceShutdown(di);
                        deviceTapLockedUntilMs[di] = millis() + CTRL_TAP_LOCKOUT_MS;
                        if (sent) {
                            startDeviceFeedback(style, rects[i], di, TFT_RED);
                        } else {
                            showDeviceNotSetUp(style, rects[i], di);
                        }
                        break;
                    }
                }
            }
        }
    } else if (deviceTouch.pressActive) {
        if (!deviceTouch.releasePending) {
            deviceTouch.releasePending = true;
            deviceTouch.releaseStartMs = millis();
        } else if (millis() - deviceTouch.releaseStartMs >= Config::TOUCH_RELEASE_DEBOUNCE_MS) {
            deviceTouch.pressActive = false;
            deviceTouch.releasePending = false;
            deviceTouch.longFired = false;
            deviceTouch.pressedDeviceIdx = -1;
        }
    }

    for (uint8_t i = 0; i < count; i++) {
        uint8_t di = deviceIdx[i];
        if (deviceFbColor[di] != 0 && millis() >= deviceFbUntilMs[di]) {
            deviceFbColor[di] = 0;
            drawDeviceWidget(style, rects[i], di);
        }
    }
}

void drawCtrlMacIdle() {
    drawCtrlTileBase(CTRL_MAC, TFT_CYAN, deviceCfg.labelMac);
    drawCtrlIcon(CTRL_MAC, TFT_WHITE);
    drawCtrlOutline(CTRL_MAC);
}

void drawCtrlMenuIdle() {
    drawCtrlTileBase(CTRL_MENU, TFT_PURPLE, I18n::t(StringId::MENU_BUTTON));
    drawCtrlIcon(CTRL_MENU, TFT_WHITE);
    drawCtrlOutline(CTRL_MENU);
}

bool fanAvailableNow() {
    return haveMacIp && current.valid && current.fanOk;
}

// Label der Luefter-Kachel (Auftrag "Umlaute und Umrandung", Schritt A0/
// A3-Test h: "Sprache auf Englisch -> Label 'Mac Fan'"). Anders als
// labelLaptop/labelMac ist dieses eine Label sprachabhaengig, ABER nur
// solange Alex es nicht selbst geaendert hat - ein eigener Name bleibt
// unabhaengig von der Sprache so stehen (gleiches Prinzip wie bei den
// beiden anderen Labels: einmal angepasst, nie automatisch ueberschrieben).
// DE_DEFAULT muss nach Schritt A3 (echter Umlaut) hier mit angepasst
// werden, siehe CLAUDE.md.
String fanTileLabel() {
    static const char* DE_DEFAULT = "Lüfter Mac";
    // ASCII-Platzhalter aus Schritt A0 (vor der neuen Schrift) - zaehlt
    // ebenfalls als "noch nicht von Alex angepasst", damit ein Geraet,
    // das diesen Uebergang mitgemacht hat, nicht faelschlich als
    // "eigener Name" behandelt wird.
    static const char* DE_DEFAULT_OLD_ASCII = "Luefter Mac";
    static const char* EN_DEFAULT = "Mac Fan";
    bool isStillDefault = (deviceCfg.labelFan == DE_DEFAULT) || (deviceCfg.labelFan == DE_DEFAULT_OLD_ASCII);
    if (I18n::currentLanguage() == "en" && isStillDefault) {
        return EN_DEFAULT;
    }
    return deviceCfg.labelFan;
}

void drawCtrlFanIdle() {
    const Rect& r = ctrlTileRects[CTRL_FAN];
    if (current.fanBoost) {
        // Aktiver Boost: ganze Kachel orange, Streifen "BOOST", Koerper
        // links das schwarze Luefterrad, rechts die Restzeit gross+fett
        // (Auftrag) - "BOOST" bleibt bewusst unuebersetzt wie bisher
        // auf dem Hauptscreen (siehe formatFanBoostLeft()).
        drawCtrlTileShape(CTRL_FAN, TFT_ORANGE, TFT_ORANGE);
        tft.setTextColor(TFT_BLACK, TFT_ORANGE);
        tft.setTextDatum(MC_DATUM);
        pickClearFont("BOOST", (int16_t)(r.w - 6), (int16_t)(CTRL_STRIPE_H - 4));
        tft.drawString("BOOST", r.x + r.w / 2, r.y + CTRL_STRIPE_H / 2);

        int16_t iconCx = r.x + 36, iconCy = r.y + 54;
        drawFanIcon(iconCx, iconCy, TFT_BLACK, TFT_ORANGE);

        String timeText = formatFanLeftShort(current.fanLeft);
        tft.setTextColor(TFT_WHITE, TFT_ORANGE);
        pickClearFont(timeText, (int16_t)(r.w - 78), 30);
        tft.drawString(timeText, (int16_t)(r.x + r.w - 40), iconCy);
        tft.setTextDatum(TL_DATUM);
        drawCtrlOutline(CTRL_FAN);
    } else {
        bool avail = fanAvailableNow();
        uint16_t stripeColor = avail ? TFT_GREEN : TFT_DARKGREY;
        String label = avail ? fanTileLabel() : String(I18n::t(StringId::PWR_NOT_AVAILABLE));
        drawCtrlTileBase(CTRL_FAN, stripeColor, label);
        drawCtrlIcon(CTRL_FAN, avail ? TFT_WHITE : TFT_DARKGREY);
        drawCtrlOutline(CTRL_FAN);
    }
}

// Zeichnet die Luefter-Kachel nur neu, wenn sich etwas am (echten,
// vom Mac gemeldeten) Zustand geaendert hat - verhindert Aufblitzen,
// gleiches Prinzip wie frueher updateFanStripe() auf dem Hauptscreen.
void updateCtrlFanTile(bool force) {
    static bool lastBoost = false;
    static int lastLeft = -999;
    static bool lastAvail = true;
    static bool initialized = false;
    bool avail = fanAvailableNow();
    if (force || !initialized || current.fanBoost != lastBoost || current.fanLeft != lastLeft || avail != lastAvail) {
        drawCtrlFanIdle();
        lastBoost = current.fanBoost;
        lastLeft = current.fanLeft;
        lastAvail = avail;
        initialized = true;
    }
}

// Seitenpunkte unten (Auftrag) - NUR auf Controls: auf dem Hauptscreen
// liesse die zweite Kachelreihe dafuer nur ca. 3px Platz (nicht die
// geforderten mind. 8px), siehe CLAUDE.md.
// Verallgemeinert fuer 2 ODER 3 Seiten (Auftrag "WoL-Geraete") - die
// Seite "Geraete" existiert nur bei wolDeviceCount>=2 (Auftrag Schritt
// 4). Bei 2 Seiten bleiben die Positionen (150,170) exakt wie vorher
// (20px Abstand, zentriert um die Bildschirmmitte 160) - bei 3 Seiten
// symmetrisch auf (140,160,180) erweitert. Aktiver Punkt = currentPage,
// aufgerufen sowohl von drawControlsStaticLayout() als auch von der
// neuen Geraete-Seite.
void drawPageDots() {
    bool hasDevicesPage = deviceCfg.wolDeviceCount >= 2;
    uint8_t pageCount = hasDevicesPage ? 3 : 2;
    int16_t spacing = 20;
    int16_t startX = (int16_t)(160 - (pageCount - 1) * spacing / 2);
    for (uint8_t i = 0; i < pageCount; i++) {
        int16_t x = (int16_t)(startX + i * spacing);
        bool active = (i == 0 && currentPage == Page::Main) ||
                      (i == 1 && currentPage == Page::Controls) ||
                      (i == 2 && currentPage == Page::Devices);
        tft.fillCircle(x, 228, 3, active ? TFT_WHITE : CTRL_DOT_INACTIVE);
    }
}

// Zurueck-Button (Auftrag "Controls-Bedienung", Schritt 1) - Pfeil aus
// zwei Linien gezeichnet (nicht als Schriftzeichen, damit er in jeder
// Sprache/Schrift gleich aussieht), dieselbe feine Umrandung wie die
// Kacheln. "flashed" = kurze weisse Rueckmeldung nach dem Antippen,
// Pfeil dann schwarz.
void drawCtrlBackButton(bool flashed) {
    const Rect r = { CTRL_BACK_VISIBLE_X, CTRL_BACK_VISIBLE_Y, CTRL_BACK_VISIBLE_W, CTRL_BACK_VISIBLE_H };
    uint16_t bg = flashed ? TFT_WHITE : TFT_BLACK;
    uint16_t arrowColor = flashed ? TFT_BLACK : TFT_WHITE;
    tft.fillRoundRect(r.x, r.y, r.w, r.h, CTRL_RADIUS, bg);
    for (int16_t i = 0; i < Config::CTRL_OUTLINE_PX; i++) {
        tft.drawRoundRect((int16_t)(r.x + i), (int16_t)(r.y + i), (int16_t)(r.w - 2 * i), (int16_t)(r.h - 2 * i),
                           CTRL_RADIUS, Config::CTRL_OUTLINE_COLOR);
    }
    // Chevron "<": Spitze links, zwei Linien nach oben/unten rechts.
    int16_t tipX = r.x + 15, tipY = r.y + r.h / 2;
    int16_t armX = r.x + 29;
    tft.drawWideLine(tipX, tipY, armX, (int16_t)(tipY - 9), 3.0f, arrowColor, bg);
    tft.drawWideLine(tipX, tipY, armX, (int16_t)(tipY + 9), 3.0f, arrowColor, bg);
}

// Einfacher Ja/Nein-Bestaetigungsscreen (Auftrag "WoL-Geraete", fuer
// die Loeschen-Rueckfrage) - blockierend, gibt true (Ja) oder false
// (Nein) zurueck. Eigene, schlichte Umsetzung statt einer Kopie der
// Flightradar-WLAN-Liste, die dort bewusst OHNE Rueckfrage sofort
// loescht (siehe Fortschritts-Abschnitt Schritt 1).
bool confirmYesNo(const char* title, const String& body) {
    const int16_t W = Config::SCREEN_WIDTH;
    tft.fillScreen(TFT_BLACK);
#ifdef LAYOUT_DEBUG
    UiText::beginScreen("Bestaetigen");
#endif
    UiText::drawHeader(tft, title, false);
    UiText::draw(tft, { 8, (int16_t)UiText::CONTENT_Y, (int16_t)(W - 16), 40 }, body, TFT_WHITE, TFT_BLACK, UiText::Align::Left);

    Rect yesBtn = { 20, 150, (int16_t)(W / 2 - 30), 36 };
    Rect noBtn = { (int16_t)(W / 2 + 10), 150, (int16_t)(W / 2 - 30), 36 };
    drawButton(yesBtn, I18n::t(StringId::CONFIRM_YES), TFT_RED);
    drawButton(noBtn, I18n::t(StringId::CONFIRM_NO), TFT_GREEN);

    while (true) {
        TouchCalib::Point tap;
        if (TouchCalib::wasTapped(tap)) {
            if (yesBtn.contains(tap.x, tap.y)) return true;
            if (noBtn.contains(tap.x, tap.y)) return false;
        }
        pollUsbSerial();
        delay(10);
    }
}

// Name+MAC+optionale Shutdown-IP abfragen (Auftrag "WoL-Geraete",
// Schritt 3) - fuer "Geraet hinzufuegen" UND "Eintrag bearbeiten"
// (gleiche Funktion, kein Duplikat). "shutdownIp" dient als IN/OUT:
// beim Hinzufuegen leer hineingegeben, beim Bearbeiten der bisherige
// Wert - ueberspringt man den optionalen IP-Schritt (Zurueck), bleibt
// er UNVERAENDERT (beim Hinzufuegen also leer, beim Bearbeiten wie
// vorher - loescht eine schon eingetragene IP nicht versehentlich).
// Gibt false zurueck, wenn Name- oder MAC-Schritt per Zurueck komplett
// abgebrochen wurde (dann bleiben alle Parameter unveraendert).
//
// "name"/"mac" dienen SEIT Auftrag "Modularer Listener" ebenfalls als
// Vorbelegung (von Alex live gewuenscht, 10.11.: "wenn ich auf einen
// existierenden Namen tippe, moechte ich nur etwas aendern... nicht
// jedesmal alles neu eingeben") - beim Hinzufuegen kommen sie leer
// herein (also wie bisher leere Tastatur), beim Bearbeiten mit dem
// bisherigen Wert. Bei einem Retry (z.B. ungueltige MAC) bleibt
// zusaetzlich das zuletzt EINGETIPPTE erhalten, nicht der urspruengliche
// Wert - ein Tippfehler laesst sich so per Ruecktaste korrigieren statt
// alles neu eintippen zu muessen.
// Auftrag "Monitor-Umschaltung" - neuer optionaler Schritt im WoL-
// Geraet-Dialog: 5 grosse Tasten (Aus/HDMI1/HDMI2/DP1/DP2), aktuelle
// Auswahl invertiert dargestellt. "Zurueck" (Kopfzeilen-Pfeil, siehe
// UiText::BACK_TAP_RECT, gleiches Muster wie runWolDevicesScreen())
// behaelt den bisherigen Wert bei - dieser Schritt ist optional, kein
// Abbruch des gesamten Dialogs. "?" oeffnet eine kurze Erklaerung.
uint8_t runMonitorInputPicker(uint8_t current) {
    const int16_t W = Config::SCREEN_WIDTH;
    constexpr int16_t ROW_H = 26, ROW_GAP = 4, TOP = 40;
    const char* labels[5] = {
        I18n::t(StringId::MONITOR_INPUT_OFF),
        I18n::t(StringId::MONITOR_INPUT_HDMI1),
        I18n::t(StringId::MONITOR_INPUT_HDMI2),
        I18n::t(StringId::MONITOR_INPUT_DP1),
        I18n::t(StringId::MONITOR_INPUT_DP2),
    };
    Rect optionRects[5];
    for (uint8_t i = 0; i < 5; i++) {
        optionRects[i] = { 10, (int16_t)(TOP + i * (ROW_H + ROW_GAP)), (int16_t)(W - 20), ROW_H };
    }
    Rect helpBtn = { (int16_t)(W - 32), 4, 28, 26 };

    tft.fillScreen(TFT_BLACK);
    UiText::drawHeader(tft, I18n::t(StringId::MONITOR_INPUT_TITLE), true);
    for (uint8_t i = 0; i < 5; i++) {
        drawButton(optionRects[i], labels[i], TFT_CYAN, i == current);
    }
    drawButton(helpBtn, "?", TFT_YELLOW);

    while (true) {
        TouchCalib::Point tap;
        if (!TouchCalib::wasTapped(tap)) {
            pollUsbSerial();
            delay(10);
            continue;
        }
        if (UiText::BACK_TAP_RECT.contains(tap.x, tap.y)) {
            UiText::drawHeader(tft, I18n::t(StringId::MONITOR_INPUT_TITLE), true, true);
            delay(150);
            return current;
        }
        if (helpBtn.contains(tap.x, tap.y)) {
            showHelpPopup(I18n::t(StringId::MONITOR_INPUT_TITLE), I18n::t(StringId::MONITOR_INPUT_HELP));
            tft.fillScreen(TFT_BLACK);
            UiText::drawHeader(tft, I18n::t(StringId::MONITOR_INPUT_TITLE), true);
            for (uint8_t i = 0; i < 5; i++) {
                drawButton(optionRects[i], labels[i], TFT_CYAN, i == current);
            }
            drawButton(helpBtn, "?", TFT_YELLOW);
            continue;
        }
        for (uint8_t i = 0; i < 5; i++) {
            if (optionRects[i].contains(tap.x, tap.y)) {
                return i;
            }
        }
    }
}

bool runWolDeviceEntryFlow(String& name, String& mac, String& shutdownIp, uint8_t& monitorInput) {
    String nameDraft = name;
    while (true) {
        bool cancelled = false;
        String n = SetupFlow::runTextEntry(tft, I18n::t(StringId::WOL_NAME_PROMPT), cancelled, nameDraft.c_str());
        if (cancelled) return false;
        n.trim();
        nameDraft = n;
        if (n.length() == 0) {
            showHelpPopup(I18n::t(StringId::SETUP_WOL_MAC), I18n::t(StringId::WOL_EMPTY_NAME));
            continue;
        }
        name = n;
        break;
    }
    String macDraft = mac;
    while (true) {
        bool cancelled = false;
        String m = SetupFlow::runTextEntry(tft, I18n::t(StringId::KB_WOL_MAC_PROMPT), cancelled, macDraft.c_str());
        if (cancelled) return false;
        m.trim();
        m.toUpperCase();
        macDraft = m;
        uint8_t probe[6];
        if (!parseMacAddress(m, probe)) {
            showHelpPopup(I18n::t(StringId::SETUP_WOL_MAC), I18n::t(StringId::WOL_INVALID_MAC));
            continue;
        }
        mac = m;
        break;
    }
    String keepIp = shutdownIp;
    bool ipCancelled = false;
    String ip = SetupFlow::runTextEntry(tft, I18n::t(StringId::WOL_SHUTDOWN_IP_PROMPT), ipCancelled, shutdownIp.c_str());
    if (ipCancelled) {
        shutdownIp = keepIp;
    } else {
        ip.trim();
        IPAddress probe;
        shutdownIp = probe.fromString(ip) ? ip : "";
    }
    // Auftrag "Monitor-Umschaltung" - letzter, ebenfalls optionaler
    // Schritt. "Zurueck" dort behaelt monitorInput unveraendert bei
    // (siehe runMonitorInputPicker()).
    monitorInput = runMonitorInputPicker(monitorInput);
    return true;
}

// Geraeteliste verwalten (Auftrag "WoL-Geraete", Schritt 3) - Liste mit
// Name+MAC, rotes "X" je Zeile (Rueckfrage vor dem Loeschen), "Geraet
// hinzufuegen" unten (verschwindet ab Config::MAX_WOL_DEVICES). Tippen
// auf eine Zeile oeffnet dieselbe Eingabe-Abfolge wie beim Hinzufuegen
// (runWolDeviceEntryFlow()), mit den bisherigen Werten vorbelegt.
void runWolDevicesScreen() {
    const int16_t W = Config::SCREEN_WIDTH;
    constexpr int16_t ROW_H = 40, ROW_GAP = 6, X_W = 36;

    bool done = false;
    while (!done) {
        tft.fillScreen(TFT_BLACK);
#ifdef LAYOUT_DEBUG
        UiText::beginScreen("WolGeraete");
#endif
        UiText::drawHeader(tft, I18n::t(StringId::SETUP_WOL_MAC), true);

        uint8_t count = deviceCfg.wolDeviceCount;
        Rect rowRects[Config::MAX_WOL_DEVICES];
        Rect xRects[Config::MAX_WOL_DEVICES];
        int16_t y = (int16_t)(UiText::CONTENT_Y + 2);
        for (uint8_t i = 0; i < count; i++) {
            rowRects[i] = { 8, y, (int16_t)(W - 16 - X_W - 6), ROW_H };
            xRects[i] = { (int16_t)(W - 8 - X_W), y, X_W, ROW_H };
            const Rect& rr = rowRects[i];
            // Name und MAC getrennt zeichnen (Auftrag: "Name und kleine
            // MAC") statt beides in EINER Zeile/EINEM UiText::draw()-
            // Aufruf zu verketten - sonst waehlt UiText::draw() wegen des
            // langen Gesamttexts automatisch die kleinste Schriftstufe,
            // auch fuer den Namen (von Alex als "andere/weniger schoene
            // Schrift" bemerkt - behoben, 10.11.). Name bekommt dadurch
            // dieselbe grosse Schrift wie ueberall sonst (Kachel/Button).
            tft.fillRect(rr.x, rr.y, rr.w, rr.h, TFT_BLACK);
            tft.drawRect(rr.x, rr.y, rr.w, rr.h, TFT_CYAN);
            UiText::Box nameBox = { (int16_t)(rr.x + 6), rr.y, (int16_t)(rr.w - 10), (int16_t)(rr.h * 2 / 3) };
            UiText::Box macBox = { (int16_t)(rr.x + 6), (int16_t)(rr.y + rr.h * 2 / 3), (int16_t)(rr.w - 10), (int16_t)(rr.h - rr.h * 2 / 3) };
            UiText::draw(tft, nameBox, deviceCfg.wolDevices[i].name, TFT_CYAN, TFT_BLACK, UiText::Align::Left, 0);
            UiText::draw(tft, macBox, deviceCfg.wolDevices[i].mac, TFT_DARKGREY, TFT_BLACK, UiText::Align::Left, 0);
            drawButton(xRects[i], "X", TFT_RED);
            y = (int16_t)(y + ROW_H + ROW_GAP);
        }

        bool canAdd = count < Config::MAX_WOL_DEVICES;
        Rect addBtn = { 8, y, (int16_t)(W - 16), ROW_H };
        if (canAdd) {
            drawButton(addBtn, I18n::t(StringId::WOL_ADD_DEVICE), TFT_GREEN);
        }

        TouchCalib::Point tap;
        while (true) {
            if (TouchCalib::wasTapped(tap)) break;
            pollUsbSerial();
            delay(10);
        }

        if (UiText::BACK_TAP_RECT.contains(tap.x, tap.y)) {
            UiText::drawHeader(tft, I18n::t(StringId::SETUP_WOL_MAC), true, true);
            delay(150);
            done = true;
            continue;
        }

        bool handled = false;
        for (uint8_t i = 0; i < count && !handled; i++) {
            if (xRects[i].contains(tap.x, tap.y)) {
                handled = true;
                String prefix = String(I18n::t(StringId::WOL_DELETE_CONFIRM_PREFIX)) + deviceCfg.wolDevices[i].name;
                if (confirmYesNo(I18n::t(StringId::SETUP_WOL_MAC), prefix)) {
                    for (uint8_t j = i; (uint8_t)(j + 1) < deviceCfg.wolDeviceCount; j++) {
                        deviceCfg.wolDevices[j] = deviceCfg.wolDevices[(uint8_t)(j + 1)];
                    }
                    deviceCfg.wolDeviceCount--;
                    Storage::save(deviceCfg);
                }
            }
        }
        for (uint8_t i = 0; i < count && !handled; i++) {
            if (rowRects[i].contains(tap.x, tap.y)) {
                handled = true;
                String name = deviceCfg.wolDevices[i].name;
                String mac = deviceCfg.wolDevices[i].mac;
                String shutdownIp = deviceCfg.wolDevices[i].shutdownIp;
                uint8_t monitorInput = deviceCfg.wolDevices[i].monitorInputOnPowerOn;
                if (runWolDeviceEntryFlow(name, mac, shutdownIp, monitorInput)) {
                    deviceCfg.wolDevices[i].name = name;
                    deviceCfg.wolDevices[i].mac = mac;
                    deviceCfg.wolDevices[i].shutdownIp = shutdownIp;
                    deviceCfg.wolDevices[i].monitorInputOnPowerOn = monitorInput;
                    Storage::save(deviceCfg);
                }
            }
        }
        if (!handled && canAdd && addBtn.contains(tap.x, tap.y)) {
            String name, mac, shutdownIp;
            uint8_t monitorInput = 0;
            if (runWolDeviceEntryFlow(name, mac, shutdownIp, monitorInput)) {
                Storage::WolDevice& d = deviceCfg.wolDevices[deviceCfg.wolDeviceCount];
                d.name = name;
                d.mac = mac;
                d.shutdownIp = shutdownIp;
                d.monitorInputOnPowerOn = monitorInput;
                d.color = pickRandomDeviceColor();
                d.legacyBroadcastShutdown = false;
                deviceCfg.wolDeviceCount++;
                Storage::save(deviceCfg);
            }
        }
    }
}

void drawControlsStaticLayout() {
    tft.fillScreen(TFT_BLACK);
    useBuiltinFont();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    pickClearFont(I18n::t(StringId::CONTROLS_TITLE), (int16_t)(Config::SCREEN_WIDTH - 20), 20);
    tft.setTextDatum(BC_DATUM);
    tft.drawString(I18n::t(StringId::CONTROLS_TITLE), Config::SCREEN_WIDTH / 2, 26);
    tft.setTextDatum(TL_DATUM);

    drawCtrlBackButton(false);
    // Geraet Nr. 1 (bisher fest "Acer") - Kachel bleibt leer, wenn die
    // Liste leer ist (Auftrag "WoL-Geraete", Schritt 4).
    if (deviceCfg.wolDeviceCount > 0) {
        drawDeviceWidget(DeviceWidgetStyle::Tile, ctrlTileRects[CTRL_ACER], 0);
    }
    drawCtrlMacIdle();
    drawCtrlFanIdle();
    drawCtrlMenuIdle();
    drawPageDots();
}

// Seite "Geraete" (Auftrag "WoL-Geraete") - nur erreichbar per Wischen
// von Controls (siehe loop()), zeigt die Geraete 2-4 als Kacheln an
// exakt denselben drei Positionen wie Controls' CTRL_ACER/CTRL_MAC/
// CTRL_FAN. Gleiche Kopfzeile/Zurueck-Pfeil-Optik wie Controls
// (drawCtrlBackButton()/ctrlBackTapRect wiederverwendet), fuehrt aber
// zurueck zu Controls statt zu Main.
void drawDevicesStaticLayout() {
    tft.fillScreen(TFT_BLACK);
    useBuiltinFont();
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    pickClearFont(I18n::t(StringId::DEVICES_PAGE_TITLE), (int16_t)(Config::SCREEN_WIDTH - 20), 20);
    tft.setTextDatum(BC_DATUM);
    tft.drawString(I18n::t(StringId::DEVICES_PAGE_TITLE), Config::SCREEN_WIDTH / 2, 26);
    tft.setTextDatum(TL_DATUM);

    drawCtrlBackButton(false);
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t deviceIdx = (uint8_t)(i + 1);
        if (deviceIdx < deviceCfg.wolDeviceCount) {
            drawDeviceWidget(DeviceWidgetStyle::Tile, deviceTileRects[i], deviceIdx);
        }
    }
    drawPageDots();
}

void handleDevicesPage() {
    TouchCalib::Point tap;
    TouchCalib::Gesture g = TouchCalib::pollGesture(tap);

    if (g == TouchCalib::Gesture::SwipeRight) {
        currentPage = Page::Controls;
        drawControlsStaticLayout();
        return;
    }

    if (g == TouchCalib::Gesture::Tap && ctrlBackTapRect.contains(tap.x, tap.y)) {
        drawCtrlBackButton(true);
        delay(150);
        currentPage = Page::Controls;
        drawControlsStaticLayout();
        return;
    }

    Rect rects[3];
    uint8_t idxs[3];
    uint8_t rectCount = 0;
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t deviceIdx = (uint8_t)(i + 1);
        if (deviceIdx < deviceCfg.wolDeviceCount) {
            rects[rectCount] = deviceTileRects[i];
            idxs[rectCount] = deviceIdx;
            rectCount++;
        }
    }
    handleDeviceWidgets(DeviceWidgetStyle::Tile, rects, idxs, rectCount, g, tap);
}

// Zustandsautomat "Mac schlaeft absichtlich" (Auftrag "Display aus im
// Ruhezustand") - wird aus loop() JEDEN Durchlauf aufgerufen, NACH der
// bestehenden "Keine Verbindung"-Umschaltung weiter oben (wichtig: falls
// hier echte Daten das Aufwachen ausloesen, hat die bestehende Logik zu
// diesem Zeitpunkt das Dashboard bereits fertig gezeichnet - die
// Beleuchtung geht deshalb genau hier, NACH diesem Zeichnen, wieder an,
// siehe Auftrag). Das Anfordern (Off -> Requested) passiert separat in
// handleControlsPage() beim Antippen der Mac-Kachel.
void updateMacSleepState() {
    if (macSleepState == MacSleepState::Off) return;

    if (macSleepState == MacSleepState::Requested) {
        // Bestaetigung = current.sleepAck hat sich seit dem Senden
        // geaendert (derselbe Zaehler/dieselbe Momentaufnahme wie das
        // kurze Controls-Feedback, siehe sleepCmdBaselineAck oben).
        if (current.sleepAck != sleepCmdBaselineAck) {
            macSleepState = MacSleepState::Confirmed;
            macSleepConfirmedAtMs = millis();
            Serial.println("[Eiswolfs iBoard] Mac-Schlaf: Bestaetigung da");
        } else if (millis() - macSleepRequestedAtMs >= SLEEP_ACK_TIMEOUT_MS) {
            macSleepState = MacSleepState::Off;
            Serial.println("[Eiswolfs iBoard] Mac-Schlaf: keine Bestaetigung, abgebrochen");
        }
        return;
    }

    if (macSleepState == MacSleepState::Confirmed) {
        if (millis() - macSleepConfirmedAtMs >= SLEEP_DISPLAY_OFF_DELAY_MS) {
            macSleepState = MacSleepState::Active;
            macSleepBaselineLastPacketMs = computeLastAnyPacketMs();
            macSleepWakeCandidateSinceMs = 0;
            macSleepWokenAtMs = 0;
            // "Keine Verbindung" ist hier sachlich korrekt (es kommen ja
            // keine Daten) - wird aber NICHT gezeichnet (Beleuchtung aus,
            // siehe Auftrag "keine Meldungen auf dem Bildschirm").
            currentPage = Page::Main;
            showingNoConnection = true;
            analogWrite(TFT_BL, 0);
            Serial.println("[Eiswolfs iBoard] Mac-Schlaf: bestaetigt, Display aus");
        }
        return;
    }

    // MacSleepState::Active - Beleuchtung geht erst wieder an, wenn Daten
    // ANHALTEND ankommen (siehe WAKE_SUSTAIN_MS/WAKE_GAP_RESET_MS oben),
    // nicht schon bei einem einzelnen, isolierten Paket (z.B. DarkWake).
    uint32_t lastAnyPacketMs = computeLastAnyPacketMs();
    if (lastAnyPacketMs != macSleepBaselineLastPacketMs) {
        if (macSleepWakeCandidateSinceMs == 0 ||
            millis() - macSleepLastNewPacketMs > WAKE_GAP_RESET_MS) {
            macSleepWakeCandidateSinceMs = millis();
            Serial.println("[Eiswolfs iBoard] Mac-Schlaf: Daten kurz, ignoriert");
        }
        macSleepLastNewPacketMs = millis();
        macSleepBaselineLastPacketMs = lastAnyPacketMs;

        if (millis() - macSleepWakeCandidateSinceMs >= WAKE_SUSTAIN_MS) {
            macSleepState = MacSleepState::Off;
            macSleepWokenAtMs = 0;
            // Dashboard zuerst fertig zeichnen, dann erst die Beleuchtung
            // an (Auftrag) - die generelle "Keine Verbindung"-Umschaltung
            // in loop() ist waehrend Active bewusst blockiert, deshalb
            // hier selbst zeichnen.
            showingNoConnection = false;
            drawStaticLayout();
            lastDrawnValid = false;
            applyBrightness();
            Serial.println("[Eiswolfs iBoard] Mac-Schlaf: Daten anhaltend, Display an");
        }
        return;
    }
    if (macSleepWakeCandidateSinceMs != 0 &&
        millis() - macSleepLastNewPacketMs > WAKE_GAP_RESET_MS) {
        macSleepWakeCandidateSinceMs = 0; // Streak abgebrochen, Mac schlaeft weiter
    }

    if (macSleepWokenAtMs != 0 && millis() - macSleepWokenAtMs >= TOUCH_WAKE_MS) {
        macSleepWokenAtMs = 0;
        analogWrite(TFT_BL, 0);
    }
}

// Verarbeitet Touch/Zustand auf Controls - wird aus loop() JEDEN
// Durchlauf aufgerufen, solange currentPage==Controls (Empfang/
// Timeouts laufen dabei normal weiter, siehe loop()).
void handleControlsPage() {
    TouchCalib::Point tap;
    TouchCalib::Gesture g = TouchCalib::pollGesture(tap);

    if (g == TouchCalib::Gesture::SwipeRight) {
        currentPage = Page::Main;
        if (showingNoConnection) {
            drawNoConnectionScreen();
        } else {
            drawStaticLayout();
            lastDrawnValid = false;
        }
        return;
    }

    // Seite "Geraete" (Auftrag "WoL-Geraete") - nur erreichbar, wenn es
    // ueberhaupt etwas dort zu zeigen gibt (mind. 2 Geraete insgesamt).
    if (g == TouchCalib::Gesture::SwipeLeft && deviceCfg.wolDeviceCount >= 2) {
        currentPage = Page::Devices;
        drawDevicesStaticLayout();
        return;
    }

    if (g == TouchCalib::Gesture::Tap) {
        if (ctrlBackTapRect.contains(tap.x, tap.y)) {
            drawCtrlBackButton(true);
            delay(150);
            currentPage = Page::Main;
            if (showingNoConnection) {
                drawNoConnectionScreen();
            } else {
                drawStaticLayout();
                lastDrawnValid = false;
            }
            return;
        } else if (ctrlTileRects[CTRL_MAC].contains(tap.x, tap.y) && millis() >= macTapLockedUntilMs) {
            sendSleepCommand();
            macTapLockedUntilMs = millis() + CTRL_TAP_LOCKOUT_MS;
            sleepCmdPending = true;
            sleepCmdSentMs = millis();
            sleepCmdBaselineAck = current.sleepAck;
            startCtrlFeedback(macFb, CTRL_MAC, TFT_GREEN, PWR_FEEDBACK_RUHE_MS);
            // Auftrag "Display aus im Ruhezustand"/"Ruhezustand-Fix" -
            // eigener Zustand, der (ueber denselben sleepCmdBaselineAck
            // oben) auf die sleep_ack-Bestaetigung der Mac-App wartet,
            // siehe updateMacSleepState().
            macSleepState = MacSleepState::Requested;
            macSleepRequestedAtMs = millis();
            Serial.println("[Eiswolfs iBoard] Mac-Schlaf: angefordert");
        } else if (ctrlTileRects[CTRL_FAN].contains(tap.x, tap.y) && millis() >= fanTapLockedUntilMs) {
            fanTapLockedUntilMs = millis() + CTRL_TAP_LOCKOUT_MS;
            bool wantBoost = !current.fanBoost;
            if (wantBoost && !fanAvailableNow()) {
                // Ohne Verbindung/Hilfsdienst sofort "n/v", kein Befehl (Auftrag).
                startCtrlFeedback(fanFb, CTRL_FAN, TFT_DARKGREY, 700);
                tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
                tft.setTextDatum(MC_DATUM);
                pickClearFont(I18n::t(StringId::PWR_NOT_AVAILABLE), (int16_t)(CTRL_TILE_W - 6), 30);
                tft.drawString(I18n::t(StringId::PWR_NOT_AVAILABLE),
                               ctrlTileRects[CTRL_FAN].x + CTRL_TILE_W / 2, ctrlTileRects[CTRL_FAN].y + CTRL_TILE_H / 2);
                tft.setTextDatum(TL_DATUM);
            } else {
                sendFanCommand(wantBoost);
                startCtrlFeedback(fanFb, CTRL_FAN, wantBoost ? TFT_GREEN : TFT_RED, wantBoost ? 900 : 700);
            }
        } else if (ctrlTileRects[CTRL_MENU].contains(tap.x, tap.y)) {
            runMainMenu();
            drawControlsStaticLayout();
        }
    }

    // Geraet Nr. 1 (bisher fest "Acer"): Tippen=WoL/Halten=Ausschalten
    // ueber die gemeinsame Logik (Auftrag "WoL-Geraete") - ersetzt den
    // fruehren, Acer-spezifischen Hold-Block UND den Tap-Zweig oben.
    // Keine Kachel/Logik, wenn die Liste leer ist.
    if (deviceCfg.wolDeviceCount > 0) {
        Rect r = ctrlTileRects[CTRL_ACER];
        uint8_t di = 0;
        handleDeviceWidgets(DeviceWidgetStyle::Tile, &r, &di, 1, g, tap);
    }

    if (macFb.color != 0 && millis() >= macFb.untilMs) {
        macFb.color = 0;
        drawCtrlMacIdle();
    }
    bool fanFlashJustEnded = false;
    if (fanFb.color != 0 && millis() >= fanFb.untilMs) {
        fanFb.color = 0;
        fanFlashJustEnded = true;
    }

    // Bestaetigung eines gesendeten Luefter-Befehls (siehe sendFanCommand()/
    // fanCmdPending oben) - unveraendertes Timeout-Verhalten, nur das Ziel
    // der "n/v"-Anzeige ist jetzt die Controls-Kachel statt der alte
    // Hauptscreen-Streifen.
    if (fanCmdPending) {
        if (current.fanBoost == fanCmdWantBoost) {
            fanCmdPending = false;
        } else if (millis() - fanCmdSentMs > Config::FAN_CMD_CONFIRM_TIMEOUT_MS) {
            fanCmdPending = false;
            startCtrlFeedback(fanFb, CTRL_FAN, TFT_DARKGREY, 700);
            tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
            tft.setTextDatum(MC_DATUM);
            pickClearFont(I18n::t(StringId::PWR_NOT_AVAILABLE), (int16_t)(CTRL_TILE_W - 6), 30);
            tft.drawString(I18n::t(StringId::PWR_NOT_AVAILABLE),
                           ctrlTileRects[CTRL_FAN].x + CTRL_TILE_W / 2, ctrlTileRects[CTRL_FAN].y + CTRL_TILE_H / 2);
            tft.setTextDatum(TL_DATUM);
            fanFlashJustEnded = false; // neues Feedback-Fenster laeuft gerade erst an
        }
    }
    if (!fanFb.color) {
        updateCtrlFanTile(fanFlashJustEnded);
    }

    // Bestaetigung/Timeout des gesendeten SLEEP-Befehls (siehe loop()-
    // Kommentar frueher) - gleiches Timeout-Verhalten wie bisher, Ziel
    // ist jetzt die Mac-Kachel statt der Kopfzeile.
    if (sleepCmdPending) {
        if (current.sleepAck != sleepCmdBaselineAck) {
            sleepCmdPending = false;
        } else if (millis() - sleepCmdSentMs > Config::FAN_CMD_CONFIRM_TIMEOUT_MS) {
            sleepCmdPending = false;
            startCtrlFeedback(macFb, CTRL_MAC, TFT_DARKGREY, PWR_FEEDBACK_NA_MS);
            tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
            tft.setTextDatum(MC_DATUM);
            pickClearFont(I18n::t(StringId::PWR_NOT_AVAILABLE), (int16_t)(CTRL_TILE_W - 6), 30);
            tft.drawString(I18n::t(StringId::PWR_NOT_AVAILABLE),
                           ctrlTileRects[CTRL_MAC].x + CTRL_TILE_W / 2, ctrlTileRects[CTRL_MAC].y + CTRL_TILE_H / 2);
            tft.setTextDatum(TL_DATUM);
        }
    }
}

#ifdef SCREEN_TOUR
// Pruefung ohne Alex, Schritt 6 - zeigt Bildschirme automatisch
// nacheinander mit Extremtexten (langer WLAN-Name mit Umlauten, lange
// IP), je 4s, Name im Serial-Log. EHRLICHE EINSCHRAENKUNG (nicht
// verschwiegen): nur die Bildschirme, die als eigenstaendige, NICHT
// blockierende Zeichenfunktion existieren, lassen sich automatisch
// durchschalten (hier: "Keine Verbindung"). Menue/WLAN-Liste/Tastatur
// sind als blockierende Eingabeschleifen gebaut (warten auf echten
// Touch) - ein automatischer Durchlauf dieser Seiten wuerde eine
// groessere Umstrukturierung (Render-Funktion von Eingabeschleife
// trennen) brauchen und ist hier NICHT umgesetzt. Diese Seiten werden
// stattdessen manuell mit Alex getestet (siehe Auftrag Schritt 7).
void runScreenTour() {
    String savedSsid = deviceCfg.wifiSsid;
    Storage::WolDevice savedDevices[Config::MAX_WOL_DEVICES];
    for (uint8_t i = 0; i < Config::MAX_WOL_DEVICES; i++) savedDevices[i] = deviceCfg.wolDevices[i];
    uint8_t savedCount = deviceCfg.wolDeviceCount;
    Page savedPage = currentPage;
    String savedLang = I18n::currentLanguage();
    const char* testSsid = "Müller-Straße-Äußerst 5GHz";
    // Auftrag "WoL-Geraete", Schritt 7 - langer Name mit Umlauten, damit
    // Kacheln/Buttons mit demselben Extremtext geprueft werden wie die
    // SSID oben. Deckt 0/1/2/4 Geraete ab (die Grenzfaelle fuer Seite
    // "Geraete"-Sichtbarkeit und Seitenpunkte-Anzahl: 0->keine Kachel,
    // 1->keine Seite 3, 2->Seite 3 mit 1 Kachel, 4->voll/"hinzufuegen"
    // verschwindet).
    const char* longName = "Müller-Äußerst Laptop";
    const char* names4[4] = { "Acer Predator", "Test Laptop", "Büro-PC", longName };

    auto setDevices = [&](uint8_t count) {
        deviceCfg.wolDeviceCount = count;
        for (uint8_t i = 0; i < count; i++) {
            deviceCfg.wolDevices[i] = Storage::WolDevice{};
            deviceCfg.wolDevices[i].name = (count == 1) ? longName : names4[i];
            deviceCfg.wolDevices[i].mac = "AA:BB:CC:DD:EE:FF";
            deviceCfg.wolDevices[i].color = WOL_NEW_DEVICE_PALETTE[i % 4];
        }
    };

    for (int langIdx = 0; langIdx < 2; langIdx++) {
        const char* lang = (langIdx == 0) ? "de" : "en";
        I18n::setLanguage(lang);
        Serial.printf("[tour] Sprache=%s\n", lang);
        deviceCfg.wifiSsid = testSsid;

        for (uint8_t count : {0, 1, 2, 4}) {
            setDevices(count);
            currentPage = Page::Main;
            Serial.printf("[tour] Bildschirm: KeineVerbindung (%u Geraete)\n", count);
            drawNoConnectionScreen();
            delay(2000);
            currentPage = Page::Controls;
            Serial.printf("[tour] Bildschirm: Controls (%u Geraete)\n", count);
            drawControlsStaticLayout();
            delay(2000);
            if (count >= 2) {
                currentPage = Page::Devices;
                Serial.printf("[tour] Bildschirm: Geraete (%u Kacheln)\n", (unsigned)(count - 1));
                drawDevicesStaticLayout();
                delay(2000);
            }
        }
    }

    for (uint8_t i = 0; i < Config::MAX_WOL_DEVICES; i++) deviceCfg.wolDevices[i] = savedDevices[i];
    deviceCfg.wolDeviceCount = savedCount;
    deviceCfg.wifiSsid = savedSsid;
    currentPage = savedPage;
    I18n::setLanguage(savedLang);
    Serial.println("[tour] Ende");
}
#endif

} // namespace

void setup() {
    Serial.begin(115200);
    // Auftrag "Controls-Optik und USB-Sync", Teil B: sofort nach dem
    // Start EINMAL unaufgefordert "IBOARD_OK" senden, damit die Mac-App
    // das Geraet bemerkt, ohne erst selbst fragen zu muessen (sie fragt
    // zusaetzlich periodisch nach, siehe mac/usb_link.py::
    // _maybe_check_liveness() - doppelt haelt hier besser).
    Serial.print("IBOARD_OK\n");

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
    // Diagnose beim Start (Auftrag "WoL-Geraete") - zeigt u.a. die MAC
    // des ersten Geraets, damit sie nach einem Test-Loeschen wieder
    // eingegeben werden kann, ohne sie separat notiert haben zu muessen.
    Serial.printf("[Eiswolfs iBoard] WoL-Geraete: %u", deviceCfg.wolDeviceCount);
    if (deviceCfg.wolDeviceCount > 0) {
        Serial.printf(" (Geraet 1: \"%s\", %s)", deviceCfg.wolDevices[0].name.c_str(),
                      deviceCfg.wolDevices[0].mac.c_str());
    }
    Serial.println();
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
        tft.setTextDatum(TL_DATUM);
        // drawString()+TL_DATUM statt setCursor()+println() (Auftrag
        // "Umlaute und Umrandung", Schritt A3) - siehe Kommentar in
        // drawNoConnectionScreen() fuer den Grund. Zeilenabstand von
        // Hand, da println() bei der neuen Schrift sonst zu eng/zu
        // weit haette rechnen koennen.
        tft.drawString("Keine SD-Karte gefunden.", 10, 10);
        tft.drawString("Einstellungen werden nur", 10, 10 + tft.fontHeight() + 2);
        tft.drawString("intern (NVS) gespeichert.", 10, 10 + 2 * (tft.fontHeight() + 2));
        delay(2000);
    }

#ifdef SCREEN_TOUR
    runScreenTour();
#endif

    // Alexa nur starten, wenn im Menue eingeschaltet (Standard AUS) - sonst
    // wird NICHTS angelegt (kein Server, kein SSDP, kein Port 80).
    alexaActive = deviceCfg.alexaEnabled;
    if (alexaActive) {
    alexaNameActive = sanitizeAlexaName(deviceCfg.alexaName);
    if (alexaNameActive.length() == 0) alexaNameActive = Config::ALEXA_DEFAULT_NAME;
    // Geraet/Callback koennen schon vor einer bestehenden WLAN-Verbindung
    // angelegt werden - fauxmo.enable(true) (das die SSDP-Multicast-
    // Gruppe beitritt) passiert bewusst erst in loop(), sobald WiFi.status()
    // tatsaechlich WL_CONNECTED meldet (siehe dort).
    fauxmo.createServer(true);
    fauxmo.setPort(80);
    fauxmo.addDevice(alexaNameActive.c_str());
    fauxmo.onSetState([](unsigned char deviceId, const char* deviceName, bool state, unsigned char value) {
        (void)deviceId;
        (void)deviceName;
        (void)value;
        // Auftrag "Alexa-Test 2"/"Alexa Aus": "an" UND "aus" setzen NUR
        // ein Merker-Flag (AsyncTCP-Kontext, nichts Schweres hier) - der
        // eigentliche WoL-/Shutdown-Versand UND die 3s-Sperrpruefung
        // passieren in loop(), NIE hier. Ob "aus" wirklich etwas
        // ausloest (deviceCfg.alexaShutdownEnabled), entscheidet
        // ebenfalls erst loop() - siehe dort.
        if (state) {
            alexaWolRequested = true;
        } else {
            alexaShutdownRequested = true;
        }
    });
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

    if (alexaActive) {
    // Erst-Aktivierung NACH der ersten bestaetigten WLAN-Verbindung (siehe
    // Kommentar in setup()) - fauxmo.enable(true) tritt der SSDP-
    // Multicast-Gruppe (239.255.255.250:1900) bei, das braucht ein bereits
    // bestehendes Interface mit IP. WiFi.setSleep(false) ist dafuer schon
    // laengst an jeder WiFi.begin()-Stelle gesetzt (siehe oben/loop-Kopf),
    // keine eigene Absicherung hier noetig.
    if (!alexaStarted && WiFi.status() == WL_CONNECTED) {
        alexaStarted = true;
        fauxmo.enable(true);
        Serial.printf("[Eiswolfs iBoard] Alexa: Hue-Emulation \"%s\" aktiv, IP=%s\n",
                      alexaNameActive.c_str(), WiFi.localIP().toString().c_str());
    }
    if (alexaStarted) {
        fauxmo.handle();
    }

    // Auftrag "Alexa-Test 2": WoL fuer Geraet Nr. 1 (deviceCfg.wolDevices[0],
    // der migrierte Acer) - exakt dieselbe Funktion UND dieselbe 3s-Sperre
    // (deviceTapLockedUntilMs[0]/CTRL_TAP_LOCKOUT_MS) wie beim Touch auf
    // Controls/"Keine Verbindung" (siehe handleDeviceWidgets() oben), damit
    // ein doppelt zugestelltes Alexa-Kommando (kommt laut Auftrag manchmal
    // doppelt) nur EIN WoL-Paket auslöst.
    if (alexaWolRequested) {
        alexaWolRequested = false;
        if (deviceCfg.wolDeviceCount == 0) {
            Serial.println("[Eiswolfs iBoard] Alexa: kein WoL-Geraet eingerichtet");
        } else if (millis() < deviceTapLockedUntilMs[0]) {
            Serial.println("[Eiswolfs iBoard] Alexa: Befehl AN, Sperre aktiv, ignoriert");
        } else {
            // triggerWolForDevice() (Auftrag "Monitor-Umschaltung") kuemmert
            // sich ggf. selbst um den MONITOR-Befehl VOR dem WoL-Paket -
            // Alexa bekommt ihre HTTP-Antwort trotzdem sofort, da nur das
            // Merker-Flag hier in loop() verarbeitet wird, nicht im
            // AsyncTCP-Callback selbst.
            triggerWolForDevice(0);
            deviceTapLockedUntilMs[0] = millis() + CTRL_TAP_LOCKOUT_MS;
            Serial.printf("[Eiswolfs iBoard] Alexa: Befehl AN, WoL angefordert fuer \"%s\"\n",
                          deviceCfg.wolDevices[0].name.c_str());
            // Taster-Verhalten (Auftrag): nach kurzer Zeit selbst wieder auf
            // "aus" zuruecksetzen, damit der naechste "an"-Befehl fuer Alexa
            // immer ein Zustandswechsel ist (siehe alexaOffRevertAtMs unten).
            alexaOffRevertAtMs = millis() + 5000;
        }
    }
    if (alexaOffRevertAtMs != 0 && millis() >= alexaOffRevertAtMs) {
        alexaOffRevertAtMs = 0;
        fauxmo.setState((unsigned char)0, false, 0);
    }

    // Auftrag "Alexa Aus" - "aus" fuehrt (nur wenn eingeschaltet) zu
    // GENAU demselben Shutdown-Weg wie das lange Antippen
    // (sendDeviceShutdown(0), dieselbe 3s-Sperre deviceTapLockedUntilMs[0]
    // wie ueberall sonst) - der Windows-Listener schaltet dabei
    // unveraendert auch den Monitor zurueck. Kein eigener setState()-
    // Rucksprung noetig (anders als bei "an"): fauxmoESP hat Alexa schon
    // VOR diesem Callback "aus" gemeldet, das passt so oder so zum
    // Ergebnis (nichts getan oder tatsaechlich heruntergefahren).
    if (alexaShutdownRequested) {
        alexaShutdownRequested = false;
        if (!deviceCfg.alexaShutdownEnabled) {
            Serial.println("[Eiswolfs iBoard] Alexa: Befehl AUS (Schalter aus, keine Aktion)");
        } else if (deviceCfg.wolDeviceCount == 0) {
            Serial.println("[Eiswolfs iBoard] Alexa: kein WoL-Geraet eingerichtet");
        } else if (deviceCfg.wolDevices[0].shutdownIp.length() == 0) {
            Serial.println("[Eiswolfs iBoard] Alexa: IP noetig zum Herunterfahren");
            if (currentPage == Page::Controls) {
                startDeviceFeedback(DeviceWidgetStyle::Tile, ctrlTileRects[CTRL_ACER], 0, TFT_DARKGREY);
                UiText::Box box = { (int16_t)(ctrlTileRects[CTRL_ACER].x + 2), (int16_t)(ctrlTileRects[CTRL_ACER].y + 2),
                                     (int16_t)(ctrlTileRects[CTRL_ACER].w - 4), (int16_t)(ctrlTileRects[CTRL_ACER].h - 4) };
                UiText::draw(tft, box, I18n::t(StringId::ALEXA_SHUTDOWN_NO_IP), TFT_WHITE, TFT_DARKGREY, UiText::Align::Center, 2);
            } else if (showingNoConnection) {
                showDeviceNotSetUp(DeviceWidgetStyle::Button, noConnDeviceButtonRect(0), 0);
            }
        } else if (millis() < deviceTapLockedUntilMs[0]) {
            Serial.println("[Eiswolfs iBoard] Alexa: Befehl AUS, Sperre aktiv, ignoriert");
        } else {
            bool sent = sendDeviceShutdown(0);
            deviceTapLockedUntilMs[0] = millis() + CTRL_TAP_LOCKOUT_MS;
            if (sent) {
                Serial.printf("[Eiswolfs iBoard] Alexa: Befehl AUS, Shutdown gesendet an \"%s\"\n",
                              deviceCfg.wolDevices[0].name.c_str());
                if (currentPage == Page::Controls) {
                    startDeviceFeedback(DeviceWidgetStyle::Tile, ctrlTileRects[CTRL_ACER], 0, TFT_RED);
                } else if (showingNoConnection) {
                    startDeviceFeedback(DeviceWidgetStyle::Button, noConnDeviceButtonRect(0), 0, TFT_RED);
                }
            } else {
                Serial.println("[Eiswolfs iBoard] Alexa: Befehl AUS, Shutdown konnte nicht gesendet werden (kein WLAN)");
            }
        }
    }
    } // if (alexaActive)

    // Auftrag "Monitor-Umschaltung" - Fortsetzung von triggerWolForDevice()
    // (ausserhalb von if (alexaActive), da auch der normale Touch-Weg
    // ueber handleDeviceWidgets() hierueber laeuft). Kein delay() - wartet
    // ueber mehrere loop()-Durchlaeufe auf current.monitorAck.
    if (monitorPreWolState == MonitorPreWolState::WaitingForAck) {
        if (current.monitorAck != monitorAckBaselineForWol) {
            Serial.printf("[Eiswolfs iBoard] Monitor: Befehl gesendet, Quittung nach %ums\n",
                          (unsigned)(millis() - monitorCmdSentMs));
            sendWolPacket(deviceCfg.wolDevices[monitorPendingWolDeviceIdx].mac);
            monitorPreWolState = MonitorPreWolState::Idle;
        } else if (millis() - monitorCmdSentMs >= Config::MONITOR_ACK_TIMEOUT_MS) {
            Serial.println("[Eiswolfs iBoard] Monitor: keine Quittung, WoL trotzdem gesendet");
            sendWolPacket(deviceCfg.wolDevices[monitorPendingWolDeviceIdx].mac);
            monitorPreWolState = MonitorPreWolState::Idle;
        }
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

    uint32_t lastAnyPacketMs = computeLastAnyPacketMs();

    bool noData = (millis() - lastAnyPacketMs > Config::NO_DATA_TIMEOUT_MS) || lastAnyPacketMs == 0;

    // Die Umschaltung auf "Keine Verbindung zum Mac" betrifft NUR den
    // Hauptscreen (Auftrag "Controls-Screen") - auf Controls bleibt man
    // trotzdem, der Zustand wird nur gemerkt und beim naechsten Wechsel
    // zurueck zu Main passend angezeigt (siehe handleControlsPage()).
    // WAEHREND MacSleepState::Active bewusst gesperrt (Auftrag
    // "Ruhezustand-Fix", Schritt 4): sonst wuerde hier schon ein
    // einzelnes, isoliertes Paket (z.B. aus einem DarkWake) sofort das
    // Dashboard zeichnen, BEVOR updateMacSleepState() seine eigene
    // "anhaltende Daten"-Pruefung (WAKE_SUSTAIN_MS) ueberhaupt starten
    // konnte - die Entscheidung liegt in diesem Zustand daher komplett
    // bei updateMacSleepState() selbst (zeichnet bei bestaetigtem
    // Aufwachen auch selbst, siehe dort).
    if (noData != showingNoConnection && macSleepState != MacSleepState::Active) {
        showingNoConnection = noData;
        if (currentPage == Page::Main) {
            if (showingNoConnection) {
                drawNoConnectionScreen();
            } else {
                drawStaticLayout();
                lastDrawnValid = false;
            }
        }
    }

    // Auftrag "Display aus im Ruhezustand" - NACH der obigen Umschaltung
    // (falls echte Daten gerade das Dashboard neu gezeichnet haben, geht
    // die Beleuchtung hier erst danach wieder an).
    updateMacSleepState();

    // Waehrend "Mac schlaeft absichtlich" UND Beleuchtung dauerhaft aus
    // (nicht per Beruehrung geweckt): jede Beruehrung wird hier verschluckt
    // (weckt nur die Beleuchtung fuer TOUCH_WAKE_MS, loest aber
    // KEINE Kachel/Taste/Wisch aus) - TouchCalib::pollGesture() konsumiert
    // die Geste, der normale Dispatch unten sieht danach keine Geste mehr
    // fuer diese Runde (Auftrag Schritt 4).
    if (macSleepState == MacSleepState::Active && macSleepWokenAtMs == 0) {
        TouchCalib::Point tap;
        TouchCalib::Gesture g = TouchCalib::pollGesture(tap);
        if (g != TouchCalib::Gesture::None) {
            macSleepWokenAtMs = millis();
            applyBrightness();
            drawNoConnectionScreen();
            Serial.println("[Eiswolfs iBoard] Mac-Schlaf: Beruehrung, 15s an");
        }
        return;
    }

    if (currentPage == Page::Controls) {
        handleControlsPage();
    } else if (currentPage == Page::Devices) {
        handleDevicesPage();
    } else if (showingNoConnection) {
        // Wischen nach Controls geht auch hier (Auftrag) - z.B. um den
        // Laptop per WoL zu wecken, auch ohne Mac-Verbindung. Der
        // "Menue"-Button bleibt zusaetzlich bestehen.

        // Auftrag "WoL-Button auf Keine Verbindung", Schritt 3: JEDE
        // Beruehrung auf diesem Screen haelt die Beleuchtung wach (15s
        // ab der LETZTEN Beruehrung, nicht nur der ersten) - sonst wuerde
        // ein Halten fuer den Shutdown-Button mitten im Halten abbrechen.
        // Nur relevant, wenn bereits per Beruehrung geweckt
        // (macSleepWokenAtMs!=0) - die ALLERERSTE Beruehrung im Dunkeln
        // bleibt unveraendert verschluckt (Block weiter oben, endet mit
        // return, dieser Zweig wird in jener Runde gar nicht erreicht).
        if (macSleepState == MacSleepState::Active && macSleepWokenAtMs != 0 &&
            TouchCalib::rawPoint().touched) {
            macSleepWokenAtMs = millis();
        }

        TouchCalib::Point tap;
        TouchCalib::Gesture g = TouchCalib::pollGesture(tap);
        if (g == TouchCalib::Gesture::SwipeLeft) {
            currentPage = Page::Controls;
            drawControlsStaticLayout();
        } else if (g == TouchCalib::Gesture::Tap && setupButtonRect.contains(tap.x, tap.y)) {
            runMainMenu();
            drawNoConnectionScreen();
        }

        // Geraete-Buttonspalte (Auftrag "WoL-Geraete") - dieselbe
        // gemeinsame Tap/Halten-Logik wie auf Controls/Geraete-Seite,
        // fuer alle sichtbaren Buttons (0 bis wolDeviceCount-1).
        if (deviceCfg.wolDeviceCount > 0) {
            Rect rects[Config::MAX_WOL_DEVICES];
            uint8_t idxs[Config::MAX_WOL_DEVICES];
            for (uint8_t i = 0; i < deviceCfg.wolDeviceCount; i++) {
                rects[i] = noConnDeviceButtonRect(i);
                idxs[i] = i;
            }
            handleDeviceWidgets(DeviceWidgetStyle::Button, rects, idxs, deviceCfg.wolDeviceCount, g, tap);
        }
    } else {
        // Hauptscreen ist seit Auftrag "Controls-Screen" reine Anzeige -
        // die einzige Geste hier ist das Wischen nach Controls.
        TouchCalib::Point tap;
        TouchCalib::Gesture g = TouchCalib::pollGesture(tap);
        if (g == TouchCalib::Gesture::SwipeLeft) {
            currentPage = Page::Controls;
            drawControlsStaticLayout();
        }

        if (current.valid) {
            updateDisplay();
        }
    }

    delay(20);
}
