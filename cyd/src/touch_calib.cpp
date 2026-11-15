#include "touch_calib.h"
#include "config.h"
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

namespace TouchCalib {

namespace {
    SPIClass touchSpi(VSPI);
    XPT2046_Touchscreen touch(Config::TOUCH_CS_PIN, Config::TOUCH_IRQ_PIN);

    Calibration cal;
    bool lastTouched = false;
    Point lastMapped;

    int16_t clampi(int16_t v, int16_t lo, int16_t hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    // Loest das 3x3-Gleichungssystem A*x = b per Cramer-Regel.
    bool solve3x3(const double A[3][3], const double bvec[3], double x[3]) {
        auto det3 = [](const double m[3][3]) {
            return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                 - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                 + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        };
        double detA = det3(A);
        if (fabs(detA) < 1e-9) return false;

        for (int col = 0; col < 3; col++) {
            double M[3][3];
            for (int r = 0; r < 3; r++)
                for (int cc = 0; cc < 3; cc++)
                    M[r][cc] = (cc == col) ? bvec[r] : A[r][cc];
            x[col] = det3(M) / detA;
        }
        return true;
    }
}

void begin() {
    touchSpi.begin(Config::TOUCH_CLK_PIN, Config::TOUCH_MISO_PIN,
                    Config::TOUCH_MOSI_PIN, Config::TOUCH_CS_PIN);
    touch.begin(touchSpi);
    touch.setRotation(0);
}

void setCalibration(const Calibration& c) { cal = c; }
Calibration calibration() { return cal; }
bool hasCalibration() { return cal.valid; }

Point rawPoint() {
    Point p;
    p.touched = touch.touched();
    if (p.touched) {
        TS_Point raw = touch.getPoint();
        p.x = raw.x;
        p.y = raw.y;
    }
    return p;
}

Point applyCalibration(const Point& raw) {
    Point out;
    out.touched = raw.touched;
    if (!raw.touched || !cal.valid) return out;

    float sx = cal.a * raw.x + cal.b * raw.y + cal.c;
    float sy = cal.d * raw.x + cal.e * raw.y + cal.f;

    out.x = clampi((int16_t)sx, 0, Config::SCREEN_WIDTH - 1);
    out.y = clampi((int16_t)sy, 0, Config::SCREEN_HEIGHT - 1);
    return out;
}

Point mappedPoint() {
    return applyCalibration(rawPoint());
}

bool wasTapped(Point& outPoint) {
    Point cur = mappedPoint();
    bool fired = false;
    if (!cur.touched && lastTouched) {
        outPoint = lastMapped;
        fired = true;
    }
    lastTouched = cur.touched;
    if (cur.touched) lastMapped = cur;
    return fired;
}

// Seiten-Wischen + Antippen (Auftrag "Controls-Screen", strengere
// Erkennung durch Auftrag "Controls-Bedienung") - eigener Zustand,
// unabhaengig von wasTapped() oben. Klassifiziert erst beim (entprellten)
// Loslassen:
//  - Wischen: waagerechte Strecke >= SWIPE_MIN_DX_PX, groesste senkrechte
//    Abweichung <= SWIPE_MAX_DY_PX, waagerecht mindestens
//    SWIPE_MIN_HORIZONTAL_RATIO-mal so viel wie die senkrechte Abweichung,
//    Dauer >= SWIPE_MIN_HOLD_MS.
//  - Antippen: die GROESSTE Bewegung waehrend der GANZEN Beruehrung (nicht
//    nur Start/Ende) bleibt unter TAP_MAX_MOVE_PX, UND die Dauer liegt
//    zwischen TAP_MIN_HOLD_MS und TAP_MAX_HOLD_MS.
//  - Alles andere (z.B. eine missglueckte, zu kurze/zu schraege Wisch-
//    geste, die weder die Wisch- noch die Antippen-Kriterien erfuellt)
//    verfaellt ohne Wirkung (Gesture::None) - loest NIE eine Kachel aus.
// Der Aufsetzpunkt ist der Mittelwert der ersten bis zu drei Messwerte
// (der allererste Messwert eines resistiven Touchscreens ist oft
// ungenau) - tapOut liefert diesen (stabilen) Punkt, nicht den
// Loslasspunkt.
Gesture pollGesture(Point& tapOut) {
    static bool pressActive = false;
    static Point pressStart;
    static Point pressLast;
    static uint32_t pressStartMs = 0;
    static int16_t maxAbsDy = 0;
    static int16_t maxMove = 0;
    static Point firstSamples[3];
    static uint8_t firstSampleCount = 0;
    static bool releasePending = false;
    static uint32_t releaseStartMs = 0;

    Point cur = mappedPoint();

    if (cur.touched) {
        releasePending = false;
        if (!pressActive) {
            pressActive = true;
            pressStartMs = millis();
            maxAbsDy = 0;
            maxMove = 0;
            firstSampleCount = 0;
        }
        if (firstSampleCount < 3) {
            firstSamples[firstSampleCount++] = cur;
            int32_t sx = 0, sy = 0;
            for (uint8_t i = 0; i < firstSampleCount; i++) {
                sx += firstSamples[i].x;
                sy += firstSamples[i].y;
            }
            pressStart.x = (int16_t)(sx / firstSampleCount);
            pressStart.y = (int16_t)(sy / firstSampleCount);
        }
        int16_t dxNow = (int16_t)abs((int)cur.x - (int)pressStart.x);
        int16_t dyNow = (int16_t)abs((int)cur.y - (int)pressStart.y);
        if (dyNow > maxAbsDy) maxAbsDy = dyNow;
        int16_t moveNow = (dxNow > dyNow) ? dxNow : dyNow; // groesste Bewegung in irgendeine Richtung
        if (moveNow > maxMove) maxMove = moveNow;
        pressLast = cur;
        return Gesture::None;
    }

    if (!pressActive) return Gesture::None;

    // Entprellung: ein kurzer Aussetzer des resistiven Sensors darf das
    // Loslassen nicht vorzeitig ausloesen (gleiches Muster wie die
    // bestehenden Lang-Antippen-Griffe in main.cpp). Ein ECHTES erneutes
    // Aufsetzen nach Ablauf der Entprellung zaehlt als NEUE Beruehrung
    // (pressActive ist dann schon false, siehe oben) und beeinflusst die
    // bereits beendete Beruehrung nicht mehr.
    if (!releasePending) {
        releasePending = true;
        releaseStartMs = millis();
        return Gesture::None;
    }
    if (millis() - releaseStartMs < Config::TOUCH_RELEASE_DEBOUNCE_MS) {
        return Gesture::None;
    }

    pressActive = false;
    releasePending = false;

    int16_t dx = (int16_t)((int)pressLast.x - (int)pressStart.x);
    uint32_t heldMs = releaseStartMs - pressStartMs;

    bool horizontalDominant = (float)abs(dx) >= Config::SWIPE_MIN_HORIZONTAL_RATIO * (float)maxAbsDy;
    if (abs(dx) >= Config::SWIPE_MIN_DX_PX && maxAbsDy <= Config::SWIPE_MAX_DY_PX &&
        heldMs >= Config::SWIPE_MIN_HOLD_MS && horizontalDominant) {
        return dx < 0 ? Gesture::SwipeLeft : Gesture::SwipeRight;
    }

    if (maxMove <= Config::TAP_MAX_MOVE_PX && heldMs >= Config::TAP_MIN_HOLD_MS &&
        heldMs <= Config::TAP_MAX_HOLD_MS) {
        tapOut = pressStart;
        return Gesture::Tap;
    }

    // Weder sauberes Antippen noch gueltiger Wisch (z.B. missglueckte,
    // zu kurze/zu schraege Wischgeste) - verfaellt ohne Wirkung.
    return Gesture::None;
}

Calibration fit(const Point targets[4], const Point raws[4]) {
    Calibration result;

    // Gemeinsame Matrix A (haengt nur von den Rohpunkten ab), zwei
    // rechte Seiten (Ziel-X bzw. Ziel-Y) - siehe Herleitung in
    // CLAUDE.md ("Touch-Kalibrierung").
    double Sx = 0, Sy = 0, Sxx = 0, Syy = 0, Sxy = 0, n = 4;
    double SxTx = 0, SyTx = 0, STx = 0;
    double SxTy = 0, SyTy = 0, STy = 0;

    for (int i = 0; i < 4; i++) {
        double rx = raws[i].x, ry = raws[i].y;
        double tx = targets[i].x, ty = targets[i].y;
        Sx += rx; Sy += ry; Sxx += rx * rx; Syy += ry * ry; Sxy += rx * ry;
        SxTx += rx * tx; SyTx += ry * tx; STx += tx;
        SxTy += rx * ty; SyTy += ry * ty; STy += ty;
    }

    double A[3][3] = {
        { Sxx, Sxy, Sx },
        { Sxy, Syy, Sy },
        { Sx,  Sy,  n  },
    };

    double bx[3] = { SxTx, SyTx, STx };
    double by[3] = { SxTy, SyTy, STy };
    double coefX[3], coefY[3];

    if (!solve3x3(A, bx, coefX) || !solve3x3(A, by, coefY)) {
        result.valid = false;
        return result;
    }

    result.a = (float)coefX[0];
    result.b = (float)coefX[1];
    result.c = (float)coefX[2];
    result.d = (float)coefY[0];
    result.e = (float)coefY[1];
    result.f = (float)coefY[2];
    result.valid = true;
    return result;
}

}
