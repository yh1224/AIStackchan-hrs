#include <iostream>
#include <fstream>
#include <Arduino.h>
#include <SPIFFS.h>

#include "lib/spiffs.h"

/**
 * Save string to SPIFFS
 *
 * @param path file path
 * @param value string
 * @return true: success, false: failure
 */
bool spiffsSaveString(const char *path, const String &value) {
    bool result = false;
    if (!SPIFFS.begin(true)) {
        Serial.println("ERROR: Failed to begin SPIFFS");
    } else {
        File f = SPIFFS.open(path, "w");
        if (!f) {
            Serial.printf("ERROR: Failed to open SPIFFS for writing (path=%s)\n", path);
        } else {
            f.write((u_int8_t *) value.c_str(), value.length());
            Serial.printf("SPIFFS/Saved: %s=%s\n", path, value.c_str());
            result = true;
            f.close();
        }
        SPIFFS.end();
    }
    return result;
}

/**
 * Load string from SPIFFS
 *
 * @param path file path
 * @return string (nullptr: failed)
 */
std::unique_ptr<String> spiffsLoadString(const char *path) {
    std::unique_ptr<String> value = nullptr;
    if (!SPIFFS.begin(true)) {
        Serial.println("ERROR: Failed to begin SPIFFS");
    } else {
        File f = SPIFFS.open(path, "r");
        if (!f || f.size() == 0) {
            Serial.printf("ERROR: Failed to open SPIFFS for reading (path=%s)\n", path);
        } else {
            auto tmpValue = f.readString();
            Serial.printf("SPIFFS/Loaded: %s (%d bytes)\n", path, tmpValue.length());
            value = std::make_unique<String>(tmpValue);
            f.close();
        }
        SPIFFS.end();
    }
    return value;
}
