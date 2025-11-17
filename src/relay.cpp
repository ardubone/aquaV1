#include "relay.h"
#include "config.h"
#include "pcf8574_manager.h"

extern PCF8574Manager pcfManager;

// Переменные состояния реле Tank20
static bool relayState = false;
static bool relayManualOverride = false;
static DateTime lastRelayToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы Tank20
static uint8_t relayOnHour = 8;
static uint8_t relayOffHour = 19;

// Переменные состояния реле Tank10
static bool relayTank10State = false;
static bool relayTank10ManualOverride = false;
static DateTime lastRelayTank10ToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы Tank10
static uint8_t relayTank10OnHour = 8;
static uint8_t relayTank10OffHour = 19;

// Переменные состояния UV лампы Tank10
static bool uvLampTank10State = false;
static bool uvLampTank10ManualOverride = false;
static DateTime lastUvLampTank10ToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы UV лампы Tank10
static uint8_t uvLampTank10OnHour = 8;
static uint8_t uvLampTank10OffHour = 10;

// Переменные состояния UV лампы Tank20
static bool uvLampTank20State = false;
static bool uvLampTank20ManualOverride = false;
static DateTime lastUvLampTank20ToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы UV лампы Tank20
static uint8_t uvLampTank20OnHour = 8;
static uint8_t uvLampTank20OffHour = 10;

void initRelay() {
  pinMode(RELAY_PIN_TANK20, OUTPUT);
  digitalWrite(RELAY_PIN_TANK20, LOW);
}

void initRelayTank10() {
  pinMode(RELAY_PIN_TANK10, OUTPUT);
  digitalWrite(RELAY_PIN_TANK10, LOW);
}

void initUvLampTank10() {
  // UV лампа Tank10 управляется через PCF8574 пин 0
  // Инициализация происходит в PCF8574Manager::begin()
  pcfManager.setUvLampTank10(false);
}

void initUvLampTank20() {
  // UV лампа Tank20 управляется через PCF8574 пин 1
  // Инициализация происходит в PCF8574Manager::begin()
  pcfManager.setUvLampTank20(false);
}

