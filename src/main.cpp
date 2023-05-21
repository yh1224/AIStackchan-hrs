#include <Arduino.h>

#include "app/App.h"
#include "app/AppChat.h"
#include "app/AppFace.h"
#include "app/AppServer.h"
#include "app/AppVoice.h"
#include "lib/NvsSettings.h"

// NVS Namespace
static const char *NVS_NAMESPACE = "AIStackchan-hrs";

// NVS Key for settings
static const char *NVS_SETTINGS_KEY = "settings";

static std::unique_ptr<App> app;

void setup() {
    auto settings = std::make_shared<NvsSettings>(NVS_NAMESPACE, NVS_SETTINGS_KEY);
    auto voice = std::make_shared<AppVoice>(settings);
    auto face = std::make_shared<AppFace>(settings, voice);
    auto chat = std::make_shared<AppChat>(settings, voice, face);
    auto server = std::make_shared<AppServer>(settings, voice, face, chat);
    app = std::unique_ptr<App>(new App(settings, voice, face, chat, server));
    app->setup();
}

void loop() {
    app->loop();
}
