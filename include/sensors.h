#pragma once
#include <Arduino.h>

// Инициализация датчиков комнаты
void initRoomSensors();

// Функции получения данных с датчиков комнаты
float getRoomTemp();
float getRoomHumidity();
float getRoomPressure();

// Обновление данных с датчиков
void updateRoomSensors(); 