// main.cpp (Atom Lite / ESP32)
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Encoder.h>
#include <DHT.h>
#include <RTClib.h>

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
DHT dht(DHTPIN, DHTTYPE);
Screen currentScreen = MAIN_MENU;

void setup()
{
  Serial.begin(115200);
  delay(200);
  //Serial.println(F("Setup started"));
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
  dht.begin();
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
  delay(500);
  encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  encoder.setCount(0);
  initDisplay(&lcd, &sensors);
  showScreen(currentScreen);
  //Serial.println(F("all init"));
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

  if (millis() - lastUpdateSlow > 2000)
  {
    updateTemperatureLog();
    setRoomData(dht.readTemperature(), dht.readHumidity());
    lastUpdateSlow = millis();
  }
}
