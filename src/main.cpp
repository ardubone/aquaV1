// main.cpp (Atom Lite / ESP32)
#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <ESP32Encoder.h>
#include <WiFi.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include "net.h"

#include "config.h"
#include "logger.h"
#include "web_server.h"
#include "temperature.h"
#include "relay.h"
#include "sensors.h"
#include "time_manager.h"
#include "autofeeder.h"


// Переменные определены в temperature.cpp
extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

RTC_DS1307 rtc;
// Используем переменные из temperature.cpp
extern OneWire oneWire;
extern DallasTemperature sensors;
//ESP32Encoder encoder;
Adafruit_BME280 bme;

void setup()
{
  Serial.begin(115200);
  delay(200);
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50);

  if (!rtc.begin())
  {
    for (int i = 0; i < 5; i++)
    {
      Serial.println(F("RTC not found, retrying..."));
      delay(500);
      if (rtc.begin())
        break;
    }
    if (!rtc.begin())
    {
      Serial.println(F("RTC error!"));
      while (true)
        ;
    }
  }

  if (!rtc.isrunning())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  delay(500);
  delay(500);
  if (!bme.begin(0x76))
  { 
    Serial.println(F("BME280 not found!"));
    while (true)
      ;
  }
  delay(500);
  sensors.begin();
  if (sensors.getDeviceCount() == 0)
  {
    Serial.println(F("No sensors found!"));
    while (true)
      ;
  }

  initRelay();
  initRelayTank10();
  delay(500);
  initWiFi();
  //encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  //encoder.setCount(0);
  setupWebServer();
  initTimeManager();
  
  // Инициализация автокормушки
  initAutoFeeder();
  setupAutoFeederSchedule();
}

void loop()
{
  static unsigned long lastUpdateFast = 0;
  static unsigned long lastUpdateSlow = 0;

  if (millis() - lastUpdateFast > 100)
  {
    lastUpdateFast = millis();
    
    // Обновление автокормушки в быстром цикле
    updateAutoFeeder();
  }

  // Обновляем состояние реле
  DateTime now = rtc.now();
  updateRelay(now);
  updateRelayTank10(now);
  
  handleWebRequests();

  if (millis() - lastUpdateSlow > 2000)
  {
    requestTemperatures();
    updateTemperatureLog();
    updateRoomSensors();

    lastUpdateSlow = millis();
  }
}
