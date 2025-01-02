#include "AudioFileSourceGoogleTranslateTts.h"
#include "lib/ssl.h"
#include "lib/url.h"

static const char *GOOGLE_TRANSLATION_TTS_API_URL = "http://translate.google.com/translate_tts";

AudioFileSourceGoogleTranslateTts::AudioFileSourceGoogleTranslateTts(const char *text, UrlParams params) {
#if defined(USE_CA_CERT_BUNDLE)
    _secureClient.setCACertBundle(rootca_crt_bundle);
#else
    _secureClient.setCACert(gts_root_r1_crt);
#endif
    params["ie"] = "UTF-8";
    params["q"] = text;
    params["client"] = "tw-ob";
    params["ttsspeed"] = "1";
    auto url = String(GOOGLE_TRANSLATION_TTS_API_URL) + "?" + qsBuild(params).c_str();
    open(url.c_str());
}
