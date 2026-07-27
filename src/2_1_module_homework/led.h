#pragma once
#include <Arduino.h>

enum class LedState
{
    OFF,
    ON
};

template <uint8_t Pin, uint8_t BlinkTime = 100>
class Led
{
public:
    static constexpr uint8_t _pin = Pin;
    static constexpr uint8_t _blinkTime = BlinkTime;

private:
    LedState _state;

public:
    Led() : _state(LedState::OFF) {}

    void init() const
    {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }

    void set(LedState state)
    {
        _state = state;
        digitalWrite(_pin, (_state == LedState::ON) ? HIGH : LOW);
    }

    LedState getState() const
    {
        return _state;
    }
};