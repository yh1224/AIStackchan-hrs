#include <Arduino.h>
#include "AudioFileSourceHttp.h"

AudioFileSourceHttp::AudioFileSourceHttp(const char *url) {
    open(url);
}

bool AudioFileSourceHttp::open(const char *url) {
    _http.setReuse(false);
    static const char *headerKeys[] = {"Transfer-Encoding"};
    _http.collectHeaders(headerKeys, 1);
    if (!_http.begin(String(url).startsWith("https://") ? _secureClient : _client, url)) {
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

    if (_isChunked() && _chunkLen == 0) {
        // Read chunk size
        char buf[16];
        int pos = 0;
        while (true) {
            auto available = stream->available();
            if (available == 0) {
                delay(10);
                continue;
            }
            auto c = stream->read();
            if (c < 0) {
                Serial.printf("readChunk: read failed (%d)\n", c);
                _http.end();
                return 0;
            }
            buf[pos++] = (char) c;
            if (pos > 8) {  // hex 6 桁まで
                Serial.println("readChunk: Invalid chunk size (too long)");
                _http.end();
                return 0;
            }
            if (pos >= 2 && buf[pos - 2] == '\r' && buf[pos - 1] == '\n') {
                buf[pos - 2] = '\0';
                //Serial.printf("readChunk: chunkSize=%s\n", buf);
                char *endp;
                _chunkLen = strtol((char *) buf, &endp, 16);
                if (endp != (char *) &buf[pos - 2]) {
                    Serial.printf("readChunk: Invalid chunk size: %s\n", buf);
                    _http.end();
                    return 0;
                }
                //Serial.printf("readChunk: chunkSize=%d\n", _chunkLen);
                break;
            }
        }
        if (_chunkLen == 0) {
            _http.end();
            return 0;
        }
    }

    if (_isChunked() && len > _chunkLen) {
        len = _chunkLen;
    }
    int readBytes = stream->read(reinterpret_cast<uint8_t *>(data), len);
    if (readBytes <= 0) {
        return 0;
    }
    _pos += readBytes;
    if (_isChunked()) {
        _chunkLen -= readBytes;
        if (_chunkLen == 0) {
            // Skip chunk delimiter
            char buf[2];
            auto skipLen = stream->readBytes(buf, 2);
            if (skipLen != 2 || buf[0] != '\r' || buf[1] != '\n') {
                Serial.println("readChunk: Invalid chunk delimiter");
                _http.end();
                return 0;
            }
        }
    }
    return readBytes;
}

bool AudioFileSourceHttp::_isChunked() {
    return _http.header("Transfer-Encoding") == "chunked";
}
