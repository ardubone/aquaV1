#include "relay.h"
#include "config.h"

// Переменные состояния реле
static bool relayState = false;
static bool relayManualOverride = false;
static DateTime lastRelayToggleTime = DateTime(2000, 1, 1, 0, 0, 0);

// Настройки времени работы
static uint8_t relayOnHour = 8;
static uint8_t relayOffHour = 19;

void initRelay() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void updateRelay(const DateTime &now) {
  if (!relayManualOverride) {
    int hour = now.hour();
    bool shouldBeOn = (hour >= relayOnHour && hour < relayOffHour);
    
    if (relayState != shouldBeOn) {
      relayState = shouldBeOn;
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
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

void toggleRelay(const DateTime &now) {
  relayState = !relayState;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  lastRelayToggleTime = now;
  relayManualOverride = true;
}

void resetRelayOverride() {
  relayManualOverride = false;
}

bool getRelayState() {
  return relayState;
}

DateTime getLastRelayToggleTime() {
  return lastRelayToggleTime;
}

bool isRelayManualMode() {
  return relayManualOverride;
}

void setRelayTimes(uint8_t onHour, uint8_t offHour) {
  relayOnHour = onHour;
  relayOffHour = offHour;
}

uint8_t getRelayOnHour() {
  return relayOnHour;
}

uint8_t getRelayOffHour() {
  return relayOffHour;
} 