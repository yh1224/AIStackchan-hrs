#include <utility>
#include <Arduino.h>
#include <base64.h>

#include "AudioFileSourceVoiceText.h"
#include "lib/utils.h"

static const char *VOICETEXT_TTS_API_URL = "https://api.voicetext.jp/v1/tts";

AudioFileSourceVoiceText::AudioFileSourceVoiceText(String apiKey, String text, String ttsParams)
        : _apiKey(std::move(apiKey)), _text(std::move(text)), _ttsParams(std::move(ttsParams)) {
    open(VOICETEXT_TTS_API_URL);
}

bool AudioFileSourceVoiceText::open(const char *url) {
    _http.setReuse(false);
    if (!_http.begin(url)) {
        Serial.println("ERROR: HTTPClient begin failed.");
        return false;
    }
    _http.setAuthorization(_apiKey.c_str(), "");
    _http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String request = "text=" + urlEncode(_text.c_str()) + "&format=mp3&" + _ttsParams;

    Serial.printf(">>> POST %s\n", url);
    Serial.println(request);
    auto httpCode = _http.POST(request);
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("ERROR: %d\n", httpCode);
        _http.end();
        return false;
    }
    return true;
}
