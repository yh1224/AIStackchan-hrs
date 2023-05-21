#include <memory>
#include <utility>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "lib/NvsSettings.h"
#include "lib/nvs.h"
#include "lib/utils.h"

NvsSettings::NvsSettings(String nvsNamespace, String nvsKey)
        : _nvsNamespace(std::move(nvsNamespace)), _nvsKey(std::move(nvsKey)) {
}

/**
 * Load from NVS
 *
 * @return true: success, false: failure
 */
bool NvsSettings::load() {
    auto settings = nvsLoadString(_nvsNamespace, _nvsKey, SETTINGS_MAX_SIZE);
    if (settings != nullptr) {
        return deserializeJson(_settings, settings->c_str()) == DeserializationError::Ok;
    }
    return false;
}

/**
 * Save to NVS
 *
 * @return true: success, false: failure
 */
bool NvsSettings::save() {
    String settings;
    serializeJson(_settings, settings);
    return nvsSaveString(_nvsNamespace, _nvsKey, settings.c_str());
}

/**
 * Merge json objects
 * https://arduinojson.org/v6/how-to/merge-json-objects/
 */
static void mergeJsonObjects(JsonVariant dst, JsonVariantConst src) {
    if (src.is<JsonObjectConst>()) {
        for (JsonPairConst kvp: src.as<JsonObjectConst>()) {
            if (dst[kvp.key()]) {
                mergeJsonObjects(dst[kvp.key()], kvp.value());
            } else {
                dst[kvp.key()] = kvp.value();
            }
        }
    } else {
        dst.set(src);
    }
}

/**
 * Import from json string (and save to NVS)
 *
 * @param text json string
 * @param merge true: merge, false: overwrite
 * @return true: success, false: failure
 */
bool NvsSettings::load(const String &text, bool merge) {
    DynamicJsonDocument tmp{SETTINGS_MAX_SIZE};
    bool result = deserializeJson(tmp, text) == DeserializationError::Ok;
    if (result) {
        if (merge) {
            mergeJsonObjects(_settings, tmp);
        } else {
            _settings = tmp;
        }
    }
    return result && save();
}

/**
 * Check if the key exists
 *
 * @param keyStr key string delimited by "."
 * @return true: exists, false: not exists
 */
bool NvsSettings::has(const String &keyStr) {
    auto keys = splitString(keyStr.c_str(), ".");
    return !_get(keys).isNull();
}

/**
 * Get the value
 *
 * @param keyStr key string delimited by "."
 * @return value (can be cast to any type)
 */
JsonVariant NvsSettings::get(const String &keyStr) {
    auto keys = splitString(keyStr.c_str(), ".");
    return _get(keys);
}

/**
 * Set the value
 *
 * @param keyStr key string delimited by "."
 * @param value (any type)
 * @return true: success, false: failure
 */
template<class T>
bool NvsSettings::set(const String &keyStr, const T &value) {
    auto keys = splitString(keyStr.c_str(), ".");
    auto key = keys[keys.size() - 1];
    if (std::all_of(key.begin(), key.end(), ::isdigit)) {
        _getParentOrCreate(keys)[std::stoi(key)].set(value);
    } else {
        _getParentOrCreate(keys)[key].set(value);
    }
    return save();
}

template bool NvsSettings::set<std::string>(const String &key, const std::string &value);

template bool NvsSettings::set<String>(const String &key, const String &value);

template bool NvsSettings::set<int>(const String &key, const int &value);

template bool NvsSettings::set<bool>(const String &key, const bool &value);

/**
 * Remove the value
 *
 * @param keyStr key string delimited by "."
 * @return true: success, false: failure
 */
bool NvsSettings::remove(const String &keyStr) {
    auto keys = splitString(keyStr.c_str(), ".");
    auto key = keys[keys.size() - 1];
    if (std::all_of(key.begin(), key.end(), ::isdigit)) {
        _getParentOrCreate(keys).remove(std::stoi(key));
    } else {
        _getParentOrCreate(keys).remove(key);
    }
    return save();
}

