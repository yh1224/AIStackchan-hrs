#if !defined(APP_VOICE_H)
#define APP_VOICE_H

#include <deque>
#include <utility>
#include <AudioFileSourceBuffer.h>
#include <AudioGeneratorMP3.h>

#include "lib/AudioFileSourceVoiceText.h"
#include "lib/AudioOutputM5Speaker.hpp"
#include "lib/NvsSettings.h"

class AppVoice {
public:
    explicit AppVoice(
            std::shared_ptr<NvsSettings> settings
    ) : _settings(std::move(settings)) {};

    bool init();

    void setup();

    void start();

    float getAudioLevel();

    bool isPlaying();

    bool setVoiceTextApiKey(const String &apiKey);

    bool setVolume(uint8_t volume);

    void speak(const String &text);

    void stopSpeak();

private:
    std::shared_ptr<NvsSettings> _settings;

    TaskHandle_t _taskHandle{};

    SemaphoreHandle_t _lock = xSemaphoreCreateMutex();

    bool _isRunning{};

    /// M5Speaker virtual channel (0-7)
    uint8_t _speakerChannel = 0;

    /// message list to play
    std::deque<std::unique_ptr<String>> _speechMessages;

    /// output speaker
    AudioOutputM5Speaker _audioOut{&M5.Speaker, _speakerChannel};

    /// mp3 decoder
    std::unique_ptr<AudioGeneratorMP3> _audioMp3;

    /// audio source from text
    std::unique_ptr<AudioFileSource> _audioSource;

    /// buffer for playing audio
    std::unique_ptr<AudioFileSourceBuffer> _audioSourceBuffer;

    /// buffer area for playing audio
    std::unique_ptr<uint8_t> _allocatedBuffer;

    void _loop();

    uint8_t _getVoiceVolume();

    const char *_getVoiceLang();

    const char *_getVoiceService();

    const char *_getVoiceTextApiKey();

    const char *_getVoiceTextParams();
};

#endif // !defined(APP_VOICE_H)
