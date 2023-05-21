#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "app/AppChat.h"
#include "app/AppFace.h"
#include "app/AppVoice.h"
#include "app/config.h"
#include "lib/ChatGptClient.h"
#include "lib/utils.h"

/// ChatGPT: Use Event Stream
#define CHATGPT_EVENT_STREAM

void AppChat::setup() {
}

void AppChat::start() {
    xTaskCreatePinnedToCore(
            [](void *arg) {
                auto *self = (AppChat *) arg;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
                while (true) {
                    self->_loop();
                }
#pragma clang diagnostic pop
            },
            "AppChat",
            8192,
            this,
            1,
            &_taskHandle,
            APP_CPU_NUM
    );
}

std::vector<String> AppChat::getChatRoles() {
    return _settings->getArray<String>(CONFIG_CHAT_OPENAI_ROLES_KEY);
}

bool AppChat::setOpenAiApiKey(const String &apiKey) {
    if (apiKey == "") {
        return _settings->remove(CONFIG_CHAT_OPENAI_APIKEY_KEY);
    } else {
        return _settings->set(CONFIG_CHAT_OPENAI_APIKEY_KEY, apiKey);
    }
}

bool AppChat::addRole(const String &role) {
    return _settings->add(CONFIG_CHAT_OPENAI_ROLES_KEY, role);
}

bool AppChat::clearRoles() {
    return _settings->clear(CONFIG_CHAT_OPENAI_ROLES_KEY);
}

void AppChat::toggleRandomSpeakMode() {
    xSemaphoreTake(_lock, portMAX_DELAY);
    String message;
    if (!_randomSpeakMode) {
        _randomSpeakMode = true;
        _randomSpeakNextTime = _getRandomSpeakNextTime();
        if (_getLang() == "ja_JP") {
            message = "ひとりごと始めます。";
        } else {
            message = "random speak mode started.";
        }
    } else {
        _randomSpeakMode = false;
        if (_getLang() == "ja_JP") {
            message = "ひとりごとやめます。";
        } else {
            message = "random speak mode stopped.";
        }
    }
    xSemaphoreGive(_lock);
    _voice->stopSpeak();
    _voice->speak(message);
    _setFace(m5avatar::Expression::Happy, "", 3000);
}

/**
 * Speak current time
 */
void AppChat::speakCurrentTime() {
    String message;
    struct tm tm{};
    if (getLocalTime(&tm)) {
        if (tm.tm_min == 0) {
            if (_getLang() == "ja_JP") {
                message = String(tm.tm_hour) + "時 ちょうどです。";
            } else {
                message = String(tm.tm_hour) + " o'clock";
            }
        } else {
            if (_getLang() == "ja_JP") {
                message = String(tm.tm_hour) + "時" + String(tm.tm_min) + "分です。";
            } else {
                message = String(tm.tm_hour) + " " + String(tm.tm_min);
            }
        }
    } else {
        if (_getLang() == "ja_JP") {
            message = "時刻が設定されていません。";
        } else {
            message = "The clock is not set.";
        }
    }
    _voice->stopSpeak();
    _voice->speak(message);
}

/**
 * Ask to the ChatGPT and get answer
 *
 * @param text question
 * @param useHistory use chat history
 * @return answer
 */
String AppChat::talk(const String &text, bool useHistory) {
    xSemaphoreTake(_lock, portMAX_DELAY);
    auto answer = _talk(text, useHistory);
    xSemaphoreGive(_lock);
    return answer;
}

String AppChat::_getLang() {
    return _settings->get(CONFIG_VOICE_LANG_KEY) | CONFIG_VOICE_LANG_DEFAULT;
}

const char *AppChat::_getOpenAiApiKey() {
    return _settings->get(CONFIG_CHAT_OPENAI_APIKEY_KEY);
}

int AppChat::_getMaxHistory() {
    return _settings->get(CONFIG_CHAT_OPENAI_MAX_HISTORY_KEY) | CONFIG_CHAT_OPENAI_MAX_HISTORY_DEFAULT;
}

unsigned long AppChat::_getRandomSpeakNextTime() {
    int min = _settings->get(CONFIG_CHAT_RANDOM_INTERVAL_MIN_KEY) | CONFIG_CHAT_RANDOM_INTERVAL_MIN_DEFAULT;
    int max = _settings->get(CONFIG_CHAT_RANDOM_INTERVAL_MAX_KEY) | CONFIG_CHAT_RANDOM_INTERVAL_MAX_DEFAULT;
    return millis() + ((unsigned long) random(min, max)) * 1000;
}

String AppChat::_getRandomSpeakQuestion() {
    auto questions = _settings->getArray<String>(CONFIG_CHAT_RANDOM_QUESTIONS_KEY);
    return questions[(int) random((int) questions.size())];
}

bool AppChat::_isRandomSpeakEnabled() {
    return _settings->has(CONFIG_CHAT_RANDOM_QUESTIONS_KEY);
}

