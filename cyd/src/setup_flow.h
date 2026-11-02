#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "storage.h"

namespace SetupFlow {

    // Ersteinrichtung komplett: Touch-Kalibrierung + WLAN-Auswahl +
    // Passwort + Verbindungstest. Loeschen bei Fehlschlag automatisch
    // wiederholt, kehrt erst bei Erfolg zurueck. Schreibt und speichert
    // die neue Konfiguration (SD oder NVS).
    void runFullSetup(TFT_eSPI& tft, Storage::DeviceConfig& cfg);

    // Nur WLAN-Teil neu (Kalibrierung bleibt wie sie ist).
    void runWifiOnly(TFT_eSPI& tft, Storage::DeviceConfig& cfg);

    // Nur Touch-Kalibrierung neu (WLAN bleibt wie es ist).
    void runCalibrationOnly(TFT_eSPI& tft, Storage::DeviceConfig& cfg);
}