/**
 * Count the number of elements (for array only)
 *
 * @param keyStr key string delimited by "."
 * @return number of elements
 */
size_t NvsSettings::count(const String &keyStr) {
    auto keys = splitString(keyStr.c_str(), ".");
    return _get(keys).size();
}

/**
 * Get all elements (for array only)
 *
 * @param keyStr key string delimited by "."
 * @return elements (can be cast to any type)
 */
template<class T>
std::vector<T> NvsSettings::getArray(const String &keyStr) {
    auto keys = splitString(keyStr.c_str(), ".");
    JsonArray arr = _get(keys);
    std::vector<T> values;
    for (const auto &e: arr) {
        values.push_back(e.as<T>());
    }
    return values;
}

template std::vector<std::string> NvsSettings::getArray(const String &keyStr);

template std::vector<String> NvsSettings::getArray(const String &keyStr);

template std::vector<int> NvsSettings::getArray(const String &keyStr);

template std::vector<bool> NvsSettings::getArray(const String &keyStr);

/**
 * Add element (for array only)
 *
 * @param keyStr key string delimited by "."
 * @param value element (any type)
 * @return true: success, false: failure
 */
template<class T>
bool NvsSettings::add(const String &keyStr, const T &value) {
    auto keys = splitString(keyStr.c_str(), ".");
    auto key = keys[keys.size() - 1];
    auto val = _getParentOrCreate(keys);
    if (val[key].isNull()) {
        val = val.createNestedArray(key);
    } else {
        val = val[key];
    }
    val.add(value);
    return save();
}

template bool NvsSettings::add<std::string>(const String &keyStr, const std::string &value);

template bool NvsSettings::add<String>(const String &keyStr, const String &value);

template bool NvsSettings::add<int>(const String &keyStr, const int &value);

template bool NvsSettings::add<bool>(const String &keyStr, const bool &value);

/**
 * Clear elements (for array only)
 *
 * @param keyStr key string delimited by "."
 * @return true: success, false: failure
 */
bool NvsSettings::clear(const String &keyStr) {
    auto keys = splitString(keyStr.c_str(), ".");
    auto key = keys[keys.size() - 1];
    auto val = _getParentOrCreate(keys);
    if (val[key].isNull()) {
        val = val.createNestedArray(key);
    } else {
        val = val[key];
    }
    val.clear();
    return save();
}

/**
 * Get the json element from key list
 *
 * @param keys key list
 * @return json element
 */
JsonVariant NvsSettings::_get(std::vector<std::string> &keys) {
    JsonVariant val = _settings;
    for (const auto &key: keys) {
        if (std::all_of(key.begin(), key.end(), ::isdigit)) {
            val = val[std::stoi(key)];
        } else {
            val = val[key];
        }
    }
    return val;
}

/**
 * Get the parent json element (create if not exists)
 *
 * @param keys key list
 * @return json element
 */
JsonVariant NvsSettings::_getParentOrCreate(std::vector<std::string> &keys) {
    JsonVariant val = _settings;
    for (int i = 0; i < keys.size() - 1; i++) {
        auto key = keys[i];
        auto nextKey = keys[i + 1];
        if (std::all_of(key.begin(), key.end(), ::isdigit)) {
            int keyNum = std::stoi(key);
            // val is array
            if (val[keyNum].isNull()) {
                if (std::all_of(nextKey.begin(), nextKey.end(), ::isdigit)) {
                    // create array when the next key is number
                    val = val.createNestedArray(keyNum);
                } else {
                    val = val.createNestedObject(keyNum);
                }
            } else {
                val = val[keyNum];
            }
        } else {
            // val is object
            if (val[key].isNull()) {
                if (std::all_of(nextKey.begin(), nextKey.end(), ::isdigit)) {
                    // create array when the next key is number
                    val = val.createNestedArray(key);
                } else {
                    val = val.createNestedObject(key);
                }
            } else {
                val = val[key];
            }
        }
    }
    return val;
}
