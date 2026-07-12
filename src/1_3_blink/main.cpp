#include <Arduino.h>

#define LED_MY 4
#define BLINK_DELAY_MS 200

void setup()
{
    pinMode(RGB_BUILTIN, OUTPUT);
    Serial.println("Тест через RGB_BUILTIN запущено...");
    Serial.begin(115200);
    delay(1000);
    pinMode(LED_MY, OUTPUT);
    digitalWrite(LED_MY, LOW);
}

void loop()
{
    Serial.println("Вмикаємо білий (всі канали на максимум)");
    digitalWrite(RGB_BUILTIN, HIGH);
    delay(BLINK_DELAY_MS);

    Serial.println("Вимкнено");
    digitalWrite(RGB_BUILTIN, LOW);
    delay(BLINK_DELAY_MS);

    Serial.println("Blink!");
    digitalWrite(LED_MY, HIGH);
    delay(BLINK_DELAY_MS);
    digitalWrite(LED_MY, LOW);
    delay(BLINK_DELAY_MS);
}

// #include <Arduino.h>

// #define LED_MY 4
// long blinkDelayMs = 100;
// unsigned long previousMillis = 0;
// const long interval = 1000;
// bool isFast = true;

// void setup()
// {
// Serial.begin(115200);
// pinMode(LED_MY, OUTPUT);
// digitalWrite(LED_MY, LOW);
// }

// void loop()
// {
// unsigned long currentMillis = millis();

// if (currentMillis - previousMillis >= interval)
// {
// previousMillis = currentMillis;
// isFast = !isFast;
// }
// if (isFast)
// {
// blinkDelayMs = 50;
// }
// else
// {
// blinkDelayMs = 200;
// }
// Serial.println("Blink!");
// digitalWrite(LED_MY, HIGH);
// delay(blinkDelayMs);
// digitalWrite(LED_MY, LOW);
// delay(blinkDelayMs);
// }