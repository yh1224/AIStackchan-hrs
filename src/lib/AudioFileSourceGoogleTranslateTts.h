#if !defined(AudioFileSourceGoogleTranslateTts_H)
#define AudioFileSourceGoogleTranslateTts_H

#include "AudioFileSourceHttp.h"
#include "lib/url.h"

class AudioFileSourceGoogleTranslateTts : public AudioFileSourceHttp {
public:
    explicit AudioFileSourceGoogleTranslateTts(const char *text, UrlParams params);
};

#endif // AudioFileSourceGoogleTranslateTts_H
