#include <Arduino.h>
#include <ESP32WebServer.h>

#include "app/AppChat.h"
#include "app/AppFace.h"
#include "app/AppServer.h"
#include "app/AppVoice.h"
#include "lib/utils.h"

void AppServer::setup() {
    static const char *headerKeys[] = {"Content-Type"};
    _httpServer.collectHeaders(headerKeys, 1);
    _httpServer.on("/", [&] { _onRoot(); });
    _httpServer.on("/speech", [&] { _onSpeech(); });
    _httpServer.on("/face", [&] { _onFace(); });
    _httpServer.on("/chat", [&] { _onChat(); });
    _httpServer.on("/apikey", HTTP_GET, [&] { _onApikey(); });
    _httpServer.on("/apikey_set", HTTP_POST, [&] { _onApikeySet(); });
    _httpServer.on("/role_get", HTTP_GET, [&] { _onRoleGet(); });
    _httpServer.on("/role_set", HTTP_POST, [&] { _onRoleSet(); });
    _httpServer.on("/setting", [&] { _onSetting(); });
    _httpServer.on("/settings", [&] { _onSettings(); });
    _httpServer.onNotFound([&] { _onNotFound(); });
    _httpServer.begin();
}

void AppServer::loop() {
    _httpServer.handleClient();
}

void AppServer::_onRoot() {
    _httpServer.send(200, "text/plain", "Hello, I'm Stack-chan!");
}

void AppServer::_onSpeech() {
    auto message = _httpServer.arg("say");
    auto expressionStr = _httpServer.arg("expression");
    auto voice = _httpServer.arg("voice");
    if (!_face->setExpressionIndex(expressionStr.toInt())) {
        _httpServer.send(400);
    }
    _voice->stopSpeak();
    _voice->speak(message, voice);
    _httpServer.send(200, "text/plain", "OK");
}

void AppServer::_onFace() {
    auto expressionStr = _httpServer.arg("expression");
    if (!_face->setExpressionIndex(expressionStr.toInt())) {
        _httpServer.send(400);
    }
    _httpServer.send(200, "text/plain", "OK");
}

void AppServer::_onChat() {
    auto text = _httpServer.arg("text");
    auto voiceName = _httpServer.arg("voice");
    _voice->stopSpeak();
    auto answer = _chat->talk(text, voiceName, true);
    _httpServer.send(200, "text/plain", answer);
}

void AppServer::_onApikey() {
    _httpServer.send(200, "text/plain", "OK");
}

void AppServer::_onApikeySet() {
    auto openAiApiKey = _httpServer.arg("openai");
    auto voiceTextApiKey = _httpServer.arg("voicetext");
    auto voicevoxApiKey = _httpServer.arg("voicevox");
    _chat->setOpenAiApiKey(openAiApiKey);
    if (voicevoxApiKey != "") {
        _voice->setTtsQuestVoicevoxApiKey(voicevoxApiKey);
    } else {
        _voice->setVoiceTextApiKey(voiceTextApiKey);
    }
    _httpServer.send(200, "text/plain", "OK");
}

void AppServer::_onRoleGet() {
    DynamicJsonDocument result(4 * 1024);
    result.createNestedArray("roles");
    for (const auto &role: _chat->getChatRoles()) {
        result["roles"].add(role);
    }
    _httpServer.send(200, "application/json", jsonEncode(result));
}

void AppServer::_onRoleSet() {
    auto roleStr = _httpServer.arg("plain");
    bool result;
    if (roleStr == "") {
        result = _chat->clearRoles();
    } else {
        result = _chat->addRole(roleStr);
    }
    if (!result) {
        _httpServer.send(400);
    } else {
        _httpServer.send(200, "text/plain", "OK");
    }
}

void AppServer::_onSetting() {
    auto volumeStr = _httpServer.arg("volume");
    auto voiceName = _httpServer.arg("voice");
    if (volumeStr != "") {
        if (!_voice->setVolume(volumeStr.toInt())) {
            _httpServer.send(400);
        }
    }
    if (voiceName != "") {
        if (!_voice->setVoiceName(voiceName)) {
            _httpServer.send(400);
        }
    }
    _httpServer.send(200, "text/plain", "OK");
}

void AppServer::_onSettings() {
    bool result = true;
    if (_httpServer.method() == HTTPMethod::HTTP_POST ||
        _httpServer.method() == HTTPMethod::HTTP_PUT) {
        if (_httpServer.header("Content-Type").startsWith("application/json")) {
            result = _settings->load(_httpServer.arg("plain"),
                                     _httpServer.method() == HTTPMethod::HTTP_PUT);
        } else {
            for (int i = 0; i < _httpServer.args(); i++) {
                auto name = _httpServer.argName(i);
                auto val = _httpServer.arg(i);
                if (val == "") {
                    _settings->remove(name);
                } else if (std::all_of(val.begin(), val.end(), ::isdigit)) {
                    _settings->set(name, std::stoi(val.c_str()));
                } else if (val == "true") {
                    _settings->set(name, true);
                } else if (val == "false") {
                    _settings->set(name, false);
                } else {
                    _settings->set(name, val);
                }
            }
        }
    }
    auto settings = jsonEncode(_settings->get(""));
    if (!result) {
        _httpServer.send(400);
    } else {
        _httpServer.send(200, "application/json", settings);
    }
}

void AppServer::_onNotFound() {
    _httpServer.send(404);
}
