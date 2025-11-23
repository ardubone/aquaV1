#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 19

extern OneWire oneWire;
extern DallasTemperature sensors;

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

void initTemperatureSensors();
void requestTemperatures();
float getTank20Temperature();
float getTank10Temperature();
bool isSensorConnected(uint8_t index);

// Функции для работы с адресами датчиков
uint8_t getAllConnectedSensors(DeviceAddress* addresses, uint8_t maxCount);
void saveSensorAddressesToEEPROM();
void loadSensorAddressesFromEEPROM();
bool setSensorAddress(uint8_t tankIndex, DeviceAddress address); // tankIndex: 0=Tank20, 1=Tank10 