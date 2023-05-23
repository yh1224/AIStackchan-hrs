#include <deque>
#include <Arduino.h>
#include <AudioFileSourceBuffer.h>
#include <AudioGeneratorMP3.h>
#include <M5Unified.h>

#include "app/AppVoice.h"
#include "app/config.h"
#include "lib/AudioFileSourceGoogleTranslateTts.h"
#include "lib/AudioFileSourceVoiceText.h"
#include "lib/AudioOutputM5Speaker.hpp"
#include "lib/utils.h"

/// size for audio buffer
static const int BUFFER_SIZE = 16 * 1024;

bool AppVoice::init() {
    _audioMp3 = std::unique_ptr<AudioGeneratorMP3>(new AudioGeneratorMP3());

    _allocatedBuffer = std::unique_ptr<uint8_t>((uint8_t *) malloc(BUFFER_SIZE));
    if (!_allocatedBuffer) {
        M5.Display.printf("FATAL: Unable to allocate buffer");
        return false;
    }

    return true;
}

void AppVoice::setup() {
    auto spk_cfg = M5.Speaker.config();
    spk_cfg.sample_rate = 96000;
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    M5.Speaker.config(spk_cfg);
    M5.Speaker.begin();
}

void AppVoice::start() {
    xTaskCreatePinnedToCore(
            [](void *arg) {
                auto *self = (AppVoice *) arg;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
                while (true) {
                    self->_loop();
                }
#pragma clang diagnostic pop
            },
            "AppVoice",
            8192,
            this,
            1,
            &_taskHandle,
            APP_CPU_NUM
    );
}

static const int LEVEL_MIN = 100;
static const int LEVEL_MAX = 15000;

/**
 * Get audio level
 *
 * @return audio level (0.0-1.0)
 */
float AppVoice::getAudioLevel() {
    xSemaphoreTake(_lock, portMAX_DELAY);
    int level = abs(*_audioOut.getBuffer());
    xSemaphoreGive(_lock);
    if (level < LEVEL_MIN) {
        level = 0;
    } else if (level > LEVEL_MAX) {
        level = LEVEL_MAX;
    }
    return (float) level / LEVEL_MAX;
}

/**
 * Check if voice is playing
 *
 * @return true: playing, false: not playing
 */
bool AppVoice::isPlaying() {
    xSemaphoreTake(_lock, portMAX_DELAY);
    auto result = _audioMp3->isRunning();
    xSemaphoreGive(_lock);
    return result;
}

/**
 * Set VoiceText API Key
 *
 * @param apiKey VoiceText API Key
 */
bool AppVoice::setVoiceTextApiKey(const String &apiKey) {
    if (apiKey == "") {
        return _settings->remove(CONFIG_VOICE_VOICETEXT_APIKEY_KEY)
               && _settings->set(CONFIG_VOICE_SERVICE_KEY, String(CONFIG_VOICE_SERVICE_GOOGLE_TRANSLATE_TTS));
    } else {
        return _settings->set(CONFIG_VOICE_VOICETEXT_APIKEY_KEY, apiKey)
               && _settings->set(CONFIG_VOICE_SERVICE_KEY, String(CONFIG_VOICE_SERVICE_VOICETEXT));
    }
}

/**
 * Set volume
 *
 * @param volume volume (0-255)
 * @return true: success, false: failure
 */
bool AppVoice::setVolume(uint8_t volume) {
    return _settings->set(CONFIG_VOICE_VOLUME_KEY, (int) volume);
}

/**
 * Start speaking text
 *
 * @param text text
 */
void AppVoice::speak(const String &text) {
    xSemaphoreTake(_lock, portMAX_DELAY);
    // add each sentence to message list
    for (const auto &sentence: splitSentence(text.c_str())) {
        _speechMessages.push_back(std::unique_ptr<String>(new String(sentence.c_str())));
    }
    xSemaphoreGive(_lock);
}

/**
 * Stop speaking
 */
void AppVoice::stopSpeak() {
    xSemaphoreTake(_lock, portMAX_DELAY);
    _isRunning = false;
    _speechMessages.clear();
    xSemaphoreGive(_lock);
}

void AppVoice::_loop() {
    xSemaphoreTake(_lock, portMAX_DELAY);
    auto isRunning = _isRunning;
    xSemaphoreGive(_lock);

    if (_audioMp3->isRunning()) { // playing
        if (!isRunning || !_audioMp3->loop()) {
            _audioMp3->stop();
            Serial.println("voice stop");
        }
    } else {
        // Get next message and start playing
        xSemaphoreTake(_lock, portMAX_DELAY);
        std::unique_ptr<String> message = nullptr;
        if (!_speechMessages.empty()) {
            message = std::move(_speechMessages.front());
            _speechMessages.pop_front();
        }
        xSemaphoreGive(_lock);
        if (message != nullptr) {
            xSemaphoreTake(_lock, portMAX_DELAY);
            _isRunning = true;
            xSemaphoreGive(_lock);
            M5.Speaker.setVolume(_getVoiceVolume());
            M5.Speaker.setChannelVolume(_speakerChannel, _getVoiceVolume());
            if (strcasecmp(_getVoiceService(), CONFIG_VOICE_SERVICE_VOICETEXT) == 0
                && _getVoiceTextApiKey() != nullptr) {
                // VoiceText API
                _audioSource = std::unique_ptr<AudioFileSource>(new AudioFileSourceVoiceText(
                        _getVoiceTextApiKey(), message->c_str(), _getVoiceTextParams()));
            } else {
                // Google TTS
                _audioSource = std::unique_ptr<AudioFileSource>(new AudioFileSourceGoogleTranslateTts(
                        message->c_str(), _getVoiceLang()));
            }
            _audioSourceBuffer = std::unique_ptr<AudioFileSourceBuffer>(
                    new AudioFileSourceBuffer(_audioSource.get(), _allocatedBuffer.get(), BUFFER_SIZE));
            _audioMp3->begin(_audioSourceBuffer.get(), &_audioOut);
            Serial.printf("voice start: %s\n", message->c_str());
        }
        delay(200);
    }
}

uint8_t AppVoice::_getVoiceVolume() {
    return (uint8_t) (_settings->get(CONFIG_VOICE_VOLUME_KEY) | CONFIG_VOICE_VOLUME_DEFAULT);
}

const char *AppVoice::_getVoiceLang() {
    return (const char *) (_settings->get(CONFIG_VOICE_LANG_KEY) | CONFIG_VOICE_LANG_DEFAULT);
}

const char *AppVoice::_getVoiceService() {
    return _settings->get(CONFIG_VOICE_SERVICE_KEY) | CONFIG_VOICE_SERVICE_DEFAULT;
}

const char *AppVoice::_getVoiceTextApiKey() {
    return _settings->get(CONFIG_VOICE_VOICETEXT_APIKEY_KEY);
}

const char *AppVoice::_getVoiceTextParams() {
    return (const char *) (_settings->get(CONFIG_VOICE_VOICETEXT_PARAMS_KEY)
                           | CONFIG_VOICE_VOICETEXT_PARAMS_DEFAULT);
}
