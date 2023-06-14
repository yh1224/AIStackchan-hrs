#include <utility>
#include <Arduino.h>

#include "AudioFileSourceVoiceText.h"
#include "lib/url.h"

/// https://support.globalsign.com/ca-certificates/intermediate-certificates/organizationssl-intermediate-certificates
/// GlobalSign RSA Organization Validation CA - 2018, Valid until: 21 November 2028
static const char *caCert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIETjCCAzagAwIBAgINAe5fIh38YjvUMzqFVzANBgkqhkiG9w0BAQsFADBMMSAw\n" \
"HgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEGA1UEChMKR2xvYmFs\n" \
"U2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0xODExMjEwMDAwMDBaFw0yODEx\n" \
"MjEwMDAwMDBaMFAxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9iYWxTaWduIG52\n" \
"LXNhMSYwJAYDVQQDEx1HbG9iYWxTaWduIFJTQSBPViBTU0wgQ0EgMjAxODCCASIw\n" \
"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKdaydUMGCEAI9WXD+uu3Vxoa2uP\n" \
"UGATeoHLl+6OimGUSyZ59gSnKvuk2la77qCk8HuKf1UfR5NhDW5xUTolJAgvjOH3\n" \
"idaSz6+zpz8w7bXfIa7+9UQX/dhj2S/TgVprX9NHsKzyqzskeU8fxy7quRU6fBhM\n" \
"abO1IFkJXinDY+YuRluqlJBJDrnw9UqhCS98NE3QvADFBlV5Bs6i0BDxSEPouVq1\n" \
"lVW9MdIbPYa+oewNEtssmSStR8JvA+Z6cLVwzM0nLKWMjsIYPJLJLnNvBhBWk0Cq\n" \
"o8VS++XFBdZpaFwGue5RieGKDkFNm5KQConpFmvv73W+eka440eKHRwup08CAwEA\n" \
"AaOCASkwggElMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8ECDAGAQH/AgEAMB0G\n" \
"A1UdDgQWBBT473/yzXhnqN5vjySNiPGHAwKz6zAfBgNVHSMEGDAWgBSP8Et/qC5F\n" \
"JK5NUPpjmove4t0bvDA+BggrBgEFBQcBAQQyMDAwLgYIKwYBBQUHMAGGImh0dHA6\n" \
"Ly9vY3NwMi5nbG9iYWxzaWduLmNvbS9yb290cjMwNgYDVR0fBC8wLTAroCmgJ4Yl\n" \
"aHR0cDovL2NybC5nbG9iYWxzaWduLmNvbS9yb290LXIzLmNybDBHBgNVHSAEQDA+\n" \
"MDwGBFUdIAAwNDAyBggrBgEFBQcCARYmaHR0cHM6Ly93d3cuZ2xvYmFsc2lnbi5j\n" \
"b20vcmVwb3NpdG9yeS8wDQYJKoZIhvcNAQELBQADggEBAJmQyC1fQorUC2bbmANz\n" \
"EdSIhlIoU4r7rd/9c446ZwTbw1MUcBQJfMPg+NccmBqixD7b6QDjynCy8SIwIVbb\n" \
"0615XoFYC20UgDX1b10d65pHBf9ZjQCxQNqQmJYaumxtf4z1s4DfjGRzNpZ5eWl0\n" \
"6r/4ngGPoJVpjemEuunl1Ig423g7mNA2eymw0lIYkN5SQwCuaifIFJ6GlazhgDEw\n" \
"fpolu4usBCOmmQDo8dIm7A9+O4orkjgTHY+GzYZSR+Y0fFukAj6KYXwidlNalFMz\n" \
"hriSqHKvoflShx8xpfywgVcvzfTO3PYkz6fiNJBonf6q8amaEsybwMbDqKWwIX7e\n" \
"SPY=\n" \
"-----END CERTIFICATE-----\n" \
"";

static const char *VOICETEXT_TTS_API_URL = "https://api.voicetext.jp/v1/tts";

AudioFileSourceVoiceText::AudioFileSourceVoiceText(String apiKey, String text, UrlParams params)
        : _apiKey(std::move(apiKey)), _text(std::move(text)), _params(std::move(params)) {
    _secureClient.setCACert(caCert);
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
