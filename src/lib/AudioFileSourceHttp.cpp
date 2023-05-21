#include <Arduino.h>
#include "AudioFileSourceHttp.h"

AudioFileSourceHttp::AudioFileSourceHttp(const char *url) {
    open(url);
}

bool AudioFileSourceHttp::open(const char *url) {
    _http.setReuse(false);

    if (!_http.begin(url)) {
        Serial.println("Connection failed.");
        return false;
    }

    auto code = _http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("Error: %d\n", code);
        _http.end();
        return false;
    }
    _size = _http.getSize();
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
    return _size;
}

uint32_t AudioFileSourceHttp::getPos() {
    return _pos;
}

uint32_t AudioFileSourceHttp::_read(void *data, uint32_t len, bool nonBlock) {
    if (!_http.connected() || (_size > 0 && _pos >= _size)) {
        return 0;
    }

    auto stream = _http.getStreamPtr();
    if (!nonBlock) {
        auto start = millis();
        while ((stream->available() < (int) len) && (millis() - start < 500)) {
            yield();
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
