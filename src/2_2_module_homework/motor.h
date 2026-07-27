#pragma once
#include <Arduino.h>
#include "config.h"

enum class MotorMode
{
    OFF,
    RAMP_UP,
    HOLD,
    RAMP_DOWN
};

template <uint8_t Pin, uint32_t Freq = 5000, uint8_t Res = 8>
class Motor
{
private:
    MotorMode _mode = MotorMode::OFF;
    MotorMode _lastRampDirection = MotorMode::RAMP_DOWN;
    float _currentDuty = 0;
    unsigned long _lastRampTime = 0;

public:
    void init()
    {
        ledcAttach(Pin, Freq, Res);
        setDuty(0);
    }

    void setMode(MotorMode mode)
    {
        if (_mode == MotorMode::RAMP_UP || _mode == MotorMode::RAMP_DOWN)
        {
            _lastRampDirection = _mode;
        }

        _mode = mode;

        if (_mode == MotorMode::OFF)
        {
            _currentDuty = 0;
            setDuty(0);
        }
    }

    MotorMode getMode() const { return _mode; }
    MotorMode getLastRampDirection() const { return _lastRampDirection; }
    uint8_t getDuty() const { return static_cast<uint8_t>(_currentDuty); }

    void update()
    {
        if (_mode == MotorMode::OFF || _mode == MotorMode::HOLD)
        {
            return;
        }

        unsigned long now = millis();
        if (now - _lastRampTime >= Config::RAMP_STEP_MS)
        {
            _lastRampTime = now;

            if (_mode == MotorMode::RAMP_UP)
            {
                if (_currentDuty < Config::MIN_PWM)
                {
                    _currentDuty = Config::MIN_PWM;
                }
                else
                {
                    _currentDuty++;
                }

                if (_currentDuty >= Config::MAX_PWM)
                {
                    _currentDuty = Config::MAX_PWM;
                    setMode(MotorMode::HOLD);
                }
            }
            else if (_mode == MotorMode::RAMP_DOWN)
            {
                if (_currentDuty > Config::MIN_PWM)
                {
                    _currentDuty--;
                }

                if (_currentDuty <= Config::MIN_PWM)
                {
                    _currentDuty = Config::MIN_PWM;
                    setMode(MotorMode::HOLD);
                }
            }

            setDuty(static_cast<uint8_t>(_currentDuty));
        }
    }

private:
    void setDuty(uint8_t duty)
    {
        ledcWrite(Pin, duty);
    }
};