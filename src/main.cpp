// main.cpp (Atom Lite / ESP32)
#include <Arduino.h>
#include <Wire.h>
// #include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include "net.h"

#include "config.h"
// #include "display.h"
#include "logger.h"
#include "controls.h"
#include "web_server.h"
#include "temperature.h"
#include "relay.h"
#include "sensors.h"
#include "time_manager.h"


// Переменные определены в temperature.cpp
extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

RTC_DS1307 rtc;
// Используем переменную lcd из display.cpp
// extern LiquidCrystal_I2C lcd;
// Используем переменные из temperature.cpp
extern OneWire oneWire;
extern DallasTemperature sensors;
ESP32Encoder encoder;
Adafruit_BME280 bme;
// Screen currentScreen = MAIN_MENU;

void setup()
{
  Serial.begin(115200);
  delay(200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  // lcd.init();
  // lcd.print(F("Loading...."));
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
      // lcd.setCursor(0, 0);
      // lcd.print(F("RTC error!"));
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
  // lcd.backlight();
  delay(500);
  if (!bme.begin(0x76))
  { 
    // lcd.clear();
    // lcd.print(F("BME280 not found!"));
    Serial.println(F("BME280 not found!"));
    while (true)
      ;
  }
  delay(500);
  sensors.begin();
  if (sensors.getDeviceCount() == 0)
  {
    // lcd.clear();
    // lcd.print(F("No sensors found!"));
    Serial.println(F("No sensors found!"));
    while (true)
      ;
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  initRelay();
  delay(500);
  initWiFi();
  encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  encoder.setCount(0);
  setupWebServer();
  initTimeManager();

  //initDisplay(&lcd);
  // showScreen(currentScreen);
}

void loop()
{
  static unsigned long lastUpdateFast = 0;
  static unsigned long lastUpdateSlow = 0;

  if (millis() - lastUpdateFast > 100)
  {
    // handleEncoder(currentScreen);
    // handleButton(currentScreen);
    // updateScreen(currentScreen);
    lastUpdateFast = millis();
  }

  // Обновляем состояние реле
  DateTime now = rtc.now();
  updateRelay(now);
  
  handleWebRequests();

  if (millis() - lastUpdateSlow > 2000)
  {
    requestTemperatures();
    updateTemperatureLog();
    updateRoomSensors();

    lastUpdateSlow = millis();
  }
}
