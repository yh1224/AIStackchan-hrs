#include <Arduino.h>

#include "AppSettings.h"
#include "lib/sdcard.h"

/// File path for settings
static const char *APP_SETTINGS_SD_PATH = "/settings.json";

static const char *NETWORK_WIFI_SSID_KEY = "network.wifi.ssid";
static const char *NETWORK_WIFI_PASS_KEY = "network.wifi.pass";
static const char *NETWORK_HOSTNAME_KEY = "network.hostname";
static const char *TIME_ZONE_KEY = "time.zone";
static const char *TIME_ZONE_DEFAULT = "JST-9";
static const char *TIME_NTP_SERVER_KEY = "time.ntpServer";
static const char *TIME_NTP_SERVER_DEFAULT = "ntp.nict.jp";

static const char *SERVO_KEY = "servo";
static const char *SERVO_PIN_X_KEY = "servo.pin.x";
static const char *SERVO_PIN_Y_KEY = "servo.pin.y";
static const char *SWING_HOME_X_KEY = "swing.home.x";
static const char *SWING_ENABLE_KEY = "swing.enable";
static const int SWING_ENABLE_DEFAULT = true;
static const int SWING_HOME_X_DEFAULT = 90;
static const char *SWING_HOME_Y_KEY = "swing.home.y";
static const int SWING_HOME_Y_DEFAULT = 80;
static const char *SWING_RANGE_X_KEY = "swing.range.x";
static const int SWING_RANGE_X_DEFAULT = 30;
static const char *SWING_RANGE_Y_KEY = "swing.range.y";
static const int SWING_RANGE_Y_DEFAULT = 20;

static const char *VOICE_LANG_KEY = "voice.lang";
static const char *VOICE_LANG_DEFAULT = "ja";
static const char *VOICE_VOLUME_KEY = "voice.volume";
static const int VOICE_VOLUME_DEFAULT = 200;
static const char *VOICE_SERVICE_KEY = "voice.service";
static const char *VOICE_SERVICE_DEFAULT = VOICE_SERVICE_GOOGLE_TRANSLATE_TTS;
static const char *VOICE_VOICETEXT_APIKEY_KEY = "voice.voicetext.apiKey";
static const char *VOICE_VOICETEXT_PARAMS_KEY = "voice.voicetext.params";
static const char *VOICE_VOICETEXT_PARAMS_DEFAULT = "speaker=hikari&speed=120&pitch=130&emotion=happiness";
static const char *VOICE_TTS_QUEST_VOICEVOX_APIKEY_KEY = "voice.tts-quest-voicevox.apiKey";
static const char *VOICE_TTS_QUEST_VOICEVOX_PARAMS_KEY = "voice.tts-quest-voicevox.params";
static const char *VOICE_TTS_QUEST_VOICEVOX_PARAMS_DEFAULT = "";

static const char *CHAT_OPENAI_APIKEY_KEY = "chat.openai.apiKey";
static const char *CHAT_OPENAI_CHATGPT_MODEL_KEY = "chat.openai.model";
static const char *CHAT_OPENAI_CHATGPT_MODEL_DEFAULT = "gpt-3.5-turbo";
static const char *CHAT_OPENAI_STREAM_KEY = "chat.openai.stream";
static const bool CHAT_OPENAI_STREAM_DEFAULT = false;
static const char *CHAT_OPENAI_ROLES_KEY = "chat.openai.roles";
static const char *CHAT_OPENAI_MAX_HISTORY_KEY = "chat.openai.maxHistory";
static const int CHAT_OPENAI_MAX_HISTORY_DEFAULT = 10;
static const char *CHAT_RANDOM_INTERVAL_MIN_KEY = "chat.random.interval.min";
static const int CHAT_RANDOM_INTERVAL_MIN_DEFAULT = 60;
static const char *CHAT_RANDOM_INTERVAL_MAX_KEY = "chat.random.interval.min";
static const int CHAT_RANDOM_INTERVAL_MAX_DEFAULT = 120;
static const char *CHAT_RANDOM_QUESTIONS_KEY = "chat.random.questions";
static const char *CHAT_CLOCK_HOURS_KEY = "chat.clock.hours";

bool AppSettings::init() {
    auto settings = sdLoadString(APP_SETTINGS_SD_PATH);
    if (settings != nullptr) {
        if (!load(*settings)) {
            return false;
        }
    } else {
        load();
    }
    return true;
}

const char *AppSettings::getNetworkWifiSsid() {
    return get(NETWORK_WIFI_SSID_KEY);
}

const char *AppSettings::getNetworkWifiPass() {
    return get(NETWORK_WIFI_PASS_KEY);
}

const char *AppSettings::getNetworkHostname() {
    return get(NETWORK_HOSTNAME_KEY);
}

const char *AppSettings::getTimeZone() {
    return get(TIME_ZONE_KEY) | TIME_ZONE_DEFAULT;
}

const char *AppSettings::getTimeNtpServer() {
    return get(TIME_NTP_SERVER_KEY) | TIME_NTP_SERVER_DEFAULT;;
}

bool AppSettings::isServoEnabled() {
    return has(SERVO_KEY);
}

std::pair<int, int> AppSettings::getServoPin() {
    int servoPinX = get(SERVO_PIN_X_KEY);
    int servoPinY = get(SERVO_PIN_Y_KEY);
    return std::make_pair(servoPinX, servoPinY);
}

bool AppSettings::getSwingEnabled() {
    return has(SWING_ENABLE_KEY) ? get(SWING_ENABLE_KEY) : SWING_ENABLE_DEFAULT;
}

