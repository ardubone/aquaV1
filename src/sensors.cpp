#include "sensors.h"
#include <Adafruit_BME280.h>

// Данные с датчиков комнаты
static float roomTemp = 0.0;
static float roomHumidity = 0.0;
static float roomPressure = 0.0;

extern Adafruit_BME280 bme;

void initRoomSensors() {
  // Инициализация BME280 уже происходит в main.cpp
}

float getRoomTemp() {
  return roomTemp;
}

float getRoomHumidity() {
  return roomHumidity;
}

float getRoomPressure() {
  return roomPressure;
}

void updateRoomSensors() {
  roomTemp = bme.readTemperature();
  roomHumidity = bme.readHumidity();
  // Преобразование давления из Па в мм рт.ст.
  roomPressure = (bme.readPressure() / 100.0) * 0.75006;
} 