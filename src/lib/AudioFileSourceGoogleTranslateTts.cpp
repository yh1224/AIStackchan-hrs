#include <google-tts.h>

#include "AudioFileSourceGoogleTranslateTts.h"

AudioFileSourceGoogleTranslateTts::AudioFileSourceGoogleTranslateTts(const char *text, const char *lang) {
    TTS tts;
    auto url = tts.getSpeechUrl(text, lang);
    // TODO: https だと音が切れる?? とりあえず http に変換する。
    url.replace("https://", "http://");
    open(url.c_str());
}
