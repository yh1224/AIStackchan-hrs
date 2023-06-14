#include <deque>
#include <Arduino.h>
#include <AudioFileSourceBuffer.h>
#include <AudioGeneratorMP3.h>
#include <M5Unified.h>

#include "app/AppVoice.h"
#include "lib/AudioFileSourceGoogleTranslateTts.h"
#include "lib/AudioFileSourceTtsQuestVoicevox.h"
#include "lib/AudioFileSourceVoiceText.h"
#include "lib/AudioOutputM5Speaker.hpp"
#include "lib/url.h"
#include "lib/utils.h"

/// size for audio buffer
static const int BUFFER_SIZE = 16 * 1024;

/// parameters for VoiceText
const static char *VOICETEXT_VOICE_PARAMS[] = {
        "speaker=takeru&speed=100&pitch=130&emotion=happiness&emotion_level=4",
        "speaker=hikari&speed=120&pitch=130&emotion=happiness&emotion_level=2",
        "speaker=bear&speed=120&pitch=100&emotion=anger&emotion_level=2",
        "speaker=haruka&speed=80&pitch=70&emotion=happiness&emotion_level=2",
        "&speaker=santa&speed=120&pitch=90&emotion=happiness&emotion_level=4",
};

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
 * Set voice name
 *
 * @param voiceName voice name
 * @return true: success, false: failure
 */
bool AppVoice::setVoiceName(const String &voiceName) {
    if (strcasecmp(_settings->getVoiceService(), VOICE_SERVICE_VOICETEXT) == 0) {
        auto params = qsParse(_settings->getVoiceTextParams());
        int voiceNum = std::stoi(voiceName.c_str());
        if (voiceNum >= 0 && voiceNum <= 4) {
            for (const auto &item: qsParse(VOICETEXT_VOICE_PARAMS[voiceNum])) {
                params[item.first] = item.second;
            }
            return _settings->setVoiceTextParams(qsBuild(params).c_str());
        } else {
            return false;
        }
    } else if (strcasecmp(_settings->getVoiceService(), VOICE_SERVICE_TTS_QUEST_VOICEVOX) == 0) {
        auto params = qsParse(_settings->getTtsQuestVoicevoxParams());
        params["speaker"] = voiceName.c_str();
        return _settings->setTtsQuestVoicevoxParams(qsBuild(params).c_str());
    } else {
        return false;
    }
}

/**
 * Start speaking text
 *
 * @param text text
 */
void AppVoice::speak(const String &text, const String &voiceName) {
    xSemaphoreTake(_lock, portMAX_DELAY);
    // add each sentence to message list
    for (const auto &sentence: splitSentence(text.c_str())) {
        _speechMessages.push_back(std::unique_ptr<SpeechMessage>(
                new SpeechMessage(sentence.c_str(), voiceName.c_str())));
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
        std::unique_ptr<SpeechMessage> message = nullptr;
        if (!_speechMessages.empty()) {
            message = std::move(_speechMessages.front());
            _speechMessages.pop_front();
        }
        xSemaphoreGive(_lock);
        if (message != nullptr) {
            xSemaphoreTake(_lock, portMAX_DELAY);
            _isRunning = true;
            xSemaphoreGive(_lock);
            M5.Speaker.setVolume(_settings->getVoiceVolume());
            M5.Speaker.setChannelVolume(_speakerChannel, _settings->getVoiceVolume());
            if (strcasecmp(_settings->getVoiceService(), VOICE_SERVICE_TTS_QUEST_VOICEVOX) == 0) {
                // TTS QUEST VOICEVOX API
                auto params = qsParse(_settings->getTtsQuestVoicevoxParams());
                if (!message->voice.isEmpty()) {
                    params["speaker"] = message->voice.c_str();
                }
                _audioSource = std::unique_ptr<AudioFileSource>(new AudioFileSourceTtsQuestVoicevox(
                        _settings->getTtsQuestVoicevoxApiKey(), message->text.c_str(), params));
            } else if (strcasecmp(_settings->getVoiceService(), VOICE_SERVICE_VOICETEXT) == 0
                       && _settings->getVoiceTextApiKey() != nullptr) {
                // VoiceText API
                auto params = qsParse(_settings->getVoiceTextParams());
                if (!message->voice.isEmpty()) {
                    int voiceNum = std::stoi(message->voice.c_str());
                    if (voiceNum >= 0 && voiceNum <= 4) {
                        for (const auto &item: qsParse(VOICETEXT_VOICE_PARAMS[voiceNum])) {
                            params[item.first] = item.second;
                        }
                    }
                }
                _audioSource = std::unique_ptr<AudioFileSource>(new AudioFileSourceVoiceText(
                        _settings->getVoiceTextApiKey(), message->text.c_str(), params));
            } else {
                // Google Translate TTS
                UrlParams params;
                params["tl"] = _settings->getLang().c_str();
                _audioSource = std::unique_ptr<AudioFileSource>(new AudioFileSourceGoogleTranslateTts(
                        message->text.c_str(), params));
            }
            _audioSourceBuffer = std::unique_ptr<AudioFileSourceBuffer>(
                    new AudioFileSourceBuffer(_audioSource.get(), _allocatedBuffer.get(), BUFFER_SIZE));
            _audioMp3->begin(_audioSourceBuffer.get(), &_audioOut);
            Serial.printf("voice start: %s\n", message->text.c_str());
        }
        delay(200);
    }
}
