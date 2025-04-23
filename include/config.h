// config.h
#pragma once

#include <DallasTemperature.h>

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

// Переименовываем RELAY_PIN и добавляем новый пин для второго реле
#define RELAY_PIN_TANK20  33  // Пин для реле аквариума 20
#define RELAY_PIN_TANK10  23  // Пин для реле аквариума 10

#define ONE_WIRE_BUS 19

#define I2C_SDA      21
#define I2C_SCL      22

// Пины для автокормушки
#define AUTOFEEDER_BUTTON_PIN     39  // Пин для кнопки автокормушки
#define AUTOFEEDER_LIMIT_PIN      26  // Пин для концевого выключателя
#define AUTOFEEDER_MOSFET_PIN     25  // Пин для управления MOSFET

// Параметры автокормушки
#define AUTOFEEDER_MOSFET_DELAY         1000  // Задержка после включения MOSFET (мс)
#define AUTOFEEDER_LIMIT_IGNORE_TIME    5000  // Время игнорирования концевика (мс)
#define AUTOFEEDER_SCHEDULE_CHECK_INT   45000 // Интервал проверки расписания (мс)