#include <Arduino.h>
#include "led.h"
#include "config.h"

Led<Config::LED_PIN, Config::BLINK_TIME_MS> statusLed;
volatile bool buttonPressed = false;
SystemMode currentMode = SystemMode::BLINKING;

void IRAM_ATTR onButtonInterrupt()
{
    buttonPressed = true;
}

void handleButton()
{
    if (!buttonPressed)
        return;

    static unsigned long lastDebounceTime = 0;
    unsigned long now = millis();

    if (now - lastDebounceTime >= Config::DEBOUNCE_TIME_MS)
    {
        lastDebounceTime = now;

        if (digitalRead(Config::BUTTON_PIN) == LOW)
        {
            switch (currentMode)
            {
            case SystemMode::BLINKING:
                currentMode = SystemMode::ALWAYS_ON;
                break;
            case SystemMode::ALWAYS_ON:
                currentMode = SystemMode::ALWAYS_OFF;
                break;
            case SystemMode::ALWAYS_OFF:
                currentMode = SystemMode::BLINKING;
                break;
            }
        }
    }

    buttonPressed = false;
}

void updateLed()
{
    static unsigned long lastBlinkTime = 0;
    unsigned long now = millis();

    switch (currentMode)
    {
    case SystemMode::BLINKING:
        if (now - lastBlinkTime >= Config::BLINK_TIME_MS)
        {
            lastBlinkTime = now;
            statusLed.toggle();
        }
        break;

    case SystemMode::ALWAYS_ON:
        if (statusLed.getState() != LedState::ON)
        {
            statusLed.set(LedState::ON);
        }
        break;

    case SystemMode::ALWAYS_OFF:
        if (statusLed.getState() != LedState::OFF)
        {
            statusLed.set(LedState::OFF);
        }
        break;
    }
}

void measureLoopSpeed()
{
    static unsigned long counter = 0;
    static unsigned long startTime = micros();

    counter++;

    if (counter >= Config::MEASURE_ITERATIONS)
    {
        unsigned long now = micros();
        float avgTimeUs = static_cast<float>(now - startTime) / Config::MEASURE_ITERATIONS;

        Serial.print("Avg loop time: ");
        Serial.print(avgTimeUs);
        Serial.println(" us");

        counter = 0;
        startTime = micros();
    }
}

void setup()
{
    Serial.begin(115200);

    statusLed.init();

    pinMode(Config::BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Config::BUTTON_PIN), onButtonInterrupt, CHANGE);

    Serial.println("Система готова!");
}

void loop()
{
    handleButton();     // Обробка натискання кнопки
    updateLed();        // Оновлення стану діода
    measureLoopSpeed(); // Замір швидкості циклу
}