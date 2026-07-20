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

#define RIGHT_BTN_PIN 7
#define LEFT_BTN_PIN 6
#define LDR_PIN 4
#define SENSOR_CHECK_INTERVAL_MS 50
#define SHADOW_THRESHOLD_PERCENT 20 // Тінь — це падіння світла на 20% від бази
#define DEBOUNCE_DELAY_MS 50

const unsigned long CALIBRATION_DURATION_MS = 2000; // Скільки триває калібрування
const unsigned long CODE_TIMEOUT_MS = 5000;         // Вікно введення коду (5 секунд)
const unsigned long FLASH_DURATION_MS = 150;        // Тривалість білого спалаху кнопки

unsigned long calibrationStartTime = 0;
unsigned long lastCodePressTime = 0;
unsigned long flashStartTime = 0;

int lastDebounceLeft = 0;
int lastDebounceRight = 0;
volatile bool lBtnClicked = false;
volatile bool rBtnClicked = false;
int codeStep = 0;            // На якому кроці пароля ми перебуваємо (0, 1, 2)
bool isCodeFlashing = false; // Чи горить зараз білий спалах підтвердження коду

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
void handleButtons();
void checkTimers();
void IRAM_ATTR onButtonLeftChange();
void IRAM_ATTR onButtonRightChange();

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(RIGHT_BTN_PIN, INPUT_PULLUP);
    pinMode(LEFT_BTN_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(LEFT_BTN_PIN), onButtonLeftChange, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RIGHT_BTN_PIN), onButtonRightChange, CHANGE);

    neopixelWrite(BUILTIN_LED, 0, 0, 0);
}

void loop()
{
    handleButtons();
    checkTimers();
    readSensors();
    updateLED();
}

void updateLED()
{
    unsigned long now = millis();

    static unsigned long lastUpdate = 0;
    static int brightness = 0;
    static int fadeAmount = 5;
    static bool strobeState = false;
    static int fadePeriod = 20;

    if (isCodeFlashing)
    {
        neopixelWrite(BUILTIN_LED, 150, 150, 150);
        return; // Виходимо, ігноруючи інші режими
    }

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

void IRAM_ATTR onButtonLeftChange()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceLeft >= DEBOUNCE_DELAY_MS)
    {
        lastDebounceLeft = currentTime;

        if (digitalRead(LEFT_BTN_PIN) == LOW)
        {
            lBtnClicked = true;
        }
    }
}

void IRAM_ATTR onButtonRightChange()
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceRight >= DEBOUNCE_DELAY_MS)
    {
        lastDebounceRight = currentTime;

        if (digitalRead(RIGHT_BTN_PIN) == LOW)
        {
            rBtnClicked = true;
        }
    }
}

void handleButtons()
{
    unsigned long now = millis();

    if (lBtnClicked)
    {
        lBtnClicked = false;

        if (currentState == STATE_UNARMED)
        {
            // Якщо вимкнено — запускаємо калібрування
            currentState = STATE_CALIBRATION;
            calibrationStartTime = now;
        }
        else if (currentState == STATE_ALARM)
        {
            // Перевірка коду: очікуємо Праву Кнопку як Крок 0 або Крок 2
            if (codeStep == 0)
            {
                codeStep = 1;
                lastCodePressTime = now;
                isCodeFlashing = true;
                flashStartTime = now; // Вмикаємо білий спалах
            }
            else if (codeStep == 2)
            {
                // Код введено повністю і правильно! Знімаємо охорону
                codeStep = 0;
                currentState = STATE_UNARMED;
            }
            else
            {
                codeStep = 0; // Помилка — скидаємо прогрес
            }
        }
    }

    if (rBtnClicked)
    {
        rBtnClicked = false;

        if (currentState == STATE_ARMED)
        {
            currentState = STATE_UNARMED;
        }
        else if (currentState == STATE_ALARM)
        {
            // Перевірка коду: очікуємо Ліву Кнопку як Крок 1
            if (codeStep == 1)
            {
                codeStep = 2;
                lastCodePressTime = now;
                isCodeFlashing = true;
                flashStartTime = now; // Вмикаємо білий спалах
            }
            else
            {
                codeStep = 0; // Помилка — скидаємо прогрес
            }
        }
    }
}

void checkTimers()
{
    unsigned long now = millis();

    // Авто-вихід з калібрування в режим охорони
    if (currentState == STATE_CALIBRATION)
    {
        if (now - calibrationStartTime >= CALIBRATION_DURATION_MS)
        {
            currentState = STATE_ARMED;
            // Тут автоматично спрацює логіка першого запуску автокалібрування в readSensors
        }
    }

    // Таймаут введення пароля
    if (currentState == STATE_ALARM && codeStep > 0)
    {
        if (now - lastCodePressTime >= CODE_TIMEOUT_MS)
        {
            codeStep = 0; // 5 секунд минуло, обнуляємо спробу
        }
    }

    // Вимкнення спалаху кнопки через 150 мс
    if (isCodeFlashing)
    {
        if (now - flashStartTime >= FLASH_DURATION_MS)
        {
            isCodeFlashing = false;
        }
    }
}