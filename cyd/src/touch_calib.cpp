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
