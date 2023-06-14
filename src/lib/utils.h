#if !defined(LIB_UTILS_H)
#define LIB_UTILS_H

#include <sstream>
#include <vector>
#include <ArduinoJson.h>

String jsonEncode(const DynamicJsonDocument &jsonDoc);

std::vector<std::string> splitString(
        const std::string &str, const std::string &delimiter, bool includeDelimiter = false);

std::vector<std::string> splitString(
        const std::string &str, const std::vector<std::string> &delimiters, bool includeDelimiter = false);

std::vector<std::string> splitLines(const std::string &str);

std::vector<std::string> splitSentence(const std::string &str);

#endif // !defined(LIB_UTILS_H)
