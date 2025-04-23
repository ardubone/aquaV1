#include "relay.h"
#include "config.h"

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

void initRelay() {
  pinMode(RELAY_PIN_TANK20, OUTPUT);
  digitalWrite(RELAY_PIN_TANK20, LOW);
}

void initRelayTank10() {
  pinMode(RELAY_PIN_TANK10, OUTPUT);
  digitalWrite(RELAY_PIN_TANK10, LOW);
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

void resetRelayOverride() {
  relayManualOverride = false;
}

void resetRelayTank10Override() {
  relayTank10ManualOverride = false;
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

void setRelayTimes(uint8_t onHour, uint8_t offHour) {
  relayOnHour = onHour;
  relayOffHour = offHour;
}

void setRelayTank10Times(uint8_t onHour, uint8_t offHour) {
  relayTank10OnHour = onHour;
  relayTank10OffHour = offHour;
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