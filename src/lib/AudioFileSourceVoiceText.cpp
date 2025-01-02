#include <utility>
#include <Arduino.h>

#include "AudioFileSourceVoiceText.h"
#include "lib/ssl.h"
#include "lib/url.h"

static const char *VOICETEXT_TTS_API_URL = "https://api.voicetext.jp/v1/tts";

AudioFileSourceVoiceText::AudioFileSourceVoiceText(String apiKey, String text, UrlParams params)
        : _apiKey(std::move(apiKey)), _text(std::move(text)), _params(std::move(params)) {
#if defined(USE_CA_CERT_BUNDLE)
    _secureClient.setCACertBundle(rootca_crt_bundle);
#else
    _secureClient.setCACert(gsrsaovsslca2018_crt);
#endif
    open(VOICETEXT_TTS_API_URL);
}

bool AudioFileSourceVoiceText::open(const char *url) {
    _http.setReuse(false);
    if (!_http.begin(_secureClient, url)) {
        Serial.println("ERROR: HTTPClient begin failed.");
        return false;
    }
    _http.setAuthorization(_apiKey.c_str(), "");
    _http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    auto params = _params;
    params["text"] = _text.c_str();
    params["format"] = "mp3";
    String request = qsBuild(params).c_str();

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
