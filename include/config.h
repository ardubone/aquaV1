// config.h
#pragma once

#include <DallasTemperature.h>

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

#define ENCODER_CLK 25 
#define ENCODER_DT  32
#define BUTTON_PIN  33 
#define RELAY_PIN  23  // выбери свободный пин

#define ONE_WIRE_BUS 19
//#define DHTPIN       23
//#define DHTTYPE      DHT11

#define I2C_SDA      21
#define I2C_SCL      22

// Время работы реле
extern uint8_t relayOnHour;
extern uint8_t relayOffHour;