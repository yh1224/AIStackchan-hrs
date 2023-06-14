#if !defined(LIB_NVS_SETTINGS_H)
#define LIB_NVS_SETTINGS_H

#include <vector>
#include <ArduinoJson.h>

/// 設定 JSON サイズ
static const size_t SETTINGS_MAX_SIZE = 4 * 1024;

class NvsSettings {
public:
    explicit NvsSettings(String nvsNamespace, String nvsKey);

    bool load();

    bool save();

    bool load(const String &text, bool merge = false);

    bool has(const String &keyStr);

    JsonVariant get(const String &keyStr);

    template<class T>
    bool set(const String &keyStr, const T &value);

    bool remove(const String &keyStr);

    size_t count(const String &keyStr);

    template<class T>
    std::vector<T> getArray(const String &keyStr);

    template<class T>
    bool add(const String &keyStr, const T &value);

    bool clear(const String &keyStr);

protected:
    String _nvsNamespace;
    String _nvsKey;

    DynamicJsonDocument _settings{SETTINGS_MAX_SIZE};

    JsonVariant _get(std::vector<std::string> &keys);

    JsonVariant _getParentOrCreate(std::vector<std::string> &keys);
};

#endif // !defined(LIB_NVS_SETTINGS_H)