std::pair<int, int> AppSettings::getSwingHome() {
    int homeX = get(SWING_HOME_X_KEY) | SWING_HOME_X_DEFAULT;
    int homeY = get(SWING_HOME_Y_KEY) | SWING_HOME_Y_DEFAULT;
    return std::make_pair(homeX, homeY);
}

std::pair<int, int> AppSettings::getSwingRange() {
    int homeX = get(SWING_RANGE_X_KEY) | SWING_RANGE_X_DEFAULT;
    int homeY = get(SWING_RANGE_Y_KEY) | SWING_RANGE_Y_DEFAULT;
    return std::make_pair(homeX, homeY);
}

String AppSettings::getLang() {
    String lang = get(VOICE_LANG_KEY) | VOICE_LANG_DEFAULT;
    return lang.substring(0, 2); // en-US -> en
}

uint8_t AppSettings::getVoiceVolume() {
    return get(VOICE_VOLUME_KEY) | VOICE_VOLUME_DEFAULT;
}

bool AppSettings::setVoiceVolume(uint8_t volume) {
    return set(VOICE_VOLUME_KEY, (int) volume);
}

const char *AppSettings::getVoiceService() {
    return get(VOICE_SERVICE_KEY) | VOICE_SERVICE_DEFAULT;
}

bool AppSettings::setVoiceService(const String &service) {
    return set(VOICE_SERVICE_KEY, service);
}

const char *AppSettings::getVoiceTextApiKey() {
    return get(VOICE_VOICETEXT_APIKEY_KEY);
}

bool AppSettings::setVoiceTextApiKey(const String &apiKey) {
    if (apiKey.isEmpty()) {
        return remove(VOICE_VOICETEXT_APIKEY_KEY);
    } else {
        return set(VOICE_VOICETEXT_APIKEY_KEY, apiKey);
    }
}

const char *AppSettings::getVoiceTextParams() {
    return get(VOICE_VOICETEXT_PARAMS_KEY) | VOICE_VOICETEXT_PARAMS_DEFAULT;
}

bool AppSettings::setVoiceTextParams(const String &params) {
    return set(VOICE_VOICETEXT_PARAMS_KEY, params);
}

const char *AppSettings::getTtsQuestVoicevoxApiKey() {
    return get(VOICE_TTS_QUEST_VOICEVOX_APIKEY_KEY);
}

bool AppSettings::setTtsQuestVoicevoxApiKey(const String &apiKey) {
    if (apiKey.isEmpty()) {
        return remove(VOICE_TTS_QUEST_VOICEVOX_APIKEY_KEY);
    } else {
        return set(VOICE_TTS_QUEST_VOICEVOX_APIKEY_KEY, apiKey);
    }
}

const char *AppSettings::getTtsQuestVoicevoxParams() {
    return get(VOICE_TTS_QUEST_VOICEVOX_PARAMS_KEY) | VOICE_TTS_QUEST_VOICEVOX_PARAMS_DEFAULT;
}

bool AppSettings::setTtsQuestVoicevoxParams(const String &params) {
    return set(VOICE_TTS_QUEST_VOICEVOX_PARAMS_KEY, params);
}

const char *AppSettings::getOpenAiApiKey() {
    return get(CHAT_OPENAI_APIKEY_KEY);
}

bool AppSettings::setOpenAiApiKey(const String &apiKey) {
    if (apiKey.isEmpty()) {
        return remove(CHAT_OPENAI_APIKEY_KEY);
    } else {
        return set(CHAT_OPENAI_APIKEY_KEY, apiKey);
    }
}

const char *AppSettings::getChatGptModel() {
    return get(CHAT_OPENAI_CHATGPT_MODEL_KEY) | CHAT_OPENAI_CHATGPT_MODEL_DEFAULT;
}

bool AppSettings::useChatGptStream() {
    return get(CHAT_OPENAI_STREAM_KEY) | CHAT_OPENAI_STREAM_DEFAULT;
}

std::vector<String> AppSettings::getChatRoles() {
    return getArray<String>(CHAT_OPENAI_ROLES_KEY);
}

bool AppSettings::addRole(const String &role) {
    return add(CHAT_OPENAI_ROLES_KEY, role);
}

bool AppSettings::clearRoles() {
    return clear(CHAT_OPENAI_ROLES_KEY);
}

int AppSettings::getMaxHistory() {
    return get(CHAT_OPENAI_MAX_HISTORY_KEY) | CHAT_OPENAI_MAX_HISTORY_DEFAULT;
}

bool AppSettings::isRandomSpeakEnabled() {
    return has(CHAT_RANDOM_QUESTIONS_KEY);
}

std::pair<int, int> AppSettings::getChatRandomInterval() {
    int min = get(CHAT_RANDOM_INTERVAL_MIN_KEY) | CHAT_RANDOM_INTERVAL_MIN_DEFAULT;
    int max = get(CHAT_RANDOM_INTERVAL_MAX_KEY) | CHAT_RANDOM_INTERVAL_MAX_DEFAULT;
    return std::make_pair(min, max);
}

std::vector<String> AppSettings::getChatRandomQuestions() {
    return getArray<String>(CHAT_RANDOM_QUESTIONS_KEY);
}

bool AppSettings::isClockSpeakEnabled() {
    return has(CHAT_CLOCK_HOURS_KEY);
}

std::vector<int> AppSettings::getChatClockHours() {
    return getArray<int>(CHAT_CLOCK_HOURS_KEY);
}
