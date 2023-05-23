#include <Arduino.h>
#include "AudioFileSourceHttp.h"

AudioFileSourceHttp::AudioFileSourceHttp(const char *url) {
    open(url);
}

bool AudioFileSourceHttp::open(const char *url) {
    _http.setReuse(false);
    if (!_http.begin(url)) {
        Serial.println("ERROR: HTTPClient begin failed.");
        return false;
    }

    Serial.printf(">>> GET %s\n", url);
    auto httpCode = _http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("ERROR: %d\n", httpCode);
        _http.end();
        return false;
    }
    return true;
}

AudioFileSourceHttp::~AudioFileSourceHttp() {
    _http.end();
}

uint32_t AudioFileSourceHttp::read(void *data, uint32_t len) {
    return _read(data, len, false);
}

uint32_t AudioFileSourceHttp::readNonBlock(void *data, uint32_t len) {
    return _read(data, len, true);
}

bool AudioFileSourceHttp::seek(int32_t pos, int dir) {
    audioLogger->printf_P(PSTR("ERROR: AudioFileSourceHttpStream::seek() not implemented"));
    return false;
}

bool AudioFileSourceHttp::close() {
    _http.end();
    return true;
}

bool AudioFileSourceHttp::isOpen() {
    return _http.connected();
}

uint32_t AudioFileSourceHttp::getSize() {
    return _http.getSize();
}

uint32_t AudioFileSourceHttp::getPos() {
    return _pos;
}

uint32_t AudioFileSourceHttp::_read(void *data, uint32_t len, bool nonBlock) {
    auto size = getSize();
    if (!_http.connected() || (size > 0 && _pos >= size)) {
        return 0;
    }

    auto stream = _http.getStreamPtr();
    if (!nonBlock) {
        auto start = millis();
        while (_http.connected() && stream->available() < (int) len && (millis() - start < 500)) {
            delay(10);
        }
    }

    auto availBytes = stream->available();
    if (availBytes == 0) {
        return 0;
    }
    int readBytes = stream->read(reinterpret_cast<uint8_t *>(data), len);
    if (readBytes <= 0) {
        return 0;
    }
    _pos += readBytes;
    return readBytes;
}
