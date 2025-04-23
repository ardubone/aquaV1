#pragma once
#include <Arduino.h>
#include <RTClib.h>

// Функции управления реле Tank20
void initRelay();
void updateRelay(const DateTime &now);
void toggleRelay(const DateTime &now);
void resetRelayOverride();

// Функции получения состояния Tank20
bool getRelayState();
DateTime getLastRelayToggleTime();
bool isRelayManualMode();

// Функции настройки времени работы Tank20
void setRelayTimes(uint8_t onHour, uint8_t offHour);
uint8_t getRelayOnHour();
uint8_t getRelayOffHour();

// Новые функции для управления реле Tank10
void initRelayTank10();
void updateRelayTank10(const DateTime &now);
void toggleRelayTank10(const DateTime &now);
void resetRelayTank10Override();

// Функции получения состояния Tank10
bool getRelayTank10State();
DateTime getRelayTank10LastToggleTime();
bool isRelayTank10ManualMode();

// Функции настройки времени работы Tank10
void setRelayTank10Times(uint8_t onHour, uint8_t offHour);
uint8_t getRelayTank10OnHour();
uint8_t getRelayTank10OffHour(); 