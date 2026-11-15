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

    // Einmalige Migration (Auftrag "Umlaute und Umrandung"): Geraete, die
    // schon VOR der neuen Schrift (Schritt A0) geflasht wurden, haben den
    // damaligen ASCII-Platzhalter "Luefter Mac" bereits fest in ihrer
    // config.json gespeichert - die normale additive Regel (bestehende
    // Werte NIE ueberschreiben) wuerde das fuer immer so belassen. Das
    // ist aber KEINE eigene Eingabe von Alex, sondern nur der damalige
    // Code-Standardwert - deshalb hier die EINZIGE gezielte Ausnahme:
    // exakt dieser eine alte Platzhalter-Text wird beim Laden durch den
    // neuen Standardwert mit echtem Umlaut ersetzt. Ein selbst
    // eingetragener Name (jeder andere Text) bleibt unberuehrt.
    void migrateLabelFan(String& labelFan) {
        if (labelFan == "Luefter Mac") labelFan = "Lüfter Mac";
    }

    // WoL-Geraeteliste lesen/schreiben (Auftrag "WoL-Geraete") - eigene
    // Helfer, von parseJson()/toJson() UND loadFromPrefs()/saveToPrefs()
    // genutzt. "wol_devices" fehlt bei einer aelteren config.json
    // komplett (leeres Array) - die Migration passiert zentral in der
    // oeffentlichen load()-Funktion unten, NICHT hier.
    void readWolDevicesJson(JsonVariant arr, DeviceConfig& out) {
        out.wolDeviceCount = 0;
        if (!arr.is<JsonArray>()) return;
        for (JsonVariant v : arr.as<JsonArray>()) {
            if (out.wolDeviceCount >= Config::MAX_WOL_DEVICES) break;
            WolDevice& d = out.wolDevices[out.wolDeviceCount];
            d.name = String((const char*)(v["name"] | ""));
            d.mac = String((const char*)(v["mac"] | ""));
            d.color = (uint16_t)(v["color"] | 0);
            d.shutdownIp = String((const char*)(v["shutdown_ip"] | ""));
            d.legacyBroadcastShutdown = v["legacy_broadcast_shutdown"] | false;
            d.monitorInputOnPowerOn = (uint8_t)(v["monitor_input"] | 0);
            out.wolDeviceCount++;
        }
    }

    void writeWolDevicesJson(JsonDocument& doc, const DeviceConfig& cfg) {
        JsonArray arr = doc.createNestedArray("wol_devices");
        for (uint8_t i = 0; i < cfg.wolDeviceCount; i++) {
            const WolDevice& d = cfg.wolDevices[i];
            JsonObject o = arr.createNestedObject();
            o["name"] = d.name;
            o["mac"] = d.mac;
            o["color"] = d.color;
            o["shutdown_ip"] = d.shutdownIp;
            o["legacy_broadcast_shutdown"] = d.legacyBroadcastShutdown;
            o["monitor_input"] = d.monitorInputOnPowerOn;
        }
    }

    bool parseJson(const String& jsonStr, DeviceConfig& out) {
        // Groesser als vorher (896 -> 2560), seit die WoL-Geraeteliste
        // (bis zu 4 Eintraege mit Name/MAC/Farbe/IP/Flag) dazukam.
        StaticJsonDocument<2560> doc;
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
        out.wolMacAddress = String((const char*)(doc["wol_mac"] | ""));
        out.cmdCounter = doc["cmd_counter"] | 0UL;
        out.labelLaptop = String((const char*)(doc["label_laptop"] | "Laptop"));
        out.labelMac = String((const char*)(doc["label_mac"] | "Mac Mini M4"));
        out.labelFan = String((const char*)(doc["label_fan"] | "Lüfter Mac"));
        migrateLabelFan(out.labelFan);
        readWolDevicesJson(doc["wol_devices"], out);
        out.alexaShutdownEnabled = doc["alexa_shutdown_enabled"] | false;
        out.alexaEnabled = doc["alexa_enabled"] | false;
        out.alexaName = String((const char*)(doc["alexa_name"] | "Laptop"));

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
        // Groesser als vorher (896 -> 2560), siehe parseJson() oben.
        StaticJsonDocument<2560> doc;
        doc["wifi_ssid"] = cfg.wifiSsid;
        doc["wifi_password"] = cfg.wifiPassword;
        doc["mac_ip"] = cfg.macIp;
        doc["font"] = cfg.font;
        doc["source"] = cfg.source;
        doc["language"] = cfg.language;
        doc["fan_boost_sec"] = cfg.fanBoostSec;
        doc["fan_boost_rpm"] = cfg.fanBoostRpm;
        doc["brightness"] = cfg.brightness;
        // wol_mac/label_laptop bleiben mitgeschrieben (Abwaertskompatibilitaet
        // zu einer aelteren Firmware, die nur diese beiden Felder kennt) -
        // gespiegelt aus Eintrag Nr. 1 der neuen Liste, falls vorhanden,
        // sonst die bisherigen Werte unveraendert.
        doc["wol_mac"] = (cfg.wolDeviceCount > 0) ? cfg.wolDevices[0].mac : cfg.wolMacAddress;
        doc["cmd_counter"] = cfg.cmdCounter;
        doc["label_laptop"] = (cfg.wolDeviceCount > 0) ? cfg.wolDevices[0].name : cfg.labelLaptop;
        doc["label_mac"] = cfg.labelMac;
        doc["label_fan"] = cfg.labelFan;
        doc["alexa_shutdown_enabled"] = cfg.alexaShutdownEnabled;
        doc["alexa_enabled"] = cfg.alexaEnabled;
        doc["alexa_name"] = cfg.alexaName;
        writeWolDevicesJson(doc, cfg);
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
        out.wolMacAddress = prefs.getString("wol_mac", "");
        out.cmdCounter = prefs.getULong("cmd_counter", 0);
        out.labelLaptop = prefs.getString("label_laptop", "Laptop");
        out.labelMac = prefs.getString("label_mac", "Mac Mini M4");
        out.labelFan = prefs.getString("label_fan", "Lüfter Mac");
        migrateLabelFan(out.labelFan);
        out.alexaShutdownEnabled = prefs.getBool("alexa_sdn", false);
        out.alexaEnabled = prefs.getBool("alexa_en", false);
        out.alexaName = prefs.getString("alexa_nm", "Laptop");
        out.wolDeviceCount = (uint8_t)min((int)prefs.getUChar("wol_cnt", 0), (int)Config::MAX_WOL_DEVICES);
        for (uint8_t i = 0; i < out.wolDeviceCount; i++) {
            char key[16];
            WolDevice& d = out.wolDevices[i];
            snprintf(key, sizeof(key), "wol%u_name", i); d.name = prefs.getString(key, "");
            snprintf(key, sizeof(key), "wol%u_mac", i); d.mac = prefs.getString(key, "");
            snprintf(key, sizeof(key), "wol%u_col", i); d.color = (uint16_t)prefs.getUShort(key, 0);
            snprintf(key, sizeof(key), "wol%u_sip", i); d.shutdownIp = prefs.getString(key, "");
            snprintf(key, sizeof(key), "wol%u_leg", i); d.legacyBroadcastShutdown = prefs.getBool(key, false);
            snprintf(key, sizeof(key), "wol%u_min", i); d.monitorInputOnPowerOn = (uint8_t)prefs.getUChar(key, 0);
        }
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
        prefs.putString("wol_mac", (cfg.wolDeviceCount > 0) ? cfg.wolDevices[0].mac : cfg.wolMacAddress);
        prefs.putULong("cmd_counter", cfg.cmdCounter);
        prefs.putString("label_laptop", (cfg.wolDeviceCount > 0) ? cfg.wolDevices[0].name : cfg.labelLaptop);
        prefs.putString("label_mac", cfg.labelMac);
        prefs.putString("label_fan", cfg.labelFan);
        prefs.putBool("alexa_sdn", cfg.alexaShutdownEnabled);
        prefs.putBool("alexa_en", cfg.alexaEnabled);
        prefs.putString("alexa_nm", cfg.alexaName);
        prefs.putUChar("wol_cnt", cfg.wolDeviceCount);
        for (uint8_t i = 0; i < cfg.wolDeviceCount; i++) {
            char key[16];
            const WolDevice& d = cfg.wolDevices[i];
            snprintf(key, sizeof(key), "wol%u_name", i); prefs.putString(key, d.name);
            snprintf(key, sizeof(key), "wol%u_mac", i); prefs.putString(key, d.mac);
            snprintf(key, sizeof(key), "wol%u_col", i); prefs.putUShort(key, d.color);
            snprintf(key, sizeof(key), "wol%u_sip", i); prefs.putString(key, d.shutdownIp);
            snprintf(key, sizeof(key), "wol%u_leg", i); prefs.putBool(key, d.legacyBroadcastShutdown);
            snprintf(key, sizeof(key), "wol%u_min", i); prefs.putUChar(key, d.monitorInputOnPowerOn);
        }
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

namespace {
    // Einmalige Migration (Auftrag "WoL-Geraete"): existiert die neue
    // Geraeteliste noch nicht (wolDeviceCount==0), aber eine alte WoL-
    // MAC, wird daraus im RAM Eintrag Nr. 1 erzeugt - Name aus
    // labelLaptop (hat selbst schon den Standard "Laptop"),
    // Farbe = das bisherige Gelb der Acer-Kachel (0xFFE0, siehe
    // drawCtrlAcerIdle() in main.cpp), legacyBroadcastShutdown=true
    // (erhaelt das heutige Broadcast-Verhalten beim Ausschalten). Wird
    // NICHT hier geschrieben (kein SD-Zugriff aus load() heraus) -
    // erst der naechste normale Storage::save() (z.B. sobald Alex
    // etwas in den neuen Einstellungen aendert) persistiert das
    // Ergebnis. Gibt es weder Liste noch alte WoL-MAC, bleibt die
    // Liste leer ("kein Geraet eingerichtet").
    void migrateWolDevices(DeviceConfig& out) {
        if (out.wolDeviceCount > 0) return;
        if (out.wolMacAddress.length() == 0) return;
        WolDevice& d = out.wolDevices[0];
        d.name = out.labelLaptop;
        d.mac = out.wolMacAddress;
        d.color = 0xFFE0; // TFT_YELLOW
        d.shutdownIp = "";
        d.legacyBroadcastShutdown = true;
        out.wolDeviceCount = 1;
    }
}

bool load(DeviceConfig& out) {
    bool ok = loadFromSd(out) || loadFromPrefs(out);
    if (ok) migrateWolDevices(out);
    return ok;
}

bool save(const DeviceConfig& cfg) {
    if (sdMounted && saveToSd(cfg)) return true;
    return saveToPrefs(cfg);
}

}
