#pragma once
#include <Arduino.h>

enum class LedState
{
    OFF,
    ON
};

enum class SystemMode
{
    BLINKING,
    ALWAYS_ON,
    ALWAYS_OFF
};

// Конфігурація проєкту
struct Config
{
    static constexpr uint8_t LED_PIN = 4;
    static constexpr uint8_t BUTTON_PIN = 2;
    static constexpr uint16_t BLINK_TIME_MS = 500;
    static constexpr uint32_t DEBOUNCE_TIME_MS = 50;
    static constexpr uint32_t MEASURE_ITERATIONS = 1000;
};