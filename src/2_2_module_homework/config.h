#pragma once
#include <Arduino.h>

struct Config
{

    static constexpr uint8_t MOTOR_PIN = 4;
    static constexpr uint8_t BUTTON_PIN = 2;
    static constexpr uint8_t PWM_CHANNEL = 0;

    static constexpr uint32_t PWM_FREQ = 5000; // 5 кГц (без свисту мотора)
    static constexpr uint8_t PWM_RESOLUTION = 8;

    static constexpr uint8_t MIN_PWM = 30;
    static constexpr uint8_t MAX_PWM = 255;
    static constexpr uint16_t RAMP_STEP_MS = 15;

    static constexpr uint16_t LONG_PRESS_MS = 800;
    static constexpr uint16_t DEBOUNCE_MS = 50;
};