// debug_mocks.h
#pragma once

#include <Arduino.h>
#include <RTClib.h>

// Функции-заглушки для дебаг режима

// Заглушка времени (использует millis() для симуляции)
DateTime getMockTime();

// Заглушки температуры
float getMockTank20Temperature();
float getMockTank10Temperature();

// Заглушки данных комнаты
float getMockRoomTemp();
float getMockRoomHumidity();
float getMockRoomPressure();

