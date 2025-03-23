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

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("Setup started"));

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);

  lcd.init();
  lcd.backlight();

  dht.begin();
  sensors.begin();

  if (!rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC not found!");
    while (true);
  }
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);


  encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  encoder.setCount(0);

  initDisplay(&lcd, &sensors);
  showScreen(currentScreen);
}

void loop() {
  handleEncoder(currentScreen);
  handleButton(currentScreen);
  updateTemperatureLog();
  updateScreen(currentScreen);
  setRoomData(dht.readTemperature(), dht.readHumidity());
}


