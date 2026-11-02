#pragma once
#include <Arduino.h>

namespace TouchCalib {

    struct Point {
        int16_t x = 0;
        int16_t y = 0;
        bool touched = false;
        Point() = default;
        Point(int16_t x_, int16_t y_, bool touched_) : x(x_), y(y_), touched(touched_) {}
    };

    // Affine Abbildung roh->Bildschirm: screenX = a*rawX + b*rawY + c,
    // screenY = d*rawX + e*rawY + f. Robust gegenueber Achsentausch/
    // Spiegelung durch die Display-Rotation, da per 4-Punkt-Fit bestimmt
    // statt fest angenommen.
    struct Calibration {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        bool valid = false;
    };

    void begin();

    void setCalibration(const Calibration& cal);
    Calibration calibration();
    bool hasCalibration();

    // Unkalibrierte Rohdaten direkt vom XPT2046.
    Point rawPoint();

    // Wendet die aktuelle Kalibrierung auf einen Rohpunkt an und clamped
    // auf den Bildschirm.
    Point applyCalibration(const Point& raw);

    // Kalibrierte, geclampte Position - nur gueltig, wenn hasCalibration().
    Point mappedPoint();

    // Liefert true genau einmal pro Beruehrung (beim Loslassen).
    bool wasTapped(Point& outPoint);

    // Berechnet aus 4 (rawPoint, screenTarget)-Paaren per kleinste-Quadrate
    // eine neue Calibration. targets/raws je 4 Eintraege.
    Calibration fit(const Point targets[4], const Point raws[4]);
}
