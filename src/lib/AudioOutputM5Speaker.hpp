#if !defined(AudioOutputM5Speaker_H)
#define AudioOutputM5Speaker_H

#include <AudioOutput.h>
#include <M5Unified.h>

static const int BUF_SIZE = 640;
static const int BUF_NUM = 3;

class AudioOutputM5Speaker : public AudioOutput {
public:
    explicit AudioOutputM5Speaker(m5::Speaker_Class *m5Speaker, uint8_t channel = 0)
            : _m5Speaker(m5Speaker), _channel(channel) {};

    bool begin() override {
        return true;
    }

    bool ConsumeSample(int16_t sample[2]) override {
        if (_pos >= BUF_SIZE) {
            flush();
            return false;
        }
        _buf[_index][_pos++] = sample[LEFTCHANNEL];
        _buf[_index][_pos++] = sample[LEFTCHANNEL];
        return true;
    }

    void flush() override {
        if (_pos > 0) {
            _m5Speaker->playRaw(
                    _buf[_index], _pos,
                    hertz, true, 1, _channel);
            _index = (_index + 1) % BUF_NUM;
            _pos = 0;
        }
    }

    bool stop() override {
        flush();
        _m5Speaker->stop(_channel);
        memset(_buf, 0, BUF_NUM * BUF_SIZE * sizeof(int16_t));
        return true;
    }

    const int16_t *getBuffer() const {
        return _buf[(_index + BUF_NUM - 1) % BUF_NUM];
    }

private:
    m5::Speaker_Class *_m5Speaker;
    uint8_t _channel;

    int16_t _buf[BUF_NUM][BUF_SIZE]{};
    size_t _pos = 0;
    size_t _index = 0;
};

#endif // !defined(AudioOutputM5Speaker_H)
