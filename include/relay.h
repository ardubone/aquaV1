#pragma once
#include <Arduino.h>
#include <RTClib.h>

// Функции управления светом аквариума L (большой)
void initLightTankLrg();
void updateLightTankLrg(const DateTime &now);
void toggleLightTankLrg(const DateTime &now);
void resetLightTankLrgOverride();

// Функции получения состояния аквариума L
bool getLightTankLrgState();
DateTime getLightTankLrgLastToggleTime();
bool isLightTankLrgManualMode();

// Функции настройки времени работы аквариума L
void setLightTankLrgTimes(uint8_t onHour, uint8_t offHour);
uint8_t getLightTankLrgOnHour();
uint8_t getLightTankLrgOffHour();

// Функции управления светом аквариума S (малый)
void initLightTankSml();
void updateLightTankSml(const DateTime &now);
void toggleLightTankSml(const DateTime &now);
void resetLightTankSmlOverride();

// Функции получения состояния аквариума S
bool getLightTankSmlState();
DateTime getLightTankSmlLastToggleTime();
bool isLightTankSmlManualMode();

// Функции настройки времени работы аквариума S
void setLightTankSmlTimes(uint8_t onHour, uint8_t offHour);
uint8_t getLightTankSmlOnHour();
uint8_t getLightTankSmlOffHour();

// Функции управления UV лампой аквариума S
void initUvLampTankSml();
void updateUvLampTankSml(const DateTime &now);
void toggleUvLampTankSml(const DateTime &now);
void resetUvLampTankSmlOverride();

// Функции получения состояния UV лампы аквариума S
bool getUvLampTankSmlState();
DateTime getUvLampTankSmlLastToggleTime();
bool isUvLampTankSmlManualMode();

// Функции настройки времени работы UV лампы аквариума S
void setUvLampTankSmlTimes(uint8_t onHour, uint8_t offHour);
uint8_t getUvLampTankSmlOnHour();
uint8_t getUvLampTankSmlOffHour();

// Функции управления UV лампой аквариума L
void initUvLampTankLrg();
void updateUvLampTankLrg(const DateTime &now);
void toggleUvLampTankLrg(const DateTime &now);
void resetUvLampTankLrgOverride();

// Функции получения состояния UV лампы аквариума L
bool getUvLampTankLrgState();
DateTime getUvLampTankLrgLastToggleTime();
bool isUvLampTankLrgManualMode();

// Функции настройки времени работы UV лампы аквариума L
void setUvLampTankLrgTimes(uint8_t onHour, uint8_t offHour);
uint8_t getUvLampTankLrgOnHour();
uint8_t getUvLampTankLrgOffHour();

// Функции синхронного управления обоими светами (для кнопки Atom)
void toggleBothLights(const DateTime &now);
void setBothLightsAutoMode();
bool areBothLightsInAutoMode();
bool areBothLightsOn();

// Обратная совместимость (старые названия)
#define initRelay initLightTankLrg
#define updateRelay updateLightTankLrg
#define toggleRelay toggleLightTankLrg
#define resetRelayOverride resetLightTankLrgOverride
#define getRelayState getLightTankLrgState
#define getLastRelayToggleTime getLightTankLrgLastToggleTime
#define isRelayManualMode isLightTankLrgManualMode
#define setRelayTimes setLightTankLrgTimes
#define getRelayOnHour getLightTankLrgOnHour
#define getRelayOffHour getLightTankLrgOffHour

#define initRelayTank10 initLightTankSml
#define updateRelayTank10 updateLightTankSml
#define toggleRelayTank10 toggleLightTankSml
#define resetRelayTank10Override resetLightTankSmlOverride
#define getRelayTank10State getLightTankSmlState
#define getRelayTank10LastToggleTime getLightTankSmlLastToggleTime
#define isRelayTank10ManualMode isLightTankSmlManualMode
#define setRelayTank10Times setLightTankSmlTimes
#define getRelayTank10OnHour getLightTankSmlOnHour
#define getRelayTank10OffHour getLightTankSmlOffHour

#define initUvLampTank10 initUvLampTankSml
#define updateUvLampTank10 updateUvLampTankSml
#define toggleUvLampTank10 toggleUvLampTankSml
#define resetUvLampTank10Override resetUvLampTankSmlOverride
#define getUvLampTank10State getUvLampTankSmlState
#define getUvLampTank10LastToggleTime getUvLampTankSmlLastToggleTime
#define isUvLampTank10ManualMode isUvLampTankSmlManualMode
#define setUvLampTank10Times setUvLampTankSmlTimes
#define getUvLampTank10OnHour getUvLampTankSmlOnHour
#define getUvLampTank10OffHour getUvLampTankSmlOffHour

#define initUvLampTank20 initUvLampTankLrg
#define updateUvLampTank20 updateUvLampTankLrg
#define toggleUvLampTank20 toggleUvLampTankLrg
#define resetUvLampTank20Override resetUvLampTankLrgOverride
#define getUvLampTank20State getUvLampTankLrgState
#define getUvLampTank20LastToggleTime getUvLampTankLrgLastToggleTime
#define isUvLampTank20ManualMode isUvLampTankLrgManualMode
#define setUvLampTank20Times setUvLampTankLrgTimes
#define getUvLampTank20OnHour getUvLampTankLrgOnHour
#define getUvLampTank20OffHour getUvLampTankLrgOffHour
