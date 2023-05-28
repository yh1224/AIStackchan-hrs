#include <map>

static const std::map<std::string, std::map<std::string, const char *>> clockSpeechMap = {
        {
                "en",
                {
                        {"apikey_not_set", "The API Key is not set"},
                        {"clock_not_set",  "The clock is not set"},
                        {"clock_now", "It's %d %d"},
                        {"clock_now_noon", "It's %d o'clock"},
                        {"chat_thinking...", "Thinking..."},
                        {"chat_i_dont_understand", "I don't understand"},
                        {"chat_random_started", "The random speak mode started."},
                        {"chat_random_stopped", "The random speak mode stopped."},
                }
        },
        {
                "ja",
                {
                        {"apikey_not_set", "API キーが設定されていません"},
                        {"clock_not_set",  "時刻が設定されていません"},
                        {"clock_now", "%d時 %d分です"},
                        {"clock_now_noon", "%d時 ちょうどです"},
                        {"chat_thinking...", "考え中..."},
                        {"chat_i_dont_understand", "わかりません"},
                        {"chat_random_started", "ひとりごと始めます"},
                        {"chat_random_stopped", "ひとりごとやめます"},
                }
        },
        {
                "ro",
                {
                        {"clock_now",      "Este ora %d și %d de minute"},
                        {"clock_now_noon", "Este exact ora %d"},
                }
        },
};

/**
 * Get text from language code
 *
 * @param lang language code
 * @param key text key
 * @return text
 */
const char *t(const char *lang, const char *key) {
    auto defaultLangMap = clockSpeechMap.at("en");
    auto langMap = defaultLangMap;
    if (clockSpeechMap.find(lang) != clockSpeechMap.end()) {
        // matches exactly
        langMap = clockSpeechMap.at(lang);
    } else {
        std::string code(lang, 2);
        if (clockSpeechMap.find(code) != clockSpeechMap.end()) {
            // matches code
            langMap = clockSpeechMap.at(code);
        }
    }
    if (langMap.find(key) != langMap.end()) {
        return langMap.at(key);
    } else if (defaultLangMap.find(key) != defaultLangMap.end()) {
        return defaultLangMap.at(key);
    }
    return key; // unexpected
}
