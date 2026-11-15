#include "cmd_auth.h"
#include "secrets.h"
#include <mbedtls/md.h>

namespace CmdAuth {

namespace {
    // Erste 8 Byte (16 Hex-Zeichen) des HMAC-SHA256 - bewusst gekuerzt
    // (volle 32 Byte waeren fuer eine Textzeile unhandlich), aber mit
    // 64 Bit immer noch praktisch nicht erratbar/faelschbar fuer eine
    // Bedrohung "jemand im selben LAN", siehe docs/TOKEN.md.
    constexpr size_t HMAC_HEX_LEN = 16;

    String hmacHex(const String& message) {
        const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        uint8_t fullHmac[32];
        mbedtls_md_hmac(
            mdInfo,
            (const uint8_t*)FAN_BOOST_TOKEN, strlen(FAN_BOOST_TOKEN),
            (const uint8_t*)message.c_str(), message.length(),
            fullHmac
        );

        char hex[HMAC_HEX_LEN + 1];
        for (size_t i = 0; i < HMAC_HEX_LEN / 2; i++) {
            snprintf(hex + i * 2, 3, "%02x", fullHmac[i]);
        }
        hex[HMAC_HEX_LEN] = '\0';
        return String(hex);
    }
}

String signCommand(const String& baseMessage, Storage::DeviceConfig& cfg) {
    cfg.cmdCounter++;
    Storage::save(cfg);

    String withCounter = baseMessage + " " + String(cfg.cmdCounter);
    String hmac = hmacHex(withCounter);
    return withCounter + " " + hmac;
}

}
