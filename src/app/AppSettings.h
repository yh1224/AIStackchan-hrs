#if !defined(APP_SETTINGS_H)
#define APP_SETTINGS_H

#include <memory>
#include <utility>
#include "lib/NvsSettings.h"

#define VOICE_SERVICE_GOOGLE_TRANSLATE_TTS "google-translate-tts"
#define VOICE_SERVICE_GOOGLE_CLOUD_TTS "google-cloud-tts"
#define VOICE_SERVICE_VOICETEXT "voicetext"
#define VOICE_SERVICE_TTS_QUEST_VOICEVOX "tts-quest-voicevox"

class AppSettings : public NvsSettings {
public:
    explicit AppSettings(String nvsNamespace, String nvsKey)
            : NvsSettings(std::move(nvsNamespace), std::move(nvsKey)) {}

    bool init();

    const char *getNetworkWifiSsid();

    const char *getNetworkWifiPass();

    const char *getNetworkHostname();

    const char *getTimeZone();

    const char *getTimeNtpServer();

    bool isServoEnabled();

    std::pair<int, int> getServoPin();

    std::pair<int, int> getSwingHome();

    std::pair<int, int> getSwingRange();

    String getLang();

    uint8_t getVoiceVolume();

    bool setVoiceVolume(uint8_t volume);

    const char *getVoiceService();

    bool setVoiceService(const String &service);

    const char *getVoiceTextApiKey();

    bool setVoiceTextApiKey(const String &apiKey);

    const char *getVoiceTextParams();

    bool setVoiceTextParams(const String &params);

    const char *getTtsQuestVoicevoxApiKey();

    bool setTtsQuestVoicevoxApiKey(const String &apiKey);

    const char *getTtsQuestVoicevoxParams();

    bool setTtsQuestVoicevoxParams(const String &params);

    const char *getOpenAiApiKey();

    bool setOpenAiApiKey(const String &apiKey);

    const char *getChatGptModel();

    bool useChatGptStream();

    std::vector<String> getChatRoles();

    bool addRole(const String &role);

    bool clearRoles();

    int getMaxHistory();

    bool isRandomSpeakEnabled();

    std::pair<int, int> getChatRandomInterval();

    std::vector<String> getChatRandomQuestions();

    bool isClockSpeakEnabled();

    std::vector<int> getChatClockHours();
};

#endif // !defined(APP_SETTINGS_H)
