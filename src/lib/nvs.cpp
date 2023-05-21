#include <iostream>
#include <fstream>
#include <Arduino.h>
#include <nvs.h>

#include "lib/nvs.h"

/**
 * Save string to NVS
 *
 * @param key Key
 * @param value string to save
 * @return true: success, false: failure
 */
bool nvsSaveString(const String &name, const String &key, const String &value) {
    bool result = false;
    nvs_handle_t nvsHandle;
    auto openResult = nvs_open(name.c_str(), NVS_READWRITE, &nvsHandle);
    if (openResult != ESP_OK) {
        Serial.printf("ERROR: Failed to open nvs for writing: %s (namespace=%s)\n",
                      esp_err_to_name(openResult), name.c_str());
    } else {
        auto setResult = nvs_set_str(nvsHandle, key.c_str(), value.c_str());
        if (setResult != ESP_OK) {
            Serial.printf("ERROR: Failed to write string to nvs: %s (key=%s)\n", esp_err_to_name(setResult),
                          key.c_str());
        } else {
            Serial.printf("NVS/Saved: %s/%s=%s\n", name.c_str(), key.c_str(), value.c_str());
            result = true;
        }
        nvs_close(nvsHandle);
    }
    return result;
}

/**
 * Load string from NVS
 *
 * @param key Key
 * @return string (nullptr: failed)
 */
std::unique_ptr<String> nvsLoadString(const String &name, const String &key, size_t maxLength) {
    std::unique_ptr<String> value = nullptr;
    nvs_handle_t nvsHandle;
    auto openResult = nvs_open(name.c_str(), NVS_READONLY, &nvsHandle);
    if (openResult != ESP_OK) {
        Serial.printf("ERROR: Failed to open nvs for reading: %s (namespace=%s)\n",
                      esp_err_to_name(openResult), name.c_str());
    } else {
        size_t len = maxLength + 1;
        auto valueBuf = std::unique_ptr<char>((char *) malloc(len));
        auto getResult = nvs_get_str(nvsHandle, key.c_str(), valueBuf.get(), &len);
        if (getResult != ESP_OK) {
            Serial.printf("ERROR: Failed to read string from nvs: %s (key=%s)\n", esp_err_to_name(getResult),
                          key.c_str());
        } else {
            Serial.printf("NVS/Loaded: %s/%s=%s\n", name.c_str(), key.c_str(), valueBuf.get());
            value = std::unique_ptr<String>(new String(valueBuf.get()));
        }
        nvs_close(nvsHandle);
    }
    return value;
}
