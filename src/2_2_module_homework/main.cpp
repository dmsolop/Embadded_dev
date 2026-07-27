#include <Arduino.h>
#include "config.h"
#include "motor.h"

Motor<Config::MOTOR_PIN, Config::PWM_CHANNEL, Config::PWM_FREQ, Config::PWM_RESOLUTION> motor;

volatile bool buttonStateChanged = false;

void IRAM_ATTR onButtonChange()
{
    buttonStateChanged = true;
}

void processShortPress()
{
    switch (motor.getMode())
    {
    case MotorMode::OFF:
        motor.setMode(MotorMode::RAMP_UP);
        Serial.println("Режим: Наростання (RAMP_UP)");
        break;

    case MotorMode::RAMP_UP:
    case MotorMode::RAMP_DOWN:
        motor.setMode(MotorMode::HOLD);
        Serial.print("Режим: Фіксація (HOLD) на ");
        Serial.print(motor.getDuty());
        Serial.println(" PWM");
        break;

    case MotorMode::HOLD:
        if (motor.getLastRampDirection() == MotorMode::RAMP_UP)
        {
            motor.setMode(MotorMode::RAMP_DOWN);
            Serial.println("Режим: Зменшення (RAMP_DOWN)");
        }
        else
        {
            motor.setMode(MotorMode::RAMP_UP);
            Serial.println("Режим: Наростання (RAMP_UP)");
        }
        break;
    }
}

void handleButton()
{
    static unsigned long pressStartTime = 0;
    static bool isKeyDown = false;
    static bool longPressHandled = false;

    unsigned long now = millis();

    if (buttonStateChanged)
    {
        buttonStateChanged = false;

        bool rawState = (digitalRead(Config::BUTTON_PIN) == LOW);

        if (rawState && !isKeyDown)
        {
            pressStartTime = now;
            isKeyDown = true;
            longPressHandled = false;
        }
        else if (!rawState && isKeyDown)
        {
            if (!longPressHandled && (now - pressStartTime >= Config::DEBOUNCE_MS))
            {
                processShortPress();
            }
            isKeyDown = false;
        }
    }

    if (isKeyDown && !longPressHandled)
    {
        if (now - pressStartTime >= Config::LONG_PRESS_MS)
        {
            longPressHandled = true;
            motor.setMode(MotorMode::OFF);
            Serial.println("Подія: ДОВГЕ НАТИСКАННЯ -> ВИМКНЕНО (OFF)");
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(800);

    motor.init();

    pinMode(Config::BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Config::BUTTON_PIN), onButtonChange, CHANGE);

    Serial.println("Система керування мотором готова!");
}

void loop()
{
    handleButton(); // Перевірка подій кнопки
    motor.update(); // Плавне оновлення обертів за таймером
}