#include <Arduino.h>

#define ATTN_BTN_PIN 7
#define BIT_BTN_PIN 6
#define LDR_PIN 4
#define LED_PIN 3
#define ADC_DELAY_MS 1000
#define ATTENUATION 0
#define BITS 12

const float uRef = 3300.0;
const float adcMax = 4095.0;
int lastTime = 0;

const int resolutions[] = {9, 10, 11, 12};
const adc_attenuation_t attenFact[] = {ADC_0db, ADC_2_5db, ADC_6db, ADC_11db};
const float uRefsMv[] = {1100.0, 1500.0, 2200.0, 3100.0};

float calculateVoltage(int rawData)
{
    return (rawData / adcMax) * uRef;
}

float calculateError(float uCalc, float uMeas)
{
    float absoluteError = fabs(uCalc - uMeas);
    return (absoluteError / uMeas) * 100.0;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(ATTN_BTN_PIN, INPUT_PULLUP);
    pinMode(BIT_BTN_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    analogSetPinAttenuation(LDR_PIN, ADC_11db);
    lastTime = millis();
}

void loop()
{
    int now = millis();
    if (now - lastTime >= ADC_DELAY_MS)
    {
        lastTime = now;
        int raw = analogRead(LDR_PIN);
        float uCalc = calculateVoltage(raw);
        float uMeasur = analogReadMilliVolts(LDR_PIN);
        float relativeError = calculateError(uCalc, uMeasur);

        Serial.printf("\nADC= %d, U_Calc= %.3fmV, uMeasur= %.1fmV\n Відносна похибка= %.2f відсотків\n\n", raw, uCalc, uMeasur, relativeError);
    }
}