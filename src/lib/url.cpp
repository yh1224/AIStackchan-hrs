#include <iomanip>
#include <sstream>
#include <unordered_map>

#include "url.h"
#include "utils.h"

std::string urlEncode(const char *msg) {
    std::stringstream ss;
    for (const char *p = msg; *p != '\0'; p++) {
        if (('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z') || ('0' <= *p && *p <= '9')
            || *p == '-' || *p == '_' || *p == '.' || *p == '~') {
            ss << *p;
        } else if (*p == ' ') {
            ss << '+';
        } else {
            ss << '%' << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int) *p;
        }
    }
    return ss.str();
}

std::string urlDecode(const char *msg) {
    std::stringstream ss;
    for (const char *p = msg; *p != '\0'; p++) {
        if (*p == '%') {
            int c1 = toupper(*(p + 1));
            int c2 = toupper(*(p + 2));
            if (c1 >= '0' && c1 <= '9') { c1 -= '0'; } else if (c1 >= 'A' && c1 <= 'F') { c1 -= 'A' - 10; }
            if (c2 >= '0' && c2 <= '9') { c2 -= '0'; } else if (c2 >= 'A' && c2 <= 'F') { c2 -= 'A' - 10; }
            ss << (char) (((c1 << 4) + c2));
            p += 2;
        } else if (*p == '+') {
            ss << " ";
        } else {
            ss << *p;
        }
    }
    return ss.str();
}

std::string qsBuild(const UrlParams &query) {
    std::stringstream ss;
    int n = 0;
    for (const auto &item: query) {
        if (n > 0) ss << "&";
        n++;
        ss << item.first << "=" << urlEncode(item.second.c_str());
    }
    return ss.str();
}

UrlParams qsParse(const char *query) {
    UrlParams result;
    for (const auto &item: splitString(query, "&")) {
        auto s = splitString(item, "=");
        result[s[0]] = urlDecode(s[1].c_str());
    }
    return result;
}
