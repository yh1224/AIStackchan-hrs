#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "app/AppChat.h"
#include "app/AppFace.h"
#include "app/AppVoice.h"
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

void AppChat::toggleRandomSpeakMode() {
    xSemaphoreTake(_lock, portMAX_DELAY);
    String message;
    if (!_randomSpeakMode) {
        _randomSpeakMode = true;
        _randomSpeakNextTime = _getRandomSpeakNextTime();
        message = String(t(_settings->getLang().c_str(), "chat_random_started"));
    } else {
        _randomSpeakMode = false;
        message = String(t(_settings->getLang().c_str(), "chat_random_stopped"));
    }
    xSemaphoreGive(_lock);
    _voice->stopSpeak();
    _voice->speak(message, "");
    _setFace(Expression::Happy, "", 3000);
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
            format = t(_settings->getLang().c_str(), "clock_now_noon");
        } else {
            format = t(_settings->getLang().c_str(), "clock_now");
        }
        char messageBuf[strlen(format) + 1];
        snprintf(messageBuf, sizeof(messageBuf), format, tm.tm_hour, tm.tm_min);
        message = String(messageBuf);
    } else {
        t(_settings->getLang().c_str(), "clock_not_set");
    }
    _voice->stopSpeak();
    _voice->speak(message, "");
}

/**
 * Ask to the ChatGPT and get answer
 *
 * @param text question
 * @param useHistory use chat history
 */
void AppChat::talk(const String &text, const String &voiceName, bool useHistory,
                   const std::function<void(const char *)> &onReceiveAnswer) {
    xSemaphoreTake(_lock, portMAX_DELAY);
    _chatRequests.push_back(std::make_unique<ChatRequest>(text, voiceName, onReceiveAnswer));
    xSemaphoreGive(_lock);
}

unsigned long AppChat::_getRandomSpeakNextTime() {
    auto interval = _settings->getChatRandomInterval();
    int min = interval.first;
    int max = interval.second;
    return millis() + ((unsigned long) random(min, max)) * 1000;
}

String AppChat::_getRandomSpeakQuestion() {
    auto questions = _settings->getChatRandomQuestions();
    return questions[(int) random((int) questions.size())];
}

bool AppChat::_isRandomSpeakTimeNow(unsigned long now) {
    if (now > _randomSpeakNextTime) {
        _randomSpeakNextTime = _getRandomSpeakNextTime();
        return true;
    }
    return false;
}

bool AppChat::_isClockSpeakTimeNow() {
    struct tm tm{};
    if (getLocalTime(&tm) && tm.tm_min == 0 && tm.tm_sec == 0) {
        auto hours = _settings->getChatClockHours();
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
void AppChat::_setFace(Expression expression, const String &text, int duration) {
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
    auto apiKey = _settings->getOpenAiApiKey();
    if (apiKey == nullptr) {
        String message = t(_settings->getLang().c_str(), "apikey_not_set");
        _voice->speak(message, voiceName);
        return message;
    }

    ChatGptClient client{apiKey, _settings->getChatGptModel()};
    _setFace(Expression::Doubt, t(_settings->getLang().c_str(), "chat_thinking..."));

    // call ChatGPT
    try {
        std::deque<String> noHistory;
        String response;
        if (_settings->useChatGptStream()) {
            std::stringstream ss;
            int index = 0;
            response = client.chat(
                    text, _settings->getChatRoles(), useHistory ? _chatHistory : noHistory,
                    [&](const String &body) {
                        //Serial.printf("%s", body.c_str());
                        ss << body.c_str();
                        auto sentences = splitSentence(ss.str());
                        if (sentences.size() > (index + 1)) {
                            _setFace(Expression::Neutral, "");
                            for (int i = index; i < sentences.size() - 1; i++) {
                                _voice->speak(sentences[i].c_str(), voiceName);
                                index++;
                            }
                        }
                    });
            _setFace(Expression::Neutral, "");
            auto sentences = splitSentence(response.c_str());
            for (int i = index; i < sentences.size(); i++) {
                _voice->speak(sentences[i].c_str(), voiceName);
            }
        } else {
            response = client.chat(text, _settings->getChatRoles(), useHistory ? _chatHistory : noHistory, nullptr);
            //Serial.printf("%s\n", response.c_str());
            _setFace(Expression::Neutral, "");
            _voice->speak(response, voiceName);
        }

        if (useHistory) {
            // チャット履歴が最大数を超えた場合、古い質問と回答を削除
            if (_chatHistory.size() > _settings->getMaxHistory() * 2) {
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
        _setFace(Expression::Sad, errorMessage, 3000);
        _voice->speak(t(_settings->getLang().c_str(), "chat_i_dont_understand"), voiceName);
        return errorMessage;
    }
}

void AppChat::_loop() {
    auto now = millis();

    // reset balloon and face
    if (_hideBalloon >= 0 && now > _hideBalloon) {
        _face->setExpression(Expression::Neutral);
        _face->setText("");
        _hideBalloon = -1;
    }

    if (!_voice->isPlaying()) {
        if (_settings->isClockSpeakEnabled() && _isClockSpeakTimeNow()) {
            // clock speak mode
            speakCurrentTime();
        } else if (_randomSpeakMode && _isRandomSpeakTimeNow(now)) {
            // random speak mode
            _talk(_getRandomSpeakQuestion(), "", false);
        } else {
            xSemaphoreTake(_lock, portMAX_DELAY);
            std::unique_ptr<ChatRequest> request = nullptr;
            if (!_chatRequests.empty()) {
                request = std::move(_chatRequests.front());
                _chatRequests.pop_front();
            }
            xSemaphoreGive(_lock);
            if (request != nullptr) {
                // handle request
                auto answer = _talk(request->text, request->voice, true);
                request->onReceiveAnswer(answer.c_str());
            }
        }
    }
    delay(500);
}
