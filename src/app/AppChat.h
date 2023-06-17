#if !defined(APP_CHAT_H)
#define APP_CHAT_H

#include <deque>
#include <utility>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "app/AppFace.h"
#include "app/AppSettings.h"
#include "app/AppVoice.h"

class AppChat {
public:
    explicit AppChat(
            std::shared_ptr<AppSettings> settings,
            std::shared_ptr<AppVoice> voice,
            std::shared_ptr<AppFace> face
    ) : _settings(std::move(settings)),
        _voice(std::move(voice)),
        _face(std::move(face)) {};

    void setup();

    void start();

    void toggleRandomSpeakMode();

    void speakCurrentTime();

    String talk(const String &text, String &voiceName) {
        return talk(text, voiceName, false);
    }

    String talk(const String &text, const String &voiceName, bool useHistory);

private:
    std::shared_ptr<AppSettings> _settings;
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

    unsigned long _getRandomSpeakNextTime();

    String _getRandomSpeakQuestion();

    bool _isRandomSpeakTimeNow(unsigned long now);

    bool _isClockSpeakTimeNow();

    void _setFace(Expression expression, const char *text) {
        _setFace(expression, text, -1);
    }

    void _setFace(Expression expression, const String &text, int duration);

    String _talk(const String &text, const String &voiceName, bool useHistory);

    void _loop();
};

#endif // !defined(APP_CHAT_H)
