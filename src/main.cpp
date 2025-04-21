// main.cpp (Atom Lite / ESP32)
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include "net.h"

#include "config.h"
#include "display.h"
#include "logger.h"
#include "controls.h"
#include "web_server.h"


DeviceAddress tank20SensorAddr = {0x28, 0x80, 0xF2, 0x53, 0x00, 0x00, 0x00, 0x81};
DeviceAddress tank10SensorAddr = {0x28, 0x29, 0x1F, 0x52, 0x00, 0x00, 0x00, 0xF2};

// Инициализация времени реле
uint8_t relayOnHour = 8;
uint8_t relayOffHour = 19;

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
  Serial.println("Starting setup...");
  delay(200);
  
  Serial.println("Initializing pins...");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  Serial.println("Initializing I2C...");
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50);

  Serial.println("Initializing RTC...");
  if (!rtc.begin())
  {
    Serial.println("RTC not found!");
    while (true) {
      Serial.println("RTC error, retrying...");
      delay(1000);
    }
  }
  
  if (!rtc.isrunning())
  {
    Serial.println("RTC not running, adjusting time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  else
  {
    Serial.println("RTC is running, adjusting time to computer time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Initializing BME280...");
  if (!bme.begin(0x76))
  { 
    Serial.println("BME280 not found!");
    while (true) {
      Serial.println("BME280 error, retrying...");
      delay(1000);
    }
  }

  Serial.println("Initializing temperature sensors...");
  sensors.begin();
  
  #ifdef DEBUG_MODE
  Serial.println("DEBUG MODE: Using mock sensor data");
  #else
  if (sensors.getDeviceCount() == 0)
  {
    Serial.println("No temperature sensors found!");
    while (true) {
      Serial.println("No sensors, retrying...");
      delay(1000);
    }
  }
  #endif
  
  Serial.println("Initializing WiFi...");
  initWiFi();
  
  Serial.println("Initializing encoder...");
  encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  encoder.setCount(0);
  
  Serial.println("Initializing web server...");
  setupWebServer();
  
  Serial.println("Setup completed successfully");
}

void loop()
{
  static unsigned long lastUpdateFast = 0;
  static unsigned long lastUpdateSlow = 0;

  if (millis() - lastUpdateFast > 100)
  {
    handleEncoder(currentScreen);
    handleButton(currentScreen);
    lastUpdateFast = millis();
  }

  if (currentScreen != SET_TIME_MENU && !relayManualOverride) {
    DateTime now = rtc.now();
    int hour = now.hour();
    bool shouldBeOn = (hour >= relayOnHour && hour < relayOffHour);

    if (relayState != shouldBeOn) {
      relayState = shouldBeOn;
      digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
      lastRelayToggleTime = now;
      Serial.printf("Relay state changed to: %s (Time: %02d:%02d)\n", 
                   relayState ? "ON" : "OFF", 
                   now.hour(), 
                   now.minute());
    }
  }

  static int lastCheckedDay = -1;
  DateTime now = rtc.now();

  if (now.day() != lastCheckedDay) {
    lastCheckedDay = now.day();
    relayManualOverride = false;
    Serial.println("Day changed, resetting relay override");
  }

  handleWebRequests();

  if (millis() - lastUpdateSlow > 2000)
  {
    #ifdef DEBUG_MODE
    // Инициализируем мок-данные для логов
    if (logCount == 0) {
        LogEntry mockLog = {
            .tank20Temp = 25.0,
            .tank10Temp = 24.0,
            .roomTemp = bme.readTemperature(),
            .roomHumidity = bme.readHumidity(),
            .roomPressure = (bme.readPressure() / 100.0) * 0.75006,
            .timestamp = rtc.now(),
            .samplesCount = 1
        };
        temperatureLogs[0] = mockLog;
        logCount = 1;
    }
    
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = (bme.readPressure() / 100.0) * 0.75006;
    setRoomData(temp, hum, pres);
    #else
    updateTemperatureLog();
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = (bme.readPressure() / 100.0) * 0.75006;
    setRoomData(temp, hum, pres);
    #endif

    lastUpdateSlow = millis();
  }
}
