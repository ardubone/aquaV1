#include "relay.h"
#include "config.h"
#include "pcf8574_manager.h"

extern PCF8574Manager pcfManager;

// Переменные состояния света аквариума L (большой)
static bool lightTankLrgState = false;
static bool lightTankLrgManualOverride = false;
static DateTime lastLightTankLrgToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы аквариума L
static uint8_t lightTankLrgOnHour = 10;
static uint8_t lightTankLrgOffHour = 18;

// Переменные состояния света аквариума S (малый)
static bool lightTankSmlState = false;
static bool lightTankSmlManualOverride = false;
static DateTime lastLightTankSmlToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы аквариума S
static uint8_t lightTankSmlOnHour = 10;
static uint8_t lightTankSmlOffHour = 18;

// Переменные состояния UV лампы аквариума S
static bool uvLampTankSmlState = false;
static bool uvLampTankSmlManualOverride = false;
static DateTime lastUvLampTankSmlToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы UV лампы аквариума S
static uint8_t uvLampTankSmlOnHour = 8;
static uint8_t uvLampTankSmlOffHour = 10;

// Переменные состояния UV лампы аквариума L
static bool uvLampTankLrgState = false;
static bool uvLampTankLrgManualOverride = false;
static DateTime lastUvLampTankLrgToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы UV лампы аквариума L
static uint8_t uvLampTankLrgOnHour = 8;
static uint8_t uvLampTankLrgOffHour = 10;

void initLightTankLrg() {
  pinMode(LIGHT_PIN_TANK_LRG, OUTPUT);
  digitalWrite(LIGHT_PIN_TANK_LRG, LOW);
}

void initLightTankSml() {
  pinMode(LIGHT_PIN_TANK_SML, OUTPUT);
  digitalWrite(LIGHT_PIN_TANK_SML, LOW);
}

void initUvLampTankSml() {
  // UV лампа аквариума S управляется через PCF8574 пин 0
  // Инициализация происходит в PCF8574Manager::begin()
  pcfManager.setUvLampTank10(false);
}

void initUvLampTankLrg() {
  // UV лампа аквариума L управляется через PCF8574 пин 1
  // Инициализация происходит в PCF8574Manager::begin()
  pcfManager.setUvLampTank20(false);
}

