#include <Arduino.h>
#include <SD.h>

/**
 * Load string from file on SD card
 *
 * @param path file path
 * @return string (nullptr: failed)
 */
std::unique_ptr<String> sdLoadString(const char *path) {
    std::unique_ptr<String> value = nullptr;
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("ERROR: Failed to begin SD");
    } else {
        auto fs = SD.open(path, FILE_READ);
        if (!fs) {
            Serial.printf("ERROR: Failed to open SD for reading (path=%s)\n", path);
        } else {
            auto tmpValue = fs.readString();
            Serial.printf("SD/Loaded: %s (%d bytes)\n", path, tmpValue.length());
            value = std::unique_ptr<String>(new String(tmpValue));
            fs.close();
        }
        SD.end();
    }
    return value;
}
