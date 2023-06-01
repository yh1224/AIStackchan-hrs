#include <Arduino.h>
#include <Avatar.h>
#include <ServoEasing.hpp>

#include "app/AppFace.h"
#include "app/AppVoice.h"
#include "app/config.h"

/// Avatar expression list
static const m5avatar::Expression EXPRESSIONS[] = {
        m5avatar::Expression::Neutral,
        m5avatar::Expression::Happy,
        m5avatar::Expression::Sleepy,
        m5avatar::Expression::Doubt,
        m5avatar::Expression::Sad,
        m5avatar::Expression::Angry,
};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

bool AppFace::init() {
    auto pin = _getServoPin();
    int servoPinX = pin.first;
    int servoPinY = pin.second;
    if (_isServoEnabled() && servoPinX != 0 && servoPinY != 0) {
        auto home = _getSwingHome();
        _homeX = home.first;
        _homeY = home.second;
        auto range = _getSwingRange();
        _rangeX = range.first;
        _rangeY = range.second;
        if (0 == _servoX.attach(
                servoPinX,
                _homeX,
                DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                DEFAULT_MICROSECONDS_FOR_180_DEGREE
        )) {
            Serial.println("ERROR: Failed to attach servo x.");
        }
        _servoX.setEasingType(EASE_QUADRATIC_IN_OUT);
        _servoX.setEaseTo(_homeX);

        if (0 == _servoY.attach(
                servoPinY,
                _homeY,
                DEFAULT_MICROSECONDS_FOR_0_DEGREE,
                DEFAULT_MICROSECONDS_FOR_180_DEGREE
        )) {
            Serial.println("ERROR: Failed to attach servo y.");
        }
        _servoY.setEasingType(EASE_QUADRATIC_IN_OUT);
        _servoY.setEaseTo(_homeY);

        setSpeedForAllServos(30);
        synchronizeAllServosStartAndWaitForAllServosToStop();
        _headSwing = true;
    }
    return true;
}

void AppFace::setup() {
    _avatar.init();
    _avatar.setSpeechFont(&fonts::efontJA_16);
}

void AppFace::start() {
    static auto face = this;
    _avatar.addTask([](void *args) { face->lipSync(args); }, "lipSync");
    if (_isServoEnabled()) {
        _avatar.addTask([](void *args) { face->servo(args); }, "servo");
    }
}

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

#pragma clang diagnostic pop

/**
 * Set text to speech bubble
 *
 * @param text text
 */
void AppFace::setText(const char *text) {
    _avatar.setSpeechText(text);
}

/**
 * Set face expression
 *
 * @param expression expression
 */
void AppFace::setExpression(m5avatar::Expression expression) {
    _avatar.setExpression(expression);
}

/**
 * Set face expression (by index)
 *
 * @param expressionIndex expression index
 */
bool AppFace::setExpressionIndex(int expressionIndex) {
    int numExpressions = sizeof(EXPRESSIONS) / sizeof(EXPRESSIONS[0]);
    if (expressionIndex < 0 || expressionIndex >= numExpressions) {
        Serial.printf("ERROR: Unknown expression: %d", expressionIndex);
        return false;
    }
    Serial.printf("Setting expression: %d\n", expressionIndex);
    _avatar.setExpression(EXPRESSIONS[expressionIndex]);
    return true;
}

/**
 * Swing ON/OFF
 */
void AppFace::toggleHeadSwing() {
    _headSwing = !_headSwing;
}

bool AppFace::_isServoEnabled() {
    return (bool) _settings->has("servo");
}

std::pair<int, int> AppFace::_getServoPin() {
    int servoPinX = _settings->get(CONFIG_SERVO_PIN_X_KEY);
    int servoPinY = _settings->get(CONFIG_SERVO_PIN_Y_KEY);
    return std::make_pair(servoPinX, servoPinY);
}

std::pair<int, int> AppFace::_getSwingHome() {
    int homeX = _settings->get(CONFIG_SWING_HOME_X_KEY) | CONFIG_SWING_HOME_X_DEFAULT;
    int homeY = _settings->get(CONFIG_SWING_HOME_Y_KEY) | CONFIG_SWING_HOME_Y_DEFAULT;
    return std::make_pair(homeX, homeY);
}

std::pair<int, int> AppFace::_getSwingRange() {
    int homeX = _settings->get(CONFIG_SWING_RANGE_X_KEY) | CONFIG_SWING_RANGE_X_DEFAULT;
    int homeY = _settings->get(CONFIG_SWING_RANGE_Y_KEY) | CONFIG_SWING_RANGE_Y_DEFAULT;
    return std::make_pair(homeX, homeY);
}
