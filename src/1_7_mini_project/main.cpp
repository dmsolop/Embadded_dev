#include <Arduino.h>

// =========================================================================
// НАЛАШТУВАННЯ АВТОКАЛІБРУВАННЯ ТА ФІЛЬТРАЦІЇ АЦП
// =========================================================================

// Важливо: сума цих двох коефіцієнтів ЗАВЖДИ повинна дорівнювати рівно 1.0!

// Вага історії (минулого стану світла в кімнаті).
// Визначає, на скільки відсотків система тримається за свої старі спогади.
// Значення 0.98 означає, що база на 98% складається з попереднього усередненого світла.
// Чим ближче до 1.0, тим повільніше датчик звикає до настання вечора/ранку і тим краще ловить тіні.
const float FILTER_HISTORY_WEIGHT = 0.98;

// Вага нового заміру (коефіцієнт реакції на свіжі дані).
// Показує, яку частку від нового кліку АЦП ми додаємо в пам'ять бази кожні 50 мс.
// Значення 0.02 означає, що новий замір впливає на загальну картину всього на 2%.
// Якщо його збільшити (наприклад, до 0.05), автокалібрування стане дуже швидким,
// але датчик почне "пробачати" швидкі рухи і може просто не помітити тінь від руки.
const float FILTER_NEW_WEIGHT = 0.02;

#define ATTN_BTN_PIN 7
#define BIT_BTN_PIN 6
#define LDR_PIN 4
#define SENSOR_CHECK_INTERVAL_MS 50
#define SHADOW_THRESHOLD_PERCENT 20 // Тінь — це падіння світла на 20% від бази

enum SystemState
{
    STATE_UNARMED,     // Знято з охорони (діод не світить або горить білим)
    STATE_CALIBRATION, // Жовта пульсація, АЦП запам'ятовує "нуль" освітленості
    STATE_ARMED,       // Охорона: зелене ШІМ-дихання, АЦП стежить за тінню
    STATE_ALARM        // Тривога: червоно-синій стробоскоп, чекає код розблокування
};

volatile SystemState currentState = STATE_UNARMED;

void updateLED();
void readSensors();

void setup()
{
    Serial.begin(115200);
    delay(1000);

    digitalWrite(BUILTIN_LED, 0);
    Serial.println("Піни та кнопки налаштовані");
}

void loop()
{
}

void updateLED()
{
    unsigned long now = millis();

    static unsigned long lastUpdate = 0;
    static int brightness = 0;
    static int fadeAmount = 5;       // крок дихання
    static bool strobeState = false; // для миготіння тривоги
    static int fadePeriod = 20;

    switch (currentState)
    {
    case STATE_UNARMED:
        // Просто вимкнений діод
        neopixelWrite(BUILTIN_LED, 0, 0, 0);
        break;

    case STATE_ARMED:
        // Зелене дихання
        if (now - lastUpdate >= fadePeriod)
        {
            lastUpdate = now;
            brightness += fadeAmount;

            if (brightness <= 0 || brightness >= 150)
            {
                fadeAmount = -fadeAmount;
            }
            neopixelWrite(BUILTIN_LED, 0, brightness, 0);
        }
        break;

    case STATE_CALIBRATION:
        // Швидке жовте миготіння (Червоний + Зелений = Жовтий)
        if (now - lastUpdate >= 150)
        {
            lastUpdate = now;
            strobeState = !strobeState;
            if (strobeState)
            {
                neopixelWrite(BUILTIN_LED, 100, 80, 0); // Помірний жовтий
            }
            else
            {
                neopixelWrite(BUILTIN_LED, 0, 0, 0);
            }
        }
        break;

    case STATE_ALARM:
        // Поліцейський стробоскоп на всю потужність: Червоний / Синій кожні 80 мс
        if (now - lastUpdate >= 80)
        {
            lastUpdate = now;
            strobeState = !strobeState;
            if (strobeState)
            {
                neopixelWrite(BUILTIN_LED, 255, 0, 0); // Чистий Червоний
            }
            else
            {
                neopixelWrite(BUILTIN_LED, 0, 0, 255); // Чистий Синій
            }
        }
        break;
    }
}

void readSensors()
{
    // Відпрацьовує тільки якщо система в режимі охорони
    if (currentState != STATE_ARMED)
        return;

    unsigned long now = millis();
    static unsigned long lastSensorCheck = 0;
    static float ambientBaseline = -1.0; // -1 означає, що першого заміру ще не було

    if (now - lastSensorCheck >= SENSOR_CHECK_INTERVAL_MS)
    {
        lastSensorCheck = now;
        int currentRaw = analogRead(LDR_PIN);

        // Ініціалізація при першому старті охорони, свого роду калібрування перед вимірюванням
        if (ambientBaseline < 0)
        {
            ambientBaseline = currentRaw;
            return;
        }

        // повільно підлаштовуємо базу під кімнатне світло
        ambientBaseline = (ambientBaseline * FILTER_HISTORY_WEIGHT) + (currentRaw * FILTER_NEW_WEIGHT);

        // рахуємо, на скільки відсотків упало світло
        float dropPercentage = ((ambientBaseline - currentRaw) / ambientBaseline) * 100.0;

        // якщо впало різко і сильніше ніж на поріг — ТРИВОГА!
        if (dropPercentage >= SHADOW_THRESHOLD_PERCENT)
        {
            currentState = STATE_ALARM;
        }
    }
}