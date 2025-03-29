// config.h
#pragma once

#include <DallasTemperature.h>

extern DeviceAddress innerSensorAddr;
extern DeviceAddress outerSensorAddr;

#define ENCODER_CLK 25 
#define ENCODER_DT  32
#define BUTTON_PIN  33 

#define ONE_WIRE_BUS 19
#define DHTPIN       23
#define DHTTYPE      DHT11

#define I2C_SDA      21
#define I2C_SCL      22