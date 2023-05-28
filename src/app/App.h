#if !defined(APP_APP_H)
#define APP_APP_H

#include <M5Unified.h>
#include <utility>

#include "app/AppChat.h"
#include "app/AppFace.h"
#include "app/AppServer.h"
#include "app/AppVoice.h"
#include "lib/NvsSettings.h"

class App {
public:
    explicit App(
            std::shared_ptr<NvsSettings> settings,
            std::shared_ptr<AppVoice> voice,
            std::shared_ptr<AppFace> face,
            std::shared_ptr<AppChat> chat,
            std::shared_ptr<AppServer> server
    ) : _settings(std::move(settings)),
        _voice(std::move(voice)),
        _face(std::move(face)),
        _chat(std::move(chat)),
        _server(std::move(server)) {};

    void setup();

    void loop();

private:
    std::shared_ptr<NvsSettings> _settings;
    std::shared_ptr<AppVoice> _voice;
    std::shared_ptr<AppFace> _face;
    std::shared_ptr<AppChat> _chat;
    std::shared_ptr<AppServer> _server;

    bool _isServoEnabled();

    void _onTapCenter();

    void _onButtonA();

    void _onButtonB();

    void _onButtonC();
};

#endif // !defined(APP_APP_H)
