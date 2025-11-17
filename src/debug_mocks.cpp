// debug_mocks.cpp
#include "debug_mocks.h"
#include "config.h"

#ifdef DEBUG_MODE

// Базовое время для симуляции (1 января 2024, 00:00:00)
static const unsigned long BASE_TIMESTAMP = 1704067200; // Unix timestamp для 1.1.2024 00:00:00

DateTime getMockTime() {
    unsigned long currentSeconds = (millis() / 1000) + BASE_TIMESTAMP;
    return DateTime(currentSeconds);
}

float getMockTank20Temperature() {
    // Симуляция температуры с небольшими колебаниями
    static float baseTemp = 25.0;
    static float variation = 0.0;
    variation += 0.01;
    if (variation > 1.0) variation = -1.0;
    return baseTemp + variation;
}

float getMockTank10Temperature() {
    // Симуляция температуры с небольшими колебаниями
    static float baseTemp = 24.5;
    static float variation = 0.0;
    variation += 0.015;
    if (variation > 1.0) variation = -1.0;
    return baseTemp + variation;
}

float getMockRoomTemp() {
    // Симуляция комнатной температуры
    static float baseTemp = 22.0;
    static float variation = 0.0;
    variation += 0.02;
    if (variation > 2.0) variation = -2.0;
    return baseTemp + variation;
}

float getMockRoomHumidity() {
    // Симуляция влажности
    static float baseHumidity = 45.0;
    static float variation = 0.0;
    variation += 0.1;
    if (variation > 5.0) variation = -5.0;
    return baseHumidity + variation;
}

float getMockRoomPressure() {
    // Симуляция давления (мм рт.ст.)
    static float basePressure = 760.0;
    static float variation = 0.0;
    variation += 0.05;
    if (variation > 3.0) variation = -3.0;
    return basePressure + variation;
}

#endif // DEBUG_MODE

