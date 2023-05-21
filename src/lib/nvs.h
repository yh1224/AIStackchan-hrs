#if !defined(LIB_NVS_H)
#define LIB_NVS_H

#include <memory>
#include <Arduino.h>

bool nvsSaveString(const String &name, const String &key, const String &value);

std::unique_ptr<String> nvsLoadString(const String &name, const String &key, size_t maxLength);

#endif // !defined(LIB_NVS_H)
