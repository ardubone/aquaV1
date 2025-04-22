// config.h
#pragma once

#include <DallasTemperature.h>

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

//#define ENCODER_CLK 25 
//#define ENCODER_DT  32
// #define BUTTON_PIN  23 
#define RELAY_PIN  33  // выбери свободный пин

#define ONE_WIRE_BUS 19

#define I2C_SDA      21
#define I2C_SCL      22

// Пины для автокормушки
#define AUTOFEEDER_BUTTON_PIN     39  // Пин для кнопки автокормушки
#define AUTOFEEDER_LIMIT_PIN      23  // Пин для концевого выключателя
#define AUTOFEEDER_MOSFET_PIN     25  // Пин для управления MOSFET

// Параметры автокормушки
#define AUTOFEEDER_MOSFET_DELAY         1000  // Задержка после включения MOSFET (мс)
#define AUTOFEEDER_LIMIT_IGNORE_TIME    5000  // Время игнорирования концевика (мс)
#define AUTOFEEDER_SCHEDULE_CHECK_INT   45000 // Интервал проверки расписания (мс)