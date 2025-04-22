#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 19

extern OneWire oneWire;

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

void initTemperatureSensors();
void requestTemperatures();
float getTank20Temperature();
float getTank10Temperature();
bool isSensorConnected(uint8_t index); 