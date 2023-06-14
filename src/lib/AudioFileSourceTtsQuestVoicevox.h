#if !defined(AudioFileSourceTtsQuestVoicevox_H)
#define AudioFileSourceTtsQuestVoicevox_H

#include <Arduino.h>

#include "AudioFileSourceHttp.h"
#include "lib/url.h"

class AudioFileSourceTtsQuestVoicevox : public AudioFileSourceHttp {
public:
    AudioFileSourceTtsQuestVoicevox(String apiKey, String text, UrlParams params);

    bool open(const char *url) override;

private:
    String _apiKey;
    String _text;
    UrlParams _params;
};

#endif // AudioFileSourceTtsQuestVoicevox_H
