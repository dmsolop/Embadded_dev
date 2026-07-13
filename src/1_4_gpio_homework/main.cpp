#include <Arduino.h>

#define BUTTON_PIN 15
#define LED_RED_PIN 4
#define LED_BLUE_PIN 16
#define BUTTON_INTERNAL_BOOT 0
#define BLINKING_DELAY_SLOW 1000
#define BLINKING_DELAY_FAST 200
static unsigned long lastBlinkTime = 0;
static bool ledState = false;

enum SystemMode
{
    MOD_BLINKING_OFF,
    MOD_BLINKING_FAST,
    MOD_BLINKING_SLOW,
    MOD_BLINKING_ONE_BY_ONE_SLOW,
    MOD_BLINKING_ONE_BY_ONE_FAST
};

enum ButtonEvent
{
    EVENT_NONE,
    EVENT_BTN1_CLICK,
    EVENT_BTN2_CLICK,
    EVENT_BTN1_HOLD,
    EVENT_BTN2_HOLD,
    EVENT_BOTH_CLICK
};

SystemMode currentMode = MOD_BLINKING_OFF;
ButtonEvent currentEvent = EVENT_NONE;

ButtonEvent checkButtons();
void blinkingLeds(bool, bool, int);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_INTERNAL_BOOT, INPUT_PULLUP);

    digitalWrite(BUILTIN_LED, 0);
    Serial.println("Піни та кнопки налаштовані");
}

void loop()
{

    currentEvent = checkButtons();

    switch (currentEvent)
    {
    case EVENT_BTN1_CLICK:
        currentMode = MOD_BLINKING_SLOW;
        Serial.println("Увімкнено режим повільного блимання");
        break;
    case EVENT_BTN2_CLICK:
        currentMode = MOD_BLINKING_FAST;
        Serial.println("Увімкнено режим швидкого блимання");
        break;
    case EVENT_BTN1_HOLD:
        currentMode = MOD_BLINKING_ONE_BY_ONE_SLOW;
        Serial.println("Увімкнено режим повільного послідовного блимання");
        break;
    case EVENT_BTN2_HOLD:
        currentMode = MOD_BLINKING_ONE_BY_ONE_FAST;
        Serial.println("Увімкнено режим швидкого послідовного блимання");
        break;
    case EVENT_BOTH_CLICK:
        currentMode = MOD_BLINKING_OFF;
        Serial.println("Всі режими вимкнені");
        break;
    case EVENT_NONE:
    default:
        break;
    }

    switch (currentMode)
    {
    case MOD_BLINKING_OFF:
        blinkingLeds(false, true, BLINKING_DELAY_FAST);
        break;
    case MOD_BLINKING_SLOW:
        blinkingLeds(true, true, BLINKING_DELAY_SLOW);
        break;
    case MOD_BLINKING_FAST:
        blinkingLeds(true, true, BLINKING_DELAY_FAST);
        break;
    case MOD_BLINKING_ONE_BY_ONE_SLOW:
        blinkingLeds(true, false, BLINKING_DELAY_SLOW);
        break;
    case MOD_BLINKING_ONE_BY_ONE_FAST:
        blinkingLeds(true, false, BLINKING_DELAY_FAST);
        break;
    default:
        break;
    }
}

ButtonEvent checkButtons()
{
    bool btn1Pressed = (digitalRead(BUTTON_PIN) == LOW);
    bool btn2Pressed = (digitalRead(BUTTON_INTERNAL_BOOT) == LOW);

    static unsigned long btn1Time = 0;
    static unsigned long btn2Time = 0;

    static bool btn1HeldHandled = false;
    static bool btn2HeldHandled = false;

    static bool btn1LastState = false;
    static bool btn2LastState = false;

    ButtonEvent result = EVENT_NONE;

    if (btn1Pressed && btn2Pressed)
    {
        btn1HeldHandled = true;
        btn2HeldHandled = true;
        if (!btn1LastState || !btn2LastState)
        {
            btn1LastState = btn1Pressed;
            btn2LastState = btn2Pressed;
            return EVENT_BOTH_CLICK;
        }
        return EVENT_NONE;
    }

    if (btn1Pressed)
    {
        if (!btn1LastState)
        {
            btn1Time = millis();
            btn1HeldHandled = false;
        }
        if ((millis() - btn1Time >= 1000) && !btn1HeldHandled)
        {
            btn1HeldHandled = true;
            result = EVENT_BTN1_HOLD;
        }
    }
    else
    {
        if (btn1LastState)
        {
            if (!btn1HeldHandled && (millis() - btn1Time > 50))
            {
                result = EVENT_BTN1_CLICK;
            }
        }
    }

    if (btn2Pressed)
    {
        if (!btn2LastState)
        {
            btn2Time = millis();
            btn2HeldHandled = false;
        }
        if ((millis() - btn2Time >= 1000) && !btn2HeldHandled)
        {
            btn2HeldHandled = true;
            result = EVENT_BTN2_HOLD;
        }
    }
    else
    {
        if (btn2LastState)
        {
            if (!btn2HeldHandled && (millis() - btn2Time > 50))
            {
                result = EVENT_BTN2_CLICK;
            }
        }
    }

    btn1LastState = btn1Pressed;
    btn2LastState = btn2Pressed;

    return result;
}

void blinkingLeds(bool isBlink, bool isBlinkSinc, int blinkDelay)
{
    if (!isBlink)
    {
        digitalWrite(LED_RED_PIN, LOW);
        digitalWrite(LED_BLUE_PIN, LOW);
        return;
    }

    if (millis() - lastBlinkTime >= (unsigned long)blinkDelay)
    {
        lastBlinkTime = millis();
        ledState = !ledState;
    }

    if (isBlinkSinc)
    {
        digitalWrite(LED_RED_PIN, ledState);
        digitalWrite(LED_BLUE_PIN, ledState);
    }
    else
    {
        digitalWrite(LED_RED_PIN, ledState);
        digitalWrite(LED_BLUE_PIN, !ledState);
    }
}