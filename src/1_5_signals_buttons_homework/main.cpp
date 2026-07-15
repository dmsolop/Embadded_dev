#include <Arduino.h>

#define BUTTON_PIN 3

volatile unsigned long lastInterruptTime = 0;
volatile unsigned int physicalPresses = 0;
volatile unsigned int bounceCount = 0;

volatile bool activePress = false;
volatile bool pressSettled = true;

void IRAM_ATTR buttonISR()
{
    unsigned long currentTime = millis();

    if (currentTime - lastInterruptTime > 200)
    {
        physicalPresses++;
        bounceCount = 0;
        activePress = true;
        pressSettled = false;
    }
    else
    {
        if (activePress)
        {
            bounceCount++;
        }
    }

    lastInterruptTime = currentTime;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    Serial.println("Систему запущено. Чекаю натискань...");
}

void loop()
{
    if (activePress && !pressSettled && (millis() - lastInterruptTime > 150))
    {
        noInterrupts();
        unsigned int currentPressNum = physicalPresses;
        unsigned int currentBounces = bounceCount;
        pressSettled = true;
        activePress = false;
        interrupts();

        Serial.print("Фізичне натискання №");
        Serial.print(currentPressNum);
        Serial.print(" завершено! Виявлено хибних спрацювань: ");
        Serial.println(currentBounces);
    }
}