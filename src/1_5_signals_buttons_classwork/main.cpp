#include <Arduino.h>

#define LED_PIN 9
#define BTN_PIN 4
#define DEBOUNCE_MS 25

bool raw = HIGH;
bool stable = HIGH;
bool PrevStable = HIGH;
unsigned long LastChanges = 0;
bool LedON = false;

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, LOW);
    Serial.println("Start");
}

void loop()
{

    const int reading = digitalRead(BTN_PIN);

    if (reading != raw)
    {
        LastChanges = millis();
        raw = reading;
        }

    if ((millis() - LastChanges) > DEBOUNCE_MS && raw != stable)
    {
        PrevStable = stable;
        stable = raw;
        if (stable == LOW && PrevStable == HIGH)
        {
            LedON = !LedON;
            digitalWrite(LED_PIN, LedON ? HIGH : LOW);
            Serial.printf("Toggle LED: %s\n", LedON ? "ON" : "OFF");
        }
    }
}