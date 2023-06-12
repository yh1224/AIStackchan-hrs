#include <Arduino.h>
#include <M5Unified.h>

#include "app/App.h"
#include "app/config.h"
#include "lib/network.h"
#include "lib/sdcard.h"

/// File path for settings
static const char *APP_SETTINGS_SD_PATH = "/settings.json";

[[noreturn]] void halt() {
    while (true) { delay(1000); }
}

class Box {
public:
    int x, y, w, h;

    Box(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

    bool contain(int x1, int y1) const {
        return this->x <= x1 && x1 < (this->x + this->w) && this->y <= y1 && y1 < (this->y + this->h);
    }
};

void App::setup() {
    auto cfg = m5::M5Unified::config();
    cfg.external_spk = true;
    M5.begin(cfg);

    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);

    // Load settings
    auto settings = sdLoadString(APP_SETTINGS_SD_PATH);
    if (settings != nullptr) {
        if (!_settings->load(*settings)) {
            M5.Display.println("ERROR: Invalid settings.");
            halt();
        }
        M5.Display.println("Settings loaded.");
    } else {
        _settings->load();
    }

    // Initialize
    if (!_voice->init() || !_face->init()) {
        Serial.println("ERROR: Failed to initialize.");
        delay(5000);
        halt();
    }

    // Connect
    const char *wifiSsid = _settings->get(CONFIG_NETWORK_WIFI_SSID_KEY);
    const char *wifiPass = _settings->get(CONFIG_NETWORK_WIFI_PASS_KEY);
    if (!connectNetwork(wifiSsid, wifiPass)) {
        Serial.println("ERROR: Failed to connect network. Rebooting...");
        delay(5000);
        ESP.restart();
        halt();
    }

    // Setup mDNS
    const char *mDnsHostname = _settings->get(CONFIG_NETWORK_HOSTNAME_KEY);
    if (mDnsHostname != nullptr) {
        Serial.printf("Setting up mDNS hostname: %s\n", mDnsHostname);
        setMDnsHostname(mDnsHostname);
    }

    // Synchronize time
    const char *tz = _settings->get(CONFIG_TIME_ZONE_KEY) | CONFIG_TIME_ZONE_DEFAULT;
    const char *ntpServer = _settings->get(CONFIG_TIME_NTP_SERVER_KEY) | CONFIG_TIME_NTP_SERVER_DEFAULT;
    if (tz != nullptr && ntpServer != nullptr) {
        Serial.printf("Synchronizing time: %s (%s)\n", ntpServer, tz);
        syncTime(tz, ntpServer);
    }

    delay(3000);
    M5.Display.clear();

    // Setup
    _voice->setup();
    _face->setup();
    _chat->setup();
    _server->setup();

    // Start
    _voice->start();
    _face->start();
    _chat->start();
}

static const Box boxCenter{120, 80, 80, 80};
static const Box boxButtonA{0, 200, 106, 40};
static const Box boxButtonB{106, 200, 108, 40};
static const Box boxButtonC{214, 200, 106, 40};

void App::loop() {
    M5.update();
    if (M5.Touch.getCount()) {
        auto t = M5.Touch.getDetail();
        if (t.wasPressed()) {
            if (boxCenter.contain(t.x, t.y)) _onTapCenter();
            if (boxButtonA.contain(t.x, t.y)) _onButtonA();
            if (boxButtonB.contain(t.x, t.y)) _onButtonB();
            if (boxButtonC.contain(t.x, t.y)) _onButtonC();
        }
    }
    if (M5.BtnA.wasPressed()) _onButtonA();
    if (M5.BtnB.wasPressed()) _onButtonB();
    if (M5.BtnC.wasPressed()) _onButtonC();

    _server->loop();
    _face->loop();

    delay(50);
}

bool App::_isServoEnabled() {
    return _settings->has("servo");
}

/**
 * Tap Center: Swing ON/OFF
 */
void App::_onTapCenter() {
    if (_isServoEnabled()) {
        M5.Speaker.tone(1000, 100);
        _face->toggleHeadSwing();
    }
}

/**
 * Button A: Random speak mode ON/OFF
 */
void App::_onButtonA() {
    M5.Speaker.tone(1000, 100);
    _chat->toggleRandomSpeakMode();
}

void App::_onButtonB() {}

/**
 * Button C: Speak current time
 */
void App::_onButtonC() {
    M5.Speaker.tone(1000, 100);
    _chat->speakCurrentTime();
}
