#if !defined(AudioFileSourceVoiceText_H)
#define AudioFileSourceVoiceText_H

#include <Arduino.h>

#include "AudioFileSourceHttp.h"

class AudioFileSourceVoiceText : public AudioFileSourceHttp {
public:
    AudioFileSourceVoiceText(String apiKey, String text, String ttsParams);

    bool open(const char *url) override;

private:
    String _apiKey;
    String _text;
    String _ttsParams;
};

#endif // AudioFileSourceVoiceText_H
