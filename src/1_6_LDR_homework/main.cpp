#include <Arduino.h>

#define ATTN_BTN_PIN 7
#define BIT_BTN_PIN 6
#define LDR_PIN 4
#define LED_PIN 3
#define DEBOUNCE_DELAY_MS 50
#define ADC_DELAY_MS 1000
#define ATTENUATION 0
#define BITS 12

float uRef = 3300.0;
float adcMax = 4095.0;
unsigned long lastTime = 0;
volatile int currentBitsIdx = 3;
volatile int currentAttenIdx = 3;
volatile bool hasChange = false;
volatile unsigned long lastDebounceBits = 0;
volatile unsigned long lastDebounceAtten = 0;

const int resolutions[] = {9, 10, 11, 12};
const size_t numResolutions = sizeof(resolutions) / sizeof(resolutions[0]);
const adc_attenuation_t attenFactor[] = {ADC_0db, ADC_2_5db, ADC_6db, ADC_11db};
const size_t numAttenuations = sizeof(attenFactor) / sizeof(attenFactor[0]);
const float uRefsMv[] = {1100.0, 1500.0, 2200.0, 3100.0};
const char *attenLabels[] = {"0 dB", "2.5 dB", "6 dB", "11 dB"};

void IRAM_ATTR setCurrentBitsIdx()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceBits >= DEBOUNCE_DELAY_MS)
    {
        currentBitsIdx = (currentBitsIdx + 1) % numResolutions;
        hasChange = true;
        lastDebounceBits = currentTime;
    }
}

void IRAM_ATTR setCurrentAttenIdx()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceAtten >= DEBOUNCE_DELAY_MS)
    {
        currentAttenIdx = (currentAttenIdx + 1) % numAttenuations;
        hasChange = true;
        lastDebounceAtten = currentTime;
    }
}

float calculateVoltage(int rawData)
{
    return (rawData / adcMax) * uRef;
}

float calculateError(float uCalc, float uMeas)
{
    float absoluteError = fabs(uCalc - uMeas);
    return (absoluteError / uMeas) * 100.0;
}

float calcAdcMax(int bits)
{
    return adcMax = (1 << bits) - 1;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(ATTN_BTN_PIN, INPUT_PULLUP);
    pinMode(BIT_BTN_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(ATTN_BTN_PIN), setCurrentAttenIdx, FALLING);
    attachInterrupt(digitalPinToInterrupt(BIT_BTN_PIN), setCurrentBitsIdx, FALLING);

    analogSetPinAttenuation(LDR_PIN, ADC_11db);
    lastTime = millis();
}

void loop()
{
    unsigned long now = millis();

    if (hasChange)
    {
        hasChange = false;
        adcMax = calcAdcMax(resolutions[currentBitsIdx]);
        uRef = uRefsMv[currentAttenIdx];

        analogReadResolution(resolutions[currentBitsIdx]);
        analogSetPinAttenuation(LDR_PIN, attenFactor[currentAttenIdx]);

        Serial.println("\n==================================================");
        Serial.println("⚙️  РЕЖИМ ВИПРОБУВАЛЬНОГО СТЕНДУ ЗМІНЕНО!");
        Serial.printf("   🔹 Розрядність АЦП : %d біт (Max ADC = %.0f)\n", resolutions[currentBitsIdx], adcMax);
        Serial.printf("   🔸 Атенюація (Gain): %s (U_Ref = %.0f mV)\n", attenLabels[currentAttenIdx], uRef);
        Serial.println("==================================================\n");
    }

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