# AIStackchan-hrs

## Summary

This is an alternative implementation of AI Stack-chan. Super thanks to [meganetaaan](https://github.com/meganetaaan) the originator of Stack-chan and [robo8080](https://github.com/robo8080) the originator of AI Stack-chan.

## Features

- The following speech services can be used
  - [Google Translate](https://translate.google.com/) Text-to-Speech API (no API Key reqiured) - *unofficial?*
  - [VoiceText Web API](https://cloud.voicetext.jp/webapi) (API Key required) - *free registration suspended for now*
- API
  - Speak API
  - Chat API (OpenAI API Key required)
- Speak randomly
  - send a question to the ChatGPT and speak answer periodically
  - start/stop by button A
- Speak clock
  - on every hour
  - current time by button C

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
  "swing": {
    "home": {"x": 90, "y": 80},
    "range": {"x": 30, "y": 20}
  },
  "voice": {
    "lang": "ja",
    "volume": 200,
    "service": "voicetext",
    "voicetext": {
      "apiKey": "VoiceText API Key",
      "params": "speaker=hikari&speed=120&pitch=130&emotion=happiness"
    }
  },
  "chat": {
    "openai": {
      "apiKey": "OpenAI API Key",
      "model": "gpt-3.5-turbo-0613",
      "stream": true,
      "roles": [
        "Answer in Japanese.",
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

### Network settings *(reboot required)*

- `network.wifi.ssid` [string] : Wi-Fi SSID
- `network.wifi.pass` [string] : Wi-Fi passphrase
- `network.hostname` [string] : mDNS hostname
- `time.zone` [string] : Time zone (Default: `"JST-9"`)
- `time.ntpServer` [string] : NTP Server (Default: `"ntp.nict.jp"`)

### Servo/Swing settings *(reboot required)*

- `servo.pin.x`, `servo.pin.y` [int] : Pin number for servo (Required to swing head)
  - M5Stack CoreS3 - Port A : `{"x": 1, "y": 2}`
  - M5Stack CoreS3 - Port B : `{"x": 8, "y": 9}`
  - M5Stack CoreS3 - Port C : `{"x": 18, "y": 17}`
  - M5Stack Core2 - Port A : `{"x": 33, "y": 32}`
  - M5Stack Core2 - Port C : `{"x": 13, "y": 14}`
  - M5Stack Core/Fire : `{"x": 21, "y": 22}`
- `swing.home.x`, `swing.home.y` [int] : Home position in degrees (Default: `{"x": 90, "y": 80}`)
- `swing.range.x`, `swing.range.y` [int] : Swing range in degrees (Default: `{"x": 30, "y": 20}`)

### Voice settings

- `voice.lang` [string] : Speech language for Google Translate TTS (Default: `"ja"`)
- `voice.volume` [int] : Speech volume (Default: `200`)
- `voice.service` [string] : Speech service (Default: `"google-translate-tts"`)
  - `"google-translate-tts"` : [Google Translate](https://translate.google.com/) Text-to-Speech API
  - `"voicetext"` : [VoiceText Web API](https://cloud.voicetext.jp/webapi)
- `voice.voicetext.apiKey` [string] : VoiceText: API Key (Required to speech by VoiceText)
- `voice.voicetext.params` [string] : VoiceText: parameters (Default: `"speaker=hikari&speed=120&pitch=130&emotion=happiness"`)

### Chat settings

- `chat.openai.apiKey` [string] : [OpenAI](https://platform.openai.com/) API Key (Required for chat)
- `chat.openai.model` [string] : ChatGPT model (Default: `gpt-3.5-turbo-0613`)
- `chat.openai.stream` [boolean] : Use stream or not (Default: `true`) 
- `chat.openai.roles` [string[]] : Roles for ChatGPT
- `chat.openai.maxHistory` [int] : Send talk history (Default: `10`)
- `chat.random.interval.min`-`random.interval.max` [int] : Random speech interval (Default: `60`-`120`)
- `chat.random.questions` [string[]] : Questions to ChatGPT for random speech
- `chat.clock.hours` [int[]] : Speech hours list

## API

### Speak API

- Path: /speech
- Parameters
  - text : Text to speak

Example

```shell
curl -X POST "http://(Stack-chan's IP address)/speech" \
    -d "text=Hello"
```

### Chat API

- Path: /chat
- Parameters
  - message : question

```shell
curl -X POST "http://(Stack-chan's IP address)/chat" \
    -d "message=Say something"
```
