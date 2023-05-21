# AIStackchan-hrs

## Summary

This is an altenative implementation of AI Stack-chan. Super thanks to [meganetaaan](https://github.com/meganetaaan) the originator of Stack-chan and [robo8080](https://github.com/robo8080) the originator of AI Stack-chan.

## Settings

Store the file named settings.json on SD card in advance (OPTIONAL). Once the stack-chan is running and connected to network, you can update settings via API.

```shell
curl -X POST "http://(Stack-chan's IP address)/settings" \
    -H "Content-Type: application/json" \
    -d @settings.json
```

You can also update some settings indivisually.

```shell
curl -X POST "http://(Stack-chan's IP address)/settings" \
    -d "voice.lang=en_US" \
    -d "voice.service=google-translate-tts"
```

Here is the example of settings.

```json
{
  "network": {
    "wifi": {
      "ssid": "SSID",
      "pass": "PASSPHRASE"
    },
    "hostname": "stackchan"
  },
  "time": {
    "zone": "JST-9",
    "ntpServer": "ntp.nict.jp"
  },
  "servo": {
    "pin": {"x": 13, "y": 14}
  },
  "voice": {
    "lang": "ja_JP",
    "volume": 200,
    "voicetext": {
      "apiKey": "VoiceText API Key",
      "params": "speaker=hikari&speed=120&pitch=130&emotion=happiness"
    }
  },
  "chat": {
    "openai": {
      "apiKey": "OpenAI API Key",
      "roles": [
        "あなたはスーパーカワイイロボット「ｽﾀｯｸﾁｬﾝ」となり人々の心を癒やすことが使命です。"
      ],
      "maxHistory": 10
    },
    "random": {
      "interval": {"min": 60, "max": 120},
      "questions": [
        "何かためになることを教えて",
        "面白い話をして",
        "ジョークを言って",
        "詩を書いて"
      ]
    },
    "clock": {
      "hours": [7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23]
    }
  }
}
```

### Network settings

- `network.wifi.ssid` [string] : Wi-Fi SSID
- `network.wifi.pass` [string] : Wi-Fi passphrase
- `network.hostname` [string] : mDNS hostname
- `time.zone` [string] : Time zone (Default: `"JST-9"`)
- `time.ntpServer` [string] : NTP Server (Default: `"ntp.nict.jp"`)

### Servo settings

- `servo.pin.x`, `servo.pin.y` [int] : Pin number for servo (Required to swing head)
  - M5Stack Core2 - Port A : `{"x": 33, "y": 32}`
  - M5Stack Core2 - Port C : `{"x": 13, "y": 14}`
  - M5Stack Core/Fire : `{"x": 21, "y": 22}`

### Voice settings

- `voice.lang` [string] : Speech language for Google TTS (Default: `"ja_JP"`)
- `voice.volume` [int] : Speech volume (Default: `200`)
- `voice.service` [string] : Speech service: `google-translate-tts`/`voicetext` (Default: `google-translate-tts`)
- `voice.voicetext.apiKey` [string] : [VoiceText](https://cloud.voicetext.jp/webapi) API Key (Required to speech by VoiceText)
- `voice.voicetext.params` [string] : Voice parameters for VoiceText API (Default: `"speaker=hikari&speed=120&pitch=130&emotion=happiness"`)

### Chat settings

- `chat.openai.apiKey` [string] : [OpenAI](https://platform.openai.com/) API Key (Required for chat)
- `chat.openai.roles` [string[]] : Roles for ChatGPT
- `chat.openai.maxHistory` [int] : Send talk history (Default: `10`)
- `chat.random.interval.min`-`random.interval.max` [int] : Random speech interval (Default: `60`-`120`)
- `chat.random.questions` [string[]] : Questions to ChatGPT for random speech
- `chat.clock.hours` [int[]] : Speech hours list
