#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 4
#define LED_DELAY 5

void setup()
{
    Serial.begin(115200);
    delay(500);

    // neopixelWrite(BUILTIN_LED, 0, 0, 0);
    pinMode(LED_PIN, OUTPUT);
    // pinMode(BUILTIN_LED, OUTPUT);
    analogWrite(LED_PIN, 0);
    // digitalWrite(BUILTIN_LED, 0);
}

void loop()
{
    // neopixelWrite(BUILTIN_LED, 128, 0, 0);
    // delay(LED_DELAY);
    // neopixelWrite(BUILTIN_LED, 0, 128, 0);
    // delay(LED_DELAY);
    // neopixelWrite(BUILTIN_LED, 0, 0, 128);
    // delay(LED_DELAY);
    // neopixelWrite(BUILTIN_LED, 128, 128, 0);
    // delay(LED_DELAY);
    // neopixelWrite(BUILTIN_LED, 0, 0, 0);
    // delay(LED_DELAY);
    for (int i = 0; i < 255; i++)
    {
        analogWrite(LED_PIN, i);
        delay(LED_DELAY);
    }
    for (int i = 255; i > 0; i--)
    {
        analogWrite(LED_PIN, i);
        delay(LED_DELAY);
    }
}