// LDR + ADC + LED — 3 скрипти
// Схема: 3.3V — 10k — GPIO4 — LDR — GND; LED: GPIO9 — 330Ω — GND. Serial 115200. LDR знизу: світло →
// raw↓; темрява → raw↑.
// - 1 — digitalWrite ON/OFF за порогом.
// - 2 — analogWrite (PWM 8 біт).
// - 3 — ledcSetup / ledcWrite (PWM 12 біт).
// - Файли: Lesson 1.6/codes/01…03.cpp

/*
 * 1. Елементарний
 * Логіка: читаємо LDR (ADC) → виводимо raw у Serial →
 * якщо raw > порогу (темніше) — LED HIGH, інакше LOW.
 */
// #include <Arduino.h>

// const uint8_t LDR_PIN = 4;
// const uint8_t LED_PIN = 9;
// const int THRESHOLD = 2500; // вище = темніше (LDR знизу)

// void setup()
// {
//     Serial.begin(115200);
//     pinMode(LED_PIN, OUTPUT);
// }

// void loop()
// {
//     int raw = analogRead(LDR_PIN);
//     digitalWrite(LED_PIN, raw > THRESHOLD ? HIGH : LOW);
//     Serial.printf("raw=%d\n", raw);
//     delay(200);
// }

/*
 * 2. Базовий (analogWrite)
 * Логіка: raw з LDR → map у duty 0…255 → analogWrite на LED.
 * RAW_LIGHT / RAW_DARK — калібрування (світло / темрява).
 */
// #include <Arduino.h>
// const uint8_t LDR_PIN = 4;
// const uint8_t LED_PIN = 9;
// const int RAW_LIGHT = 3200;
// const int RAW_DARK = 1200;
// void setup() {
// Serial.begin(115200);
// }
// void loop() {
// int raw = analogRead(LDR_PIN);
// int duty = constrain(map(raw, RAW_LIGHT, RAW_DARK, 255, 0), 0, 255);
// analogWrite(LED_PIN, duty);
// Serial.printf("raw=%d duty=%d\n", raw, duty);
// delay(100);
// }

/*
 * 3. Базовий+ (ledcSetup, 12 біт)
 * Логіка: raw з LDR → map у duty 0…4095 → ledcWrite.
 * 12 біт PWM = та сама шкала, що в ADC.
 */
#include <Arduino.h>
const uint8_t LDR_PIN = 4;
const uint8_t LED_PIN = 9;
const uint8_t LEDC_CH = 0;
const uint8_t LEDC_BITS = 12;
const int PWM_MAX = 4095;
const int RAW_LIGHT = 3200;
const int RAW_DARK = 1200;
void setup()
{
    Serial.begin(115200);
    ledcSetup(LEDC_CH, 5000, LEDC_BITS);
    ledcAttachPin(LED_PIN, LEDC_CH);
}
void loop()
{
    int raw = analogRead(LDR_PIN);
    int duty = constrain(map(raw, RAW_LIGHT, RAW_DARK, PWM_MAX, 0), 0, PWM_MAX);
    ledcWrite(LEDC_CH, duty);
    Serial.printf("raw=%d duty=%d\n", raw, duty);
    delay(100);
}