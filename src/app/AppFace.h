#if !defined(APP_FACE_H)
#define APP_FACE_H

#include <Avatar.h>
#define SUPPRESS_HPP_WARNING
#include <ServoEasing.h>
#undef SUPPRESS_HPP_WARNING

#include "app/AppSettings.h"
#include "app/AppVoice.h"

typedef enum {
    Neutral = 0,
    Happy,
    Sleepy,
    Doubt,
    Sad,
    Angry,
} Expression;

class AppFace {
public:
    explicit AppFace(
            std::shared_ptr<AppSettings> settings,
            std::shared_ptr<AppVoice> voice
    ) : _settings(std::move(settings)),
        _voice(std::move(voice)) {};

    bool init();

    void setup();

    void start();

    void loop();

    void lipSync(void *args);

    void servo(void *args);

    void setText(const char *text);

    bool setExpression(Expression expression);

    void toggleHeadSwing();

private:
    std::shared_ptr<AppSettings> _settings;
    std::shared_ptr<AppVoice> _voice;

    /// M5Stack-Avatar https://github.com/meganetaaan/m5stack-avatar
    m5avatar::Avatar _avatar;

    /// servo to swing head
    ServoEasing _servoX, _servoY;

    /// Swing parameters
    int _homeX, _homeY, _rangeX, _rangeY;

    /// head swing mode
    bool _headSwing;
};

#endif // !defined(APP_FACE_H)