bool AppChat::_isRandomSpeakTimeNow(unsigned long now) {
    if (now > _randomSpeakNextTime) {
        _randomSpeakNextTime = _getRandomSpeakNextTime();
        return true;
    }
    return false;
}

bool AppChat::_isClockSpeakEnabled() {
    return _settings->has(CONFIG_CHAT_CLOCK_HOURS_KEY);
}

bool AppChat::_isClockSpeakTimeNow() {
    struct tm tm{};
    if (getLocalTime(&tm) && tm.tm_min == 0 && tm.tm_sec == 0) {
        auto hours = _settings->getArray<int>(CONFIG_CHAT_CLOCK_HOURS_KEY);
        for (auto hour: hours) {
            if (hour == tm.tm_hour) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Set face
 *
 * @param expression expression
 * @param text text
 * @param duration delay to hide balloon (0: no hide)
 */
void AppChat::_setFace(m5avatar::Expression expression, const String &text, int duration) {
    _face->setExpression(expression);
    _face->setText("");
    _balloonText = text;
    _face->setText(_balloonText.c_str());
    _hideBalloon = (duration > 0) ? millis() + duration : -1;
}

/**
 * Ask to the ChatGPT and get answer
 *
 * @param text question
 * @param useHistory use chat history
 * @return answer (nullptr: error)
 */
String AppChat::_talk(const String &text, bool useHistory) {
    auto apiKey = _getOpenAiApiKey();
    if (apiKey == nullptr) {
        String message;
        if (_getLang() == "ja_JP") {
            message = "API キーが設定されていません";
        } else {
            message = "API Key is not set.";
        }
        _voice->speak(message);
        return message;
    }

    ChatGptClient client{apiKey};
    _setFace(m5avatar::Expression::Doubt, "...");

    // call ChatGPT
    std::deque<String> noHistory;
    std::unique_ptr<String> result = nullptr;
    try {
#if defined(CHATGPT_EVENT_STREAM)
        std::stringstream ss;
        int index = 0;
        auto response = client.chat(
                text, getChatRoles(), useHistory ? _chatHistory : noHistory,
                [&](const String &body) {
                    Serial.printf("%s", body.c_str());
                    ss << body.c_str();
                    auto sentences = splitSentence(ss.str());
                    if (sentences.size() > (index + 1)) {
                        _setFace(m5avatar::Expression::Neutral, "");
                        for (int i = index; i < sentences.size() - 1; i++) {
                            _voice->speak(sentences[i].c_str());
                            index++;
                        }
                    }
                });
        _setFace(m5avatar::Expression::Neutral, "");
        auto sentences = splitSentence(response.c_str());
        for (int i = index; i < sentences.size(); i++) {
            _voice->speak(sentences[i].c_str());
        }
#else
        auto response = client.chat(text, _getRoles(), useHistory ? _chatHistory : noHistory, nullptr);
        Serial.printf("%s\n", response.c_str());
        _voice->speech(response);
#endif // defined(CHATGPT_EVENT_STREAM)
        result = std::unique_ptr<String>(new String(response));

        if (useHistory) {
            // チャット履歴が最大数を超えた場合、古い質問と回答を削除
            if (_chatHistory.size() > _getMaxHistory() * 2) {
                _chatHistory.pop_front();
                _chatHistory.pop_front();
            }
            // 質問と回答をチャット履歴に追加
            _chatHistory.push_back(text);
            _chatHistory.push_back(*result);
        }
    } catch (ChatGptClientError &e) {
        Serial.printf("ERROR: %s\n", e.what());
        try {
            throw;
        } catch (ChatGptHttpError &e) {
            auto errorMessage = "Error: " + String(e.statusCode());
            _setFace(m5avatar::Expression::Sad, errorMessage, 3000);
        } catch (std::exception &e) {
            _setFace(m5avatar::Expression::Sad, "Error", 3000);
        }
        if (_getLang() == "ja_JP") {
            result = std::unique_ptr<String>(new String("わかりません"));
        } else {
            result = std::unique_ptr<String>(new String("I don't understand."));
        }
        _voice->speak(*result);
    }

    return *result;
}

void AppChat::_loop() {
    auto now = millis();

    // reset balloon and face
    if (_hideBalloon >= 0 && now > _hideBalloon) {
        _face->setExpression(m5avatar::Expression::Neutral);
        _face->setText("");
        _hideBalloon = -1;
    }

    if (!_voice->isPlaying() && xSemaphoreTake(_lock, 1) == pdTRUE) {
        if (_isClockSpeakEnabled() && _isClockSpeakTimeNow()) {
            // clock speak mode
            speakCurrentTime();
        } else if (_randomSpeakMode && _isRandomSpeakTimeNow(now)) {
            // random speak mode
            _talk(_getRandomSpeakQuestion(), false);
        }
        xSemaphoreGive(_lock);
    }
    delay(500);
}
