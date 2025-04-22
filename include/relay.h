#pragma once
#include <Arduino.h>
#include <RTClib.h>

// Функции управления реле
void initRelay();
void updateRelay(const DateTime &now);
void toggleRelay(const DateTime &now);
void resetRelayOverride();

// Функции получения состояния
bool getRelayState();
DateTime getLastRelayToggleTime();
bool isRelayManualMode();

// Функции настройки времени работы
void setRelayTimes(uint8_t onHour, uint8_t offHour);
uint8_t getRelayOnHour();
uint8_t getRelayOffHour(); 