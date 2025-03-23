#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Encoder.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>

#include <display.h>
#include <logger.h>
#include <controls.h>

RTC_DS1307 rtc;

#define ONE_WIRE_BUS 7
#define ENCODER_CLK 2
#define ENCODER_DT 3
#define BUTTON_PIN 4
#define DHTPIN 6
#define DHTTYPE DHT11

DeviceAddress innerSensorAddr = {0x28, 0x80, 0xF2, 0x53, 0x00, 0x00, 0x00, 0x81};
DeviceAddress outerSensorAddr = {0x28, 0x29, 0x1F, 0x52, 0x00, 0x00, 0x00, 0xF2};

LiquidCrystal_I2C lcd(0x27, 20, 4);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Encoder encoder(ENCODER_CLK, ENCODER_DT);
DHT dht(DHTPIN, DHTTYPE);
// float roomTemp = 0.0;
// float roomHumidity = 0.0;

Screen currentScreen = MAIN_MENU;

// bool buttonPressed = false;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println(F("Setup started"));
  lcd.init();
  lcd.backlight();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  dht.begin();
  sensors.begin();

  Wire.begin();

  if (!rtc.begin())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC not found!");
    while (true)
      ; // остановка
  }

  if (!rtc.isrunning())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // установить из времени компиляции
  }

  initDisplay(&lcd, &sensors);
  showScreen(currentScreen);
}

void loop()
{
  handleEncoder(currentScreen);
  handleButton(currentScreen);
  updateTemperatureLog();
  updateScreen(currentScreen);
  setRoomData(dht.readTemperature(), dht.readHumidity());
  //   Serial.print(F("Loop screen: "));
  // Serial.println((int)currentScreen);

  // roomTemp = dht.readTemperature();
  // roomHumidity = dht.readHumidity();
  // setRoomData(roomTemp, roomHumidity);
}
