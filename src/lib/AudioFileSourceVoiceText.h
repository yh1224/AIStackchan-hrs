#if !defined(AudioFileSourceVoiceText_H)
#define AudioFileSourceVoiceText_H

#include <Arduino.h>

#include "AudioFileSourceHttp.h"
#include "lib/url.h"

class AudioFileSourceVoiceText : public AudioFileSourceHttp {
public:
    AudioFileSourceVoiceText(String apiKey, String text, UrlParams params);

    bool open(const char *url) override;

private:
    String _apiKey;
    String _text;
    UrlParams _params;
};

#endif // AudioFileSourceVoiceText_H
