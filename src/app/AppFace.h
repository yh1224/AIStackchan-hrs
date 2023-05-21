#if !defined(APP_FACE_H)
#define APP_FACE_H

#include <Avatar.h>

#define SUPPRESS_HPP_WARNING

#include <ServoEasing.h>

#undef SUPPRESS_HPP_WARNING

#include "app/AppVoice.h"
#include "lib/NvsSettings.h"

class AppFace {
public:
    explicit AppFace(
            std::shared_ptr<NvsSettings> settings,
            std::shared_ptr<AppVoice> voice
    ) : _settings(std::move(settings)),
        _voice(std::move(voice)) {};

    bool init();

    void setup();

    void start();

    void lipSync(void *args);

    void servo(void *args);

    void setText(const char *text);

    void setExpression(m5avatar::Expression expression);

    bool setExpressionIndex(int expressionIndex);

    void toggleHeadSwing();

private:
    std::shared_ptr<NvsSettings> _settings;
    std::shared_ptr<AppVoice> _voice;

    /// M5Stack-Avatar https://github.com/meganetaaan/m5stack-avatar
    m5avatar::Avatar _avatar;

    /// servo to swing head
    ServoEasing _servoX, _servoY;

    /// head swing mode
    bool _headSwing;

    bool _isServoEnabled();

    std::pair<int, int> _getServoPin();
};

#endif // !defined(APP_FACE_H)
