#include <Arduino.h>
#if !defined(WITHOUT_AVATAR)
#include <Avatar.h>
#include <ServoEasing.hpp>
#endif // !defined(WITHOUT_AVATAR)

#include "app/AppFace.h"
#include "app/AppVoice.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

bool AppFace::init() {
#if !defined(WITHOUT_AVATAR)
    auto pin = _settings->getServoPin();
    int servoPinX = pin.first;
    int servoPinY = pin.second;
    if (_settings->isServoEnabled() && servoPinX != 0 && servoPinY != 0) {
        auto home = _settings->getSwingHome();
        _homeX = home.first;
        _homeY = home.second;
        auto range = _settings->getSwingRange();
        _rangeX = range.first;
        _rangeY = range.second;
        auto retX = _servoX.attach(
                servoPinX,
                _homeX,
                DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                DEFAULT_MICROSECONDS_FOR_180_DEGREE
        );
        if (retX == 0 || retX == INVALID_SERVO) {
            Serial.println("ERROR: Failed to attach servo x.");
        }
        _servoX.setEasingType(EASE_QUADRATIC_IN_OUT);
        _servoX.setEaseTo(_homeX);

        auto retY = _servoY.attach(
                servoPinY,
                _homeY,
                DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                DEFAULT_MICROSECONDS_FOR_180_DEGREE
        );
        if (retY == 0 || retY == INVALID_SERVO) {
            Serial.println("ERROR: Failed to attach servo y.");
        }
        _servoY.setEasingType(EASE_QUADRATIC_IN_OUT);
        _servoY.setEaseTo(_homeY);

        setSpeedForAllServos(30);
        synchronizeAllServosStartAndWaitForAllServosToStop();
        _headSwing = true;
    }
#endif // !defined(WITHOUT_AVATAR)
    return true;
}

void AppFace::setup() {
#if !defined(WITHOUT_AVATAR)
    _avatar.init();
    _avatar.setBatteryIcon(true);
    _avatar.setSpeechFont(&fonts::efontJA_16);
#endif // !defined(WITHOUT_AVATAR)
}

void AppFace::start() {
#if !defined(WITHOUT_AVATAR)
    static auto face = this;
    _avatar.addTask([](void *args) { face->lipSync(args); }, "lipSync");
    if (_settings->isServoEnabled()) {
        _avatar.addTask([](void *args) { face->servo(args); }, "servo");
    }
#endif // !defined(WITHOUT_AVATAR)
}

void AppFace::loop() {
#if !defined(WITHOUT_AVATAR)
    if (_lastBatteryStatus == 0 || millis() - _lastBatteryStatus > 5000) {
        _avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
        _lastBatteryStatus = millis();
    }
#endif // !defined(WITHOUT_AVATAR)
}

#if !defined(WITHOUT_AVATAR)
/**
 * Task to open mouth to match the voice.
 */
void AppFace::lipSync(void *args) {
    if (((m5avatar::DriveContext *) args)->getAvatar() != &_avatar) return;

    while (true) {
        _avatar.setMouthOpenRatio(_voice->getAudioLevel());
        delay(50);
    }
}

/**
 * Task to swing head to the gaze.
 */
void AppFace::servo(void *args) {
    if (((m5avatar::DriveContext *) args)->getAvatar() != &_avatar) return;

    while (true) {
        if (!_headSwing) {
            // Reset to home position
            _servoX.setEaseTo(_homeX);
            _servoY.setEaseTo(_homeY);
        } else if (!_voice->isPlaying()) {
            // Swing head to the gaze
            float gazeH, gazeV;
            _avatar.getGaze(&gazeV, &gazeH);
            auto degreeX = (_homeX + (int) ((float) _rangeX / 2 * gazeH) + 360) % 360;
            auto degreeY = (_homeY + (int) ((float) _rangeY / 2 * gazeV) + 360) % 360;
            //Serial.printf("gaze (%.2f, %.2f) -> degree (%d, %d)\n", gazeH, gazeV, degreeX, degreeY);
            _servoX.setEaseTo(degreeX);
            _servoY.setEaseTo(degreeY);
        }
        synchronizeAllServosStartAndWaitForAllServosToStop();
        delay(50);
    }
}
#endif // !defined(WITHOUT_AVATAR)

#pragma clang diagnostic pop

/**
 * Set text to speech bubble
 *
 * @param text text
 */
void AppFace::setText(const char *text) {
#if !defined(WITHOUT_AVATAR)
    _avatar.setSpeechText(text);
#endif // !defined(WITHOUT_AVATAR)
}

/**
 * Set face expression
 *
 * @param expression expression
 */
bool AppFace::setExpression(Expression expression) {
#if !defined(WITHOUT_AVATAR)
    static const m5avatar::Expression EXPRESSIONS[] = {
            m5avatar::Expression::Neutral,
            m5avatar::Expression::Happy,
            m5avatar::Expression::Sleepy,
            m5avatar::Expression::Doubt,
            m5avatar::Expression::Sad,
            m5avatar::Expression::Angry,
    };
    int numExpressions = sizeof(EXPRESSIONS) / sizeof(EXPRESSIONS[0]);
    if (expression >= numExpressions) {
        Serial.printf("ERROR: Unknown expression: %d", expression);
        return false;
    }
    Serial.printf("Setting expression: %d\n", expression);
    _avatar.setExpression(EXPRESSIONS[expression]);
#endif // !defined(WITHOUT_AVATAR)
    return true;
}

/**
 * Swing ON/OFF
 */
void AppFace::toggleHeadSwing() {
#if !defined(WITHOUT_AVATAR)
    _headSwing = !_headSwing;
#endif // !defined(WITHOUT_AVATAR)
}
