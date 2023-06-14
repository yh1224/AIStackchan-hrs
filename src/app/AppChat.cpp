#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "app/AppChat.h"
#include "app/AppFace.h"
#include "app/AppVoice.h"
#include "app/config.h"
#include "app/lang.h"
#include "lib/ChatGptClient.h"
#include "lib/utils.h"

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
        message = String(t(_getLang().c_str(), "chat_random_started"));
    } else {
        _randomSpeakMode = false;
        message = String(t(_getLang().c_str(), "chat_random_stopped"));
    }
    xSemaphoreGive(_lock);
    _voice->stopSpeak();
    _voice->speak(message, "");
    _setFace(m5avatar::Expression::Happy, "", 3000);
}

/**
 * Speak current time
 */
void AppChat::speakCurrentTime() {
    String message;
    struct tm tm{};
    if (getLocalTime(&tm)) {
        const char *format;
        if (tm.tm_min == 0) {
            format = t(_getLang().c_str(), "clock_now_noon");
        } else {
            format = t(_getLang().c_str(), "clock_now");
        }
        char messageBuf[strlen(format) + 1];
        snprintf(messageBuf, sizeof(messageBuf), format, tm.tm_hour, tm.tm_min);
        message = String(messageBuf);
    } else {
        t(_getLang().c_str(), "clock_not_set");
    }
    _voice->stopSpeak();
    _voice->speak(message, "");
}

/**
 * Ask to the ChatGPT and get answer
 *
 * @param text question
 * @param useHistory use chat history
 * @return answer
 */
String AppChat::talk(const String &text, const String &voiceName, bool useHistory) {
    xSemaphoreTake(_lock, portMAX_DELAY);
    auto answer = _talk(text, voiceName, useHistory);
    xSemaphoreGive(_lock);
    return answer;
}

String AppChat::_getLang() {
    String lang = _settings->get(CONFIG_VOICE_LANG_KEY) | CONFIG_VOICE_LANG_DEFAULT;
    return lang.substring(0, 2); // en-US -> en
}

const char *AppChat::_getOpenAiApiKey() {
    return _settings->get(CONFIG_CHAT_OPENAI_APIKEY_KEY);
}

const char *AppChat::_getChatGptModel() {
    return _settings->get(CONFIG_CHAT_OPENAI_CHATGPT_MODEL_KEY) | CONFIG_CHAT_OPENAI_CHATGPT_MODEL_DEFAULT;
}

bool AppChat::_useStream() {
    return _settings->get(CONFIG_CHAT_OPENAI_STREAM_KEY) | CONFIG_CHAT_OPENAI_STREAM_DEFAULT;
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
String AppChat::_talk(const String &text, const String &voiceName, bool useHistory) {
    auto apiKey = _getOpenAiApiKey();
    if (apiKey == nullptr) {
        String message = t(_getLang().c_str(), "apikey_not_set");
        _voice->speak(message, voiceName);
        return message;
    }

    ChatGptClient client{apiKey, _getChatGptModel()};
    _setFace(m5avatar::Expression::Doubt, t(_getLang().c_str(), "chat_thinking..."));

    // call ChatGPT
    try {
        std::deque<String> noHistory;
        String response;
        if (_useStream()) {
            std::stringstream ss;
            int index = 0;
            response = client.chat(
                    text, getChatRoles(), useHistory ? _chatHistory : noHistory,
                    [&](const String &body) {
                        //Serial.printf("%s", body.c_str());
                        ss << body.c_str();
                        auto sentences = splitSentence(ss.str());
                        if (sentences.size() > (index + 1)) {
                            _setFace(m5avatar::Expression::Neutral, "");
                            for (int i = index; i < sentences.size() - 1; i++) {
                                _voice->speak(sentences[i].c_str(), voiceName);
                                index++;
                            }
                        }
                    });
            _setFace(m5avatar::Expression::Neutral, "");
            auto sentences = splitSentence(response.c_str());
            for (int i = index; i < sentences.size(); i++) {
                _voice->speak(sentences[i].c_str(), voiceName);
            }
        } else {
            response = client.chat(text, getChatRoles(), useHistory ? _chatHistory : noHistory, nullptr);
            //Serial.printf("%s\n", response.c_str());
            _setFace(m5avatar::Expression::Neutral, "");
            _voice->speak(response, voiceName);
        }

        if (useHistory) {
            // チャット履歴が最大数を超えた場合、古い質問と回答を削除
            if (_chatHistory.size() > _getMaxHistory() * 2) {
                _chatHistory.pop_front();
                _chatHistory.pop_front();
            }
            // 質問と回答をチャット履歴に追加
            _chatHistory.push_back(text);
            _chatHistory.push_back(response);
        }
        return response;
    } catch (ChatGptClientError &e) {
        Serial.printf("ERROR: %s\n", e.what());
        String errorMessage;
        try {
            throw;
        } catch (ChatGptHttpError &e) {
            errorMessage = "Error: " + String(e.statusCode());
        } catch (std::exception &e) {
            errorMessage = "Error";
        }
        _setFace(m5avatar::Expression::Sad, errorMessage, 3000);
        _voice->speak(t(_getLang().c_str(), "chat_i_dont_understand"), voiceName);
        return errorMessage;
    }
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
            _talk(_getRandomSpeakQuestion(), "", false);
        }
        xSemaphoreGive(_lock);
    }
    delay(500);
}
