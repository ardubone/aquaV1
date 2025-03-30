// main.cpp (Atom Lite / ESP32)
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Encoder.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include "wifi.h"
#include <WiFi.h>

#include "config.h"
#include "display.h"
#include "logger.h"
#include "controls.h"

DeviceAddress innerSensorAddr = {0x28, 0x80, 0xF2, 0x53, 0x00, 0x00, 0x00, 0x81};
DeviceAddress outerSensorAddr = {0x28, 0x29, 0x1F, 0x52, 0x00, 0x00, 0x00, 0xF2};

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
ESP32Encoder encoder;
Adafruit_BME280 bme;
Screen currentScreen = MAIN_MENU;

void setup()
{
  Serial.begin(115200);
  delay(200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.print(F("Loading...."));
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
      lcd.setCursor(0, 0);
      lcd.print(F("RTC error!"));
      while (true)
        ;
    }
  }

  if (!rtc.isrunning())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  delay(500);
  lcd.backlight();
  delay(500);
  if (!bme.begin(0x76))
  { 
    lcd.clear();
    lcd.print(F("BME280 not found!"));
    while (true)
      ;
  }
  delay(500);
  sensors.begin();
  if (sensors.getDeviceCount() == 0)
  {
    lcd.clear();
    lcd.print(F("No sensors found!"));
    while (true)
      ;
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Начально выключено
  delay(500);
  initWiFi();
  encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  encoder.setCount(0);
  initDisplay(&lcd, &sensors);
  showScreen(currentScreen);
}

void loop()
{
  static unsigned long lastUpdateFast = 0;
  static unsigned long lastUpdateSlow = 0;

  if (millis() - lastUpdateFast > 100)
  {
    handleEncoder(currentScreen);
    handleButton(currentScreen);
    updateScreen(currentScreen);
    lastUpdateFast = millis();
  }

if (currentScreen != SET_TIME_MENU && !relayManualOverride) {
  DateTime now = rtc.now();
  int hour = now.hour();

  bool shouldBeOn = (hour >= 8 && hour < 19);

  if (relayState != shouldBeOn) {
    relayState = shouldBeOn;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    lastRelayToggleTime = now;
  }
}

static int lastCheckedDay = -1;
DateTime now = rtc.now();

if (now.day() != lastCheckedDay) {
  lastCheckedDay = now.day();
  relayManualOverride = false;  // Сброс каждый день в полночь
}



  if (millis() - lastUpdateSlow > 2000)
  {
    updateTemperatureLog();
    setRoomData(bme.readTemperature(), bme.readHumidity(), (bme.readPressure() / 100.0) * 0.75006);

    lastUpdateSlow = millis();
  }
}
