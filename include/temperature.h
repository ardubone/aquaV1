#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 19

extern OneWire oneWire;
extern DallasTemperature sensors;

extern DeviceAddress tankLrgSensorAddr;
extern DeviceAddress tankSmlSensorAddr;

void initTemperatureSensors();
void rescanTemperatureSensors(); // Повторное сканирование датчиков
void requestTemperatures();
float getLrgTemperature();
float getSmlTemperature();
String getLrgTemperatureString();
String getSmlTemperatureString();
bool isSensorConnected(uint8_t index);

// Функции для работы с адресами датчиков
uint8_t getAllConnectedSensors(DeviceAddress* addresses, uint8_t maxCount);
bool isAddressValid(const DeviceAddress address); // Проверяет, существует ли датчик с указанным адресом
void saveSensorAddressesToEEPROM();
bool loadSensorAddressesFromEEPROM(); // true, если удалось загрузить из EEPROM
bool setSensorAddress(uint8_t tankIndex, DeviceAddress address); // tankIndex: 0=Аквариум L, 1=Аквариум S

// Диагностика
void debugScanOneWireBus();

// Обратная совместимость
float getTank20Temperature();
float getTank10Temperature(); 