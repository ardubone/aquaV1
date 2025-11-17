#include "sensors.h"
#include <Adafruit_BME280.h>
#include "config.h"

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

// Данные с датчиков комнаты
static float roomTemp = 0.0;
static float roomHumidity = 0.0;
static float roomPressure = 0.0;

extern Adafruit_BME280 bme;

void initRoomSensors() {
  // Инициализация BME280 уже происходит в main.cpp
}

float getRoomTemp() {
#ifdef DEBUG_MODE
  if (!isBme280Initialized) {
    return getMockRoomTemp();
  }
#endif
  return roomTemp;
}

float getRoomHumidity() {
#ifdef DEBUG_MODE
  if (!isBme280Initialized) {
    return getMockRoomHumidity();
  }
#endif
  return roomHumidity;
}

float getRoomPressure() {
#ifdef DEBUG_MODE
  if (!isBme280Initialized) {
    return getMockRoomPressure();
  }
#endif
  return roomPressure;
}

void updateRoomSensors() {
#ifdef DEBUG_MODE
  if (!isBme280Initialized) {
    roomTemp = getMockRoomTemp();
    roomHumidity = getMockRoomHumidity();
    roomPressure = getMockRoomPressure();
    return;
  }
#endif
  roomTemp = bme.readTemperature();
  roomHumidity = bme.readHumidity();
  // Преобразование давления из Па в мм рт.ст.
  roomPressure = (bme.readPressure() / 100.0) * 0.75006;
} 