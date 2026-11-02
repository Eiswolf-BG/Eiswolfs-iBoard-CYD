#include "storage.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>
#include <Preferences.h>
#include <ArduinoJson.h>

namespace Storage {

namespace {
    SPIClass sdSpi(HSPI);
    bool sdMounted = false;
    Preferences prefs;
    constexpr const char* PREFS_NAMESPACE = "iboard";

    void ensureConfigDir() {
        if (!SD.exists(Config::SD_CONFIG_DIR)) {
            SD.mkdir(Config::SD_CONFIG_DIR);
        }
    }

    bool parseJson(const String& jsonStr, DeviceConfig& out) {
        StaticJsonDocument<768> doc;
        DeserializationError err = deserializeJson(doc, jsonStr);
        if (err) return false;

        if (!doc["wifi_ssid"].is<const char*>()) return false;

        out.wifiSsid = doc["wifi_ssid"].as<const char*>();
        out.wifiPassword = doc["wifi_password"] | "";
        out.macIp = doc["mac_ip"] | "";
        // additiv (Auftrag 29.10.): fehlt das Feld in einer aelteren
        // config.json, gilt der Standard "pixel" - bestehende Dateien
        // ohne dieses Feld bleiben dadurch gueltig/kompatibel.
        out.font = String((const char*)(doc["font"] | "pixel"));
        out.source = String((const char*)(doc["source"] | "auto"));
        out.language = String((const char*)(doc["language"] | "de"));
        out.fanBoostSec = doc["fan_boost_sec"] | 60;
        out.fanBoostRpm = String((const char*)(doc["fan_boost_rpm"] | "MAX"));
        out.brightness = doc["brightness"] | 100;

        JsonVariant t = doc["touch_cal"];
        if (t.is<JsonObject>()) {
            out.touchCal.a = t["a"] | 0.0f;
            out.touchCal.b = t["b"] | 0.0f;
            out.touchCal.c = t["c"] | 0.0f;
            out.touchCal.d = t["d"] | 0.0f;
            out.touchCal.e = t["e"] | 0.0f;
            out.touchCal.f = t["f"] | 0.0f;
            out.touchCal.valid = true;
        } else {
            out.touchCal.valid = false;
        }

        out.valid = out.wifiSsid.length() > 0 && out.touchCal.valid;
        return out.valid;
    }

    String toJson(const DeviceConfig& cfg) {
        StaticJsonDocument<768> doc;
        doc["wifi_ssid"] = cfg.wifiSsid;
        doc["wifi_password"] = cfg.wifiPassword;
        doc["mac_ip"] = cfg.macIp;
        doc["font"] = cfg.font;
        doc["source"] = cfg.source;
        doc["language"] = cfg.language;
        doc["fan_boost_sec"] = cfg.fanBoostSec;
        doc["fan_boost_rpm"] = cfg.fanBoostRpm;
        doc["brightness"] = cfg.brightness;
        JsonObject t = doc.createNestedObject("touch_cal");
        t["a"] = cfg.touchCal.a;
        t["b"] = cfg.touchCal.b;
        t["c"] = cfg.touchCal.c;
        t["d"] = cfg.touchCal.d;
        t["e"] = cfg.touchCal.e;
        t["f"] = cfg.touchCal.f;

        String out;
        serializeJson(doc, out);
        return out;
    }

    bool loadFromSd(DeviceConfig& out) {
        if (!sdMounted) return false;
        if (!SD.exists(Config::SD_CONFIG_FILE)) return false;
        File f = SD.open(Config::SD_CONFIG_FILE, FILE_READ);
        if (!f) return false;
        String content = f.readString();
        f.close();
        return parseJson(content, out);
    }

    bool saveToSd(const DeviceConfig& cfg) {
        if (!sdMounted) return false;
        ensureConfigDir();
        File f = SD.open(Config::SD_CONFIG_FILE, FILE_WRITE);
        if (!f) return false;
        String json = toJson(cfg);
        f.print(json);
        f.close();
        return true;
    }

    bool loadFromPrefs(DeviceConfig& out) {
        prefs.begin(PREFS_NAMESPACE, true);
        out.wifiSsid = prefs.getString("wifi_ssid", "");
        out.wifiPassword = prefs.getString("wifi_password", "");
        out.macIp = prefs.getString("mac_ip", "");
        out.font = prefs.getString("font", "pixel");
        out.source = prefs.getString("source", "auto");
        out.language = prefs.getString("language", "de");
        out.fanBoostSec = prefs.getInt("fan_bst_sec", 60);
        out.fanBoostRpm = prefs.getString("fan_bst_rpm", "MAX");
        out.brightness = prefs.getInt("brightness", 100);
        out.touchCal.a = prefs.getFloat("cal_a", 0);
        out.touchCal.b = prefs.getFloat("cal_b", 0);
        out.touchCal.c = prefs.getFloat("cal_c", 0);
        out.touchCal.d = prefs.getFloat("cal_d", 0);
        out.touchCal.e = prefs.getFloat("cal_e", 0);
        out.touchCal.f = prefs.getFloat("cal_f", 0);
        bool hasCal = prefs.getBool("cal_valid", false);
        prefs.end();

        out.touchCal.valid = hasCal;
        out.valid = out.wifiSsid.length() > 0 && hasCal;
        return out.valid;
    }

    bool saveToPrefs(const DeviceConfig& cfg) {
        prefs.begin(PREFS_NAMESPACE, false);
        prefs.putString("wifi_ssid", cfg.wifiSsid);
        prefs.putString("wifi_password", cfg.wifiPassword);
        prefs.putString("mac_ip", cfg.macIp);
        prefs.putString("font", cfg.font);
        prefs.putString("source", cfg.source);
        prefs.putString("language", cfg.language);
        prefs.putInt("fan_bst_sec", cfg.fanBoostSec);
        prefs.putString("fan_bst_rpm", cfg.fanBoostRpm);
        prefs.putInt("brightness", cfg.brightness);
        prefs.putFloat("cal_a", cfg.touchCal.a);
        prefs.putFloat("cal_b", cfg.touchCal.b);
        prefs.putFloat("cal_c", cfg.touchCal.c);
        prefs.putFloat("cal_d", cfg.touchCal.d);
        prefs.putFloat("cal_e", cfg.touchCal.e);
        prefs.putFloat("cal_f", cfg.touchCal.f);
        prefs.putBool("cal_valid", cfg.touchCal.valid);
        prefs.end();
        return true;
    }
}

bool sdAvailable() { return sdMounted; }

void begin() {
    sdSpi.begin(Config::SD_SPI_CLK_PIN, Config::SD_SPI_MISO_PIN,
                Config::SD_SPI_MOSI_PIN, Config::SD_SPI_CS_PIN);
    sdMounted = SD.begin(Config::SD_SPI_CS_PIN, sdSpi, 4000000);
    if (sdMounted) ensureConfigDir();
}

bool load(DeviceConfig& out) {
    if (loadFromSd(out)) return true;
    return loadFromPrefs(out);
}

bool save(const DeviceConfig& cfg) {
    if (sdMounted && saveToSd(cfg)) return true;
    return saveToPrefs(cfg);
}

}
