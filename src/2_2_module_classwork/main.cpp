#include <Arduino.h>
const uint8_t LDR_PIN = 5;
const uint8_t LED_PIN = 4;
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