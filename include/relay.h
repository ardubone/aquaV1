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

// Функции управления UV лампой Tank10
void initUvLampTank10();
void updateUvLampTank10(const DateTime &now);
void toggleUvLampTank10(const DateTime &now);
void resetUvLampTank10Override();

// Функции получения состояния UV лампы Tank10
bool getUvLampTank10State();
DateTime getUvLampTank10LastToggleTime();
bool isUvLampTank10ManualMode();

// Функции настройки времени работы UV лампы Tank10
void setUvLampTank10Times(uint8_t onHour, uint8_t offHour);
uint8_t getUvLampTank10OnHour();
uint8_t getUvLampTank10OffHour();

// Функции управления UV лампой Tank20
void initUvLampTank20();
void updateUvLampTank20(const DateTime &now);
void toggleUvLampTank20(const DateTime &now);
void resetUvLampTank20Override();

// Функции получения состояния UV лампы Tank20
bool getUvLampTank20State();
DateTime getUvLampTank20LastToggleTime();
bool isUvLampTank20ManualMode();

// Функции настройки времени работы UV лампы Tank20
void setUvLampTank20Times(uint8_t onHour, uint8_t offHour);
uint8_t getUvLampTank20OnHour();
uint8_t getUvLampTank20OffHour(); 