#if !defined(APP_CONFIG_H)
#define APP_CONFIG_H

#define CONFIG_NETWORK_WIFI_SSID_KEY "network.wifi.ssid"
#define CONFIG_NETWORK_WIFI_PASS_KEY "network.wifi.pass"
#define CONFIG_NETWORK_HOSTNAME_KEY "network.hostname"
#define CONFIG_TIME_ZONE_KEY "time.zone"
#define CONFIG_TIME_ZONE_DEFAULT "JST-9"
#define CONFIG_TIME_NTP_SERVER_KEY "time.ntpServer"
#define CONFIG_TIME_NTP_SERVER_DEFAULT "ntp.nict.jp"
#define CONFIG_SERVO_PIN_X_KEY "servo.pin.x"
#define CONFIG_SERVO_PIN_Y_KEY "servo.pin.y"
#define CONFIG_VOICE_LANG_KEY "voice.lang"
#define CONFIG_VOICE_LANG_DEFAULT "ja"
#define CONFIG_VOICE_VOLUME_KEY "voice.volume"
#define CONFIG_VOICE_VOLUME_DEFAULT 200
#define CONFIG_VOICE_SERVICE_KEY "voice.service"
#define CONFIG_VOICE_SERVICE_GOOGLE_TRANSLATE_TTS "google-translate-tts"
#define CONFIG_VOICE_SERVICE_GOOGLE_CLOUD_TTS "google-cloud-tts"
#define CONFIG_VOICE_SERVICE_VOICETEXT "voicetext"
#define CONFIG_VOICE_SERVICE_DEFAULT CONFIG_VOICE_SERVICE_GOOGLE_TRANSLATE_TTS
#define CONFIG_VOICE_VOICETEXT_APIKEY_KEY "voice.voicetext.apiKey"
#define CONFIG_VOICE_VOICETEXT_PARAMS_KEY "voice.voicetext.params"
#define CONFIG_VOICE_VOICETEXT_PARAMS_DEFAULT "speaker=hikari&speed=120&pitch=130&emotion=happiness"
#define CONFIG_CHAT_OPENAI_STREAM_KEY "chat.openai.stream"
#define CONFIG_CHAT_OPENAI_STREAM_DEFAULT true
#define CONFIG_CHAT_OPENAI_APIKEY_KEY "chat.openai.apiKey"
#define CONFIG_CHAT_OPENAI_ROLES_KEY "chat.openai.roles"
#define CONFIG_CHAT_OPENAI_MAX_HISTORY_KEY "chat.openai.maxHistory"
#define CONFIG_CHAT_OPENAI_MAX_HISTORY_DEFAULT 10
#define CONFIG_CHAT_RANDOM_INTERVAL_MIN_KEY "chat.random.interval.min"
#define CONFIG_CHAT_RANDOM_INTERVAL_MIN_DEFAULT 60
#define CONFIG_CHAT_RANDOM_INTERVAL_MAX_KEY "chat.random.interval.min"
#define CONFIG_CHAT_RANDOM_INTERVAL_MAX_DEFAULT 120
#define CONFIG_CHAT_RANDOM_QUESTIONS_KEY "chat.random.questions"
#define CONFIG_CHAT_CLOCK_HOURS_KEY "chat.clock.hours"

#endif // !defined(APP_CONFIG_H)
