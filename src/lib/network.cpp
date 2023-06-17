#include <iomanip>
#include <M5Unified.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "lib/utils.h"

/**
 * Connect to network
 *
 * @param ssid Wi-Fi SSID
 * @param passphrase Wi-Fi passphrase
 * @return true: success, false: failure
 */
bool connectNetwork(const char *ssid, const char *passphrase) {
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFiClass::mode(WIFI_STA);

    bool manualConfig = ssid != nullptr && passphrase != nullptr;
    if (manualConfig) {
        M5.Display.printf("SSID: %s\n", ssid);
        Serial.printf("SSID: %s\n", ssid);
        WiFi.begin(ssid, passphrase);
    } else {
        WiFi.begin();
    }
    M5.Display.print("Connecting");
    Serial.print("Connecting");
    while (WiFiClass::status() != WL_CONNECTED) {
        M5.Display.print(".");
        Serial.print(".");
        delay(500);
        // Give up in 10 seconds
        if (10000 < millis()) {
            break;
        }
    }
    M5.Display.println("");
    Serial.println("");

    if (WiFiClass::status() != WL_CONNECTED) {
        if (manualConfig) {
            return false;
        }

        // Try autoconfiguration by SmartConfig
        WiFiClass::mode(WIFI_STA);
        WiFi.beginSmartConfig();
        M5.Display.printf("SmartConfig: ");
        Serial.printf("SmartConfig: ");
        while (!WiFi.smartConfigDone()) {
            delay(500);
            M5.Display.print("#");
            Serial.print("#");
            // Give up in 30 seconds
            if (30000 < millis()) {
                M5.Display.println("");
                Serial.println("");
                return false;
            }
        }
        M5.Display.println("");
        Serial.println("");

        // Wait for connection
        M5.Display.println("Connecting");
        Serial.println("Connecting");
        while (WiFiClass::status() != WL_CONNECTED) {
            delay(500);
            M5.Display.print(".");
            Serial.print(".");
            // Give up in 60 seconds.
            if (60000 < millis()) {
                M5.Display.println("");
                Serial.println("");
                return false;
            }
        }
    }
    M5.Display.printf("Connected\nIP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Connected\nIP: %s\n", WiFi.localIP().toString().c_str());

    delay(3000);
    return true;
}

/**
 * Setup mDNS host
 *
 * @param hostname host name
 */
void setMDnsHostname(const char *hostname) {
    if (MDNS.begin(hostname)) {
        M5.Display.printf("hostname: %s\n", hostname);
    }
}

/**
 * Synchronize clock
 *
 * @param tz time zone
 * @param ntpServer NTP server
 */
void syncTime(const char *tz, const char *ntpServer) {
    M5.Display.printf("Synchronizing time:");
    configTzTime(tz, ntpServer);

    struct tm now{};
    if (!getLocalTime(&now)) {
        M5.Display.println(" failed");
    } else {
        std::stringstream ss;
        ss << std::put_time(&now, "%Y-%m-%d %H:%M:%S");
        M5.Display.println("");
        M5.Display.println(ss.str().c_str());
    }
}
