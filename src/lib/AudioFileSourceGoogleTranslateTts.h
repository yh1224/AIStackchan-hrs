#if !defined(AudioFileSourceGoogleTranslateTts_H)
#define AudioFileSourceGoogleTranslateTts_H

#include "AudioFileSourceHttp.h"

class AudioFileSourceGoogleTranslateTts : public AudioFileSourceHttp {
public:
    explicit AudioFileSourceGoogleTranslateTts(const char *text, const char *lang);
};

#endif // AudioFileSourceGoogleTranslateTts_H
