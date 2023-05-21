#if !defined(LIB_SPIFFS_H)
#define LIB_SPIFFS_H

#include <memory>
#include <Arduino.h>

bool spiffsSaveString(const char *path, const String &value);

std::unique_ptr<String> spiffsLoadString(const char *path);

#endif // !defined(LIB_SPIFFS_H)