void updateRelay(const DateTime &now) {
  if (!relayManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= relayOnHour && hour < relayOffHour);
    
    if (relayState != shouldBeOn) {
      relayState = shouldBeOn;
      digitalWrite(RELAY_PIN_TANK20, relayState ? HIGH : LOW);
      lastRelayToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedDay = -1;
  if (now.day() != lastCheckedDay) {
    lastCheckedDay = now.day();
    relayManualOverride = false;
  }
}

void updateRelayTank10(const DateTime &now) {
  if (!relayTank10ManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= relayTank10OnHour && hour < relayTank10OffHour);
    
    if (relayTank10State != shouldBeOn) {
      relayTank10State = shouldBeOn;
      digitalWrite(RELAY_PIN_TANK10, relayTank10State ? HIGH : LOW);
      lastRelayTank10ToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedTank10Day = -1;
  if (now.day() != lastCheckedTank10Day) {
    lastCheckedTank10Day = now.day();
    relayTank10ManualOverride = false;
  }
}

void updateUvLampTank10(const DateTime &now) {
  if (!uvLampTank10ManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= uvLampTank10OnHour && hour < uvLampTank10OffHour);
    
    if (uvLampTank10State != shouldBeOn) {
      uvLampTank10State = shouldBeOn;
      pcfManager.setUvLampTank10(uvLampTank10State);
      lastUvLampTank10ToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedTank10Day = -1;
  if (now.day() != lastCheckedTank10Day) {
    lastCheckedTank10Day = now.day();
    uvLampTank10ManualOverride = false;
  }
}

void updateUvLampTank20(const DateTime &now) {
  if (!uvLampTank20ManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= uvLampTank20OnHour && hour < uvLampTank20OffHour);
    
    if (uvLampTank20State != shouldBeOn) {
      uvLampTank20State = shouldBeOn;
      pcfManager.setUvLampTank20(uvLampTank20State);
      lastUvLampTank20ToggleTime = now;
    }
  }
  
  // Сброс ручного режима каждый день в полночь
  static int lastCheckedTank20Day = -1;
  if (now.day() != lastCheckedTank20Day) {
    lastCheckedTank20Day = now.day();
    uvLampTank20ManualOverride = false;
  }
}

void toggleRelay(const DateTime &now) {
  relayState = !relayState;
  digitalWrite(RELAY_PIN_TANK20, relayState ? HIGH : LOW);
  lastRelayToggleTime = now;
  relayManualOverride = true;
}

void toggleRelayTank10(const DateTime &now) {
  relayTank10State = !relayTank10State;
  digitalWrite(RELAY_PIN_TANK10, relayTank10State ? HIGH : LOW);
  lastRelayTank10ToggleTime = now;
  relayTank10ManualOverride = true;
}

void toggleUvLampTank10(const DateTime &now) {
  uvLampTank10State = !uvLampTank10State;
  pcfManager.setUvLampTank10(uvLampTank10State);
  lastUvLampTank10ToggleTime = now;
  uvLampTank10ManualOverride = true;
}

void toggleUvLampTank20(const DateTime &now) {
  uvLampTank20State = !uvLampTank20State;
  pcfManager.setUvLampTank20(uvLampTank20State);
  lastUvLampTank20ToggleTime = now;
  uvLampTank20ManualOverride = true;
}

void resetRelayOverride() {
  relayManualOverride = false;
}

void resetRelayTank10Override() {
  relayTank10ManualOverride = false;
}

void resetUvLampTank10Override() {
  uvLampTank10ManualOverride = false;
}

void resetUvLampTank20Override() {
  uvLampTank20ManualOverride = false;
}

bool getRelayState() {
  return relayState;
}

bool getRelayTank10State() {
  return relayTank10State;
}

DateTime getLastRelayToggleTime() {
  return lastRelayToggleTime;
}

DateTime getRelayTank10LastToggleTime() {
  return lastRelayTank10ToggleTime;
}

bool isRelayManualMode() {
  return relayManualOverride;
}

bool isRelayTank10ManualMode() {
  return relayTank10ManualOverride;
}

bool getUvLampTank10State() {
  return pcfManager.getUvLampTank10State();
}

DateTime getUvLampTank10LastToggleTime() {
  return lastUvLampTank10ToggleTime;
}

bool isUvLampTank10ManualMode() {
  return uvLampTank10ManualOverride;
}

bool getUvLampTank20State() {
  return pcfManager.getUvLampTank20State();
}

DateTime getUvLampTank20LastToggleTime() {
  return lastUvLampTank20ToggleTime;
}

bool isUvLampTank20ManualMode() {
  return uvLampTank20ManualOverride;
}

void setRelayTimes(uint8_t onHour, uint8_t offHour) {
  relayOnHour = onHour;
  relayOffHour = offHour;
}

void setRelayTank10Times(uint8_t onHour, uint8_t offHour) {
  relayTank10OnHour = onHour;
  relayTank10OffHour = offHour;
}

void setUvLampTank10Times(uint8_t onHour, uint8_t offHour) {
  uvLampTank10OnHour = onHour;
  uvLampTank10OffHour = offHour;
}

uint8_t getUvLampTank10OnHour() {
  return uvLampTank10OnHour;
}

uint8_t getUvLampTank10OffHour() {
  return uvLampTank10OffHour;
}

void setUvLampTank20Times(uint8_t onHour, uint8_t offHour) {
  uvLampTank20OnHour = onHour;
  uvLampTank20OffHour = offHour;
}

uint8_t getUvLampTank20OnHour() {
  return uvLampTank20OnHour;
}

uint8_t getUvLampTank20OffHour() {
  return uvLampTank20OffHour;
}

uint8_t getRelayOnHour() {
  return relayOnHour;
}

uint8_t getRelayTank10OnHour() {
  return relayTank10OnHour;
}

uint8_t getRelayOffHour() {
  return relayOffHour;
}

uint8_t getRelayTank10OffHour() {
  return relayTank10OffHour;
} 