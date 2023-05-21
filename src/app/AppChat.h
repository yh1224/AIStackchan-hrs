#if !defined(APP_CHAT_H)
#define APP_CHAT_H

#include <deque>
#include <utility>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "app/AppFace.h"
#include "app/AppVoice.h"
#include "lib/NvsSettings.h"

class AppChat {
public:
    explicit AppChat(
            std::shared_ptr<NvsSettings> settings,
            std::shared_ptr<AppVoice> voice,
            std::shared_ptr<AppFace> face
    ) : _settings(std::move(settings)),
        _voice(std::move(voice)),
        _face(std::move(face)) {};

    void setup();

    void start();

    std::vector<String> getChatRoles();

    bool setOpenAiApiKey(const String &apiKey);

    bool addRole(const String &role);

    bool clearRoles();

    void toggleRandomSpeakMode();

    void speakCurrentTime();

    String talk(const String &text) {
        return talk(text, false);
    }

    String talk(const String &text, bool useHistory);

private:
    std::shared_ptr<NvsSettings> _settings;
    std::shared_ptr<AppVoice> _voice;
    std::shared_ptr<AppFace> _face;

    TaskHandle_t _taskHandle;

    SemaphoreHandle_t _lock = xSemaphoreCreateMutex();

    /// text to display in the balloon
    String _balloonText = "";

    /// time to hide text in the balloon
    unsigned long _hideBalloon = -1;

    /// random speak mode
    bool _randomSpeakMode = false;

    /// random speak mode: next speak time
    unsigned long _randomSpeakNextTime = 0;

    /// chat history (questions and answers)
    std::deque<String> _chatHistory;

    String _getLang();

    const char *_getOpenAiApiKey();

    int _getMaxHistory();

    unsigned long _getRandomSpeakNextTime();

    String _getRandomSpeakQuestion();

    bool _isRandomSpeakEnabled();

    bool _isRandomSpeakTimeNow(unsigned long now);

    bool _isClockSpeakEnabled();

    bool _isClockSpeakTimeNow();

    void _setFace(m5avatar::Expression expression, const char *text) {
        _setFace(expression, text, -1);
    }

    void _setFace(m5avatar::Expression expression, const String &text, int duration);

    String _talk(const String &text, bool useHistory);

    void _loop();
};

#endif // !defined(APP_CHAT_H)
