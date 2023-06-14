#if !defined(LIB_URL_H)
#define LIB_URL_H

#include <unordered_map>

std::string urlEncode(const char *msg);

std::string urlDecode(const char *msg);

typedef std::unordered_map<std::string, std::string> UrlParams;

std::string qsBuild(const UrlParams &query);

UrlParams qsParse(const char *query);

#endif // !defined(LIB_URL_H)
