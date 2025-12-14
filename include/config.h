// config.h
#pragma once

#include <DallasTemperature.h>

// Дебаг режим - раскомментируйте для включения
// #define DEBUG_MODE

extern DeviceAddress tankLrgSensorAddr;
extern DeviceAddress tankSmlSensorAddr;

// Адреса температурных датчиков (Дефолтные значения)
const DeviceAddress TEMP_SENSOR_ADDR_TANK_LRG = {0x28, 0x29, 0x1F, 0x52, 0x00, 0x00, 0x00, 0xF2};
const DeviceAddress TEMP_SENSOR_ADDR_TANK_SML = {0x28, 0x97, 0xf9, 0x52, 0x00, 0x00, 0x00, 0xa0};
//0x28 97 f9 52 00 00 00 a0

// Флаги состояния компонентов
extern bool isRtcInitialized;
extern bool isBme280Initialized;
extern bool isDs18b20Initialized;
extern bool isPcf8574Initialized;
extern bool isAutoFeederInitialized;
extern bool isCameraInitialized;

// Пины для управления светом аквариумов
#define LIGHT_PIN_TANK_LRG  33  // Пин для света аквариума L (большой)
#define LIGHT_PIN_TANK_SML  23  // Пин для света аквариума S (малый)

// Константы для UI названий аквариумов
#define TANK_LRG_NAME "Аквариум L"
#define TANK_SML_NAME "Аквариум S"
// GPIO 25 - резервный пин (ранее использовался для UV лампы)

#define ONE_WIRE_BUS 19

// DS18B20: интервал повторного сканирования шины, если датчики не обнаружены (мс)
#define DS18B20_RESCAN_INTERVAL_MS 5000UL

// DS18B20: \"фантом\" 85°C часто появляется при сбое/неуспевшей конверсии — считаем невалидным
#define DS18B20_INVALID_TEMP_C 85.0f
#define DS18B20_INVALID_TEMP_EPS 0.01f

#define I2C_SDA      21
#define I2C_SCL      22

// Параметры автокормушки
#define AUTOFEEDER_RELAY_DELAY          1000  // Задержка после включения реле (мс)
#define AUTOFEEDER_LIMIT_IGNORE_TIME    5000  // Время игнорирования концевика (мс) - устарело, используется AUTOFEEDER_LIMIT_DEBOUNCE_MS
#define AUTOFEEDER_SCHEDULE_CHECK_INT   45000 // Интервал проверки расписания (мс)
#define AUTOFEEDER_LIMIT_DEBOUNCE_MS    50    // Время дебаунса концевика (мс) - минимальное время LOW для валидации
#define AUTOFEEDER_STOP_DELAY_MS        100   // Задержка остановки мотора после обнаружения LOW (мс)
#define AUTOFEEDER_START_COOLDOWN_MS    60000 // Защита от повторного запуска в ту же минуту (мс)

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
#define PCF8574_FEEDER_INVERT_LOGIC     false  // true = инверсия (LOW на PCF = включено), false = прямая (HIGH на PCF = включено)

#define PCF8574_ADDRESS 0x20

// PCF8574: периодическая принудительная перезапись выходов, чтобы UV/реле не \"слетали\" от помех (мс)
#define PCF8574_REFRESH_INTERVAL_MS 250UL

// Конфигурация пинов PCF8574
struct PCF8574Config {
    uint8_t pin;
    bool isOutput;
    const char* name;
};

const PCF8574Config pcf8574Pins[] = {
    {0, true, "UV_LAMP_TANK_SML"},      // UV лампа аквариума S (выход)
    {1, true, "UV_LAMP_TANK_LRG"},      // UV лампа аквариума L (выход)
    {2, true, "FEEDER_RELAY_TANK_SML"}, // Реле кормушки аквариума S (выход)
    {3, false, "FEEDER_LIMIT_TANK_SML"}, // Концевик кормушки аквариума S (вход)
    {4, true, "FEEDER_RELAY_TANK_LRG"},  // Реле кормушки аквариума L (выход)
    {5, false, "FEEDER_LIMIT_TANK_LRG"} // Концевик кормушки аквариума L (вход)
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