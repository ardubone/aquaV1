// config.h
#pragma once

#include <DallasTemperature.h>

// Дебаг режим - раскомментируйте для включения
 #define DEBUG_MODE

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

// Флаги состояния компонентов
extern bool isRtcInitialized;
extern bool isBme280Initialized;
extern bool isDs18b20Initialized;
extern bool isPcf8574Initialized;
extern bool isAutoFeederInitialized;
extern bool isCameraInitialized;

// Переименовываем RELAY_PIN и добавляем новый пин для второго реле
#define RELAY_PIN_TANK20  33  // Пин для реле аквариума 20
#define RELAY_PIN_TANK10  23  // Пин для реле аквариума 10
// GPIO 25 - резервный пин (ранее использовался для UV лампы)

#define ONE_WIRE_BUS 19

#define I2C_SDA      21
#define I2C_SCL      22

// Параметры автокормушки
#define AUTOFEEDER_RELAY_DELAY          1000  // Задержка после включения реле (мс)
#define AUTOFEEDER_LIMIT_IGNORE_TIME    5000  // Время игнорирования концевика (мс)
#define AUTOFEEDER_SCHEDULE_CHECK_INT   45000 // Интервал проверки расписания (мс)

// Дефолтное расписание кормления
// Дни недели: 0=Вс, 1=Пн, 2=Вт, 3=Ср, 4=Чт, 5=Пт, 6=Сб
#define DEFAULT_FEEDING_FIRST_HOUR      9   // Первое кормление - час
#define DEFAULT_FEEDING_FIRST_MINUTE    30  // Первое кормление - минута
#define DEFAULT_FEEDING_SECOND_HOUR     18  // Второе кормление - час
#define DEFAULT_FEEDING_SECOND_MINUTE   0   // Второе кормление - минута
// Дни кормления: Пн, Вт, Ср, Чт, Пт, Вс (без Сб)
#define DEFAULT_FEEDING_DAYS            {1, 2, 3, 4, 5, 0}
#define DEFAULT_FEEDING_DAYS_COUNT      6

// Пины PCF8574 для кормушек
#define PCF8574_FEEDER_RELAY_TANK10     2  // Реле кормушки Tank10 (выход)
#define PCF8574_FEEDER_LIMIT_TANK10     3  // Концевик кормушки Tank10 (вход)
#define PCF8574_FEEDER_RELAY_TANK20     4  // Реле кормушки Tank20 (выход)
#define PCF8574_FEEDER_LIMIT_TANK20     5  // Концевик кормушки Tank20 (вход)

// Логика управления MOSFET кормушек
// Если MOSFET не работает с инверсией (подтяжка), установите в false для прямой логики
#define PCF8574_FEEDER_INVERT_LOGIC     true  // true = инверсия (LOW на PCF = включено), false = прямая (HIGH на PCF = включено)

#define PCF8574_ADDRESS 0x20

// Конфигурация пинов PCF8574
struct PCF8574Config {
    uint8_t pin;
    bool isOutput;
    const char* name;
};

const PCF8574Config pcf8574Pins[] = {
    {0, true, "UV_LAMP_TANK10"},      // UV лампа Tank10 (выход)
    {1, true, "UV_LAMP_TANK20"},      // UV лампа Tank20 (выход)
    {2, true, "FEEDER_RELAY_TANK10"}, // Реле кормушки Tank10 (выход)
    {3, false, "FEEDER_LIMIT_TANK10"}, // Концевик кормушки Tank10 (вход)
    {4, true, "FEEDER_RELAY_TANK20"},  // Реле кормушки Tank20 (выход)
    {5, false, "FEEDER_LIMIT_TANK20"} // Концевик кормушки Tank20 (вход)
};

const uint8_t PCF8574_PIN_COUNT = sizeof(pcf8574Pins) / sizeof(PCF8574Config);

// Конфигурация камеры OV7670
#define CAMERA_FRAME_WIDTH  640
#define CAMERA_FRAME_HEIGHT 480
#define CAMERA_FPS          10

// Включить/выключить поддержку камеры
// Установите в 1, если камера подключена с параллельными пинами
// Установите в 0, если камера не подключена или подключена только по I2C
#define ENABLE_CAMERA 0