void updateLightTankLrg(const DateTime &now) {
  if (!lightTankLrgManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= lightTankLrgOnHour && hour < lightTankLrgOffHour);
    
    if (lightTankLrgState != shouldBeOn) {
      lightTankLrgState = shouldBeOn;
      digitalWrite(LIGHT_PIN_TANK_LRG, lightTankLrgState ? HIGH : LOW);
      lastLightTankLrgToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedDay = -1;
  if (now.day() != lastCheckedDay) {
    lastCheckedDay = now.day();
    lightTankLrgManualOverride = false;
  }
}

void updateLightTankSml(const DateTime &now) {
  if (!lightTankSmlManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= lightTankSmlOnHour && hour < lightTankSmlOffHour);
    
    if (lightTankSmlState != shouldBeOn) {
      lightTankSmlState = shouldBeOn;
      digitalWrite(LIGHT_PIN_TANK_SML, lightTankSmlState ? HIGH : LOW);
      lastLightTankSmlToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedSmlDay = -1;
  if (now.day() != lastCheckedSmlDay) {
    lastCheckedSmlDay = now.day();
    lightTankSmlManualOverride = false;
  }
}

void updateUvLampTankSml(const DateTime &now) {
  if (!uvLampTankSmlManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= uvLampTankSmlOnHour && hour < uvLampTankSmlOffHour);
    
    if (uvLampTankSmlState != shouldBeOn) {
      uvLampTankSmlState = shouldBeOn;
      pcfManager.setUvLampTank10(uvLampTankSmlState);
      lastUvLampTankSmlToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedSmlDay = -1;
  if (now.day() != lastCheckedSmlDay) {
    lastCheckedSmlDay = now.day();
    uvLampTankSmlManualOverride = false;
  }
}

void updateUvLampTankLrg(const DateTime &now) {
  if (!uvLampTankLrgManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= uvLampTankLrgOnHour && hour < uvLampTankLrgOffHour);
    
    if (uvLampTankLrgState != shouldBeOn) {
      uvLampTankLrgState = shouldBeOn;
      pcfManager.setUvLampTank20(uvLampTankLrgState);
      lastUvLampTankLrgToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedLrgDay = -1;
  if (now.day() != lastCheckedLrgDay) {
    lastCheckedLrgDay = now.day();
    uvLampTankLrgManualOverride = false;
  }
}

void toggleLightTankLrg(const DateTime &now) {
  lightTankLrgState = !lightTankLrgState;
  digitalWrite(LIGHT_PIN_TANK_LRG, lightTankLrgState ? HIGH : LOW);
  lastLightTankLrgToggleTime = now;
  lightTankLrgManualOverride = true;
}

void toggleLightTankSml(const DateTime &now) {
  lightTankSmlState = !lightTankSmlState;
  digitalWrite(LIGHT_PIN_TANK_SML, lightTankSmlState ? HIGH : LOW);
  lastLightTankSmlToggleTime = now;
  lightTankSmlManualOverride = true;
}

void toggleUvLampTankSml(const DateTime &now) {
  uvLampTankSmlState = !uvLampTankSmlState;
  pcfManager.setUvLampTank10(uvLampTankSmlState);
  lastUvLampTankSmlToggleTime = now;
  uvLampTankSmlManualOverride = true;
}

void toggleUvLampTankLrg(const DateTime &now) {
  uvLampTankLrgState = !uvLampTankLrgState;
  pcfManager.setUvLampTank20(uvLampTankLrgState);
  lastUvLampTankLrgToggleTime = now;
  uvLampTankLrgManualOverride = true;
}

void resetLightTankLrgOverride() {
  lightTankLrgManualOverride = false;
}

void resetLightTankSmlOverride() {
  lightTankSmlManualOverride = false;
}

void resetUvLampTankSmlOverride() {
  uvLampTankSmlManualOverride = false;
}

void resetUvLampTankLrgOverride() {
  uvLampTankLrgManualOverride = false;
}

bool getLightTankLrgState() {
  return lightTankLrgState;
}

bool getLightTankSmlState() {
  return lightTankSmlState;
}

DateTime getLightTankLrgLastToggleTime() {
  return lastLightTankLrgToggleTime;
}

DateTime getLightTankSmlLastToggleTime() {
  return lastLightTankSmlToggleTime;
}

bool isLightTankLrgManualMode() {
  return lightTankLrgManualOverride;
}

bool isLightTankSmlManualMode() {
  return lightTankSmlManualOverride;
}

bool getUvLampTankSmlState() {
  return uvLampTankSmlState;
}

DateTime getUvLampTankSmlLastToggleTime() {
  return lastUvLampTankSmlToggleTime;
}

bool isUvLampTankSmlManualMode() {
  return uvLampTankSmlManualOverride;
}

bool getUvLampTankLrgState() {
  return uvLampTankLrgState;
}

DateTime getUvLampTankLrgLastToggleTime() {
  return lastUvLampTankLrgToggleTime;
}

bool isUvLampTankLrgManualMode() {
  return uvLampTankLrgManualOverride;
}

void setLightTankLrgTimes(uint8_t onHour, uint8_t offHour) {
  lightTankLrgOnHour = onHour;
  lightTankLrgOffHour = offHour;
}

void setLightTankSmlTimes(uint8_t onHour, uint8_t offHour) {
  lightTankSmlOnHour = onHour;
  lightTankSmlOffHour = offHour;
}

void setUvLampTankSmlTimes(uint8_t onHour, uint8_t offHour) {
  uvLampTankSmlOnHour = onHour;
  uvLampTankSmlOffHour = offHour;
}

uint8_t getUvLampTankSmlOnHour() {
  return uvLampTankSmlOnHour;
}

uint8_t getUvLampTankSmlOffHour() {
  return uvLampTankSmlOffHour;
}

void setUvLampTankLrgTimes(uint8_t onHour, uint8_t offHour) {
  uvLampTankLrgOnHour = onHour;
  uvLampTankLrgOffHour = offHour;
}

uint8_t getUvLampTankLrgOnHour() {
  return uvLampTankLrgOnHour;
}

uint8_t getUvLampTankLrgOffHour() {
  return uvLampTankLrgOffHour;
}

uint8_t getLightTankLrgOnHour() {
  return lightTankLrgOnHour;
}

uint8_t getLightTankSmlOnHour() {
  return lightTankSmlOnHour;
}

uint8_t getLightTankLrgOffHour() {
  return lightTankLrgOffHour;
}

uint8_t getLightTankSmlOffHour() {
  return lightTankSmlOffHour;
}

// Функции синхронного управления обоими светами
void toggleBothLights(const DateTime &now) {
  // Переключаем оба света одновременно
  toggleLightTankLrg(now);
  toggleLightTankSml(now);
}

void setBothLightsAutoMode() {
  // Сбрасываем ручной режим для обоих светов
  resetLightTankLrgOverride();
  resetLightTankSmlOverride();
}

bool areBothLightsInAutoMode() {
  // Проверяем, что оба света в авто режиме
  return !lightTankLrgManualOverride && !lightTankSmlManualOverride;
}

bool areBothLightsOn() {
  // Проверяем, что оба света включены
  return lightTankLrgState && lightTankSmlState;
}
