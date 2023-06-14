#if !defined(AudioFileSourceHttp_H)
#define AudioFileSourceHttp_H

#include <Arduino.h>
#include <AudioFileSource.h>
#include <HTTPClient.h>

class AudioFileSourceHttp : public AudioFileSource {
public:
    explicit AudioFileSourceHttp() = default;

    explicit AudioFileSourceHttp(const char *text);

    ~AudioFileSourceHttp() override;

    bool open(const char *url) override;

    uint32_t read(void *data, uint32_t len) override;

    uint32_t readNonBlock(void *data, uint32_t len) override;

    bool seek(int32_t pos, int dir) override;

    bool close() override;

    bool isOpen() override;

    uint32_t getSize() override;

    uint32_t getPos() override;

protected:
    WiFiClient _client;
    WiFiClientSecure _secureClient;
    HTTPClient _http;

protected:
    int _pos = 0;
    size_t _chunkLen = 0;

    uint32_t _read(void *data, uint32_t len, bool nonBlock);

    bool _isChunked();
};

#endif // AudioFileSourceHttp_H
