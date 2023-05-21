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
        Serial.println("Connection failed.");
        return false;
    }

    String auth = base64::encode(_apiKey + ":");
    _http.addHeader("Authorization", "Basic " + auth);
    _http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String request = String("text=") + urlEncode(_text.c_str()) + "&format=mp3&" + _ttsParams;
    _http.addHeader("Content-Length", String(request.length()));

    auto code = _http.POST(request);
    if (code != HTTP_CODE_OK) {
        Serial.printf("Error: %d\n", code);
        _http.end();
        return false;
    }
    _size = _http.getSize();
    return true;
}
