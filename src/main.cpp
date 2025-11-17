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
#include "pcf8574_manager.h"

#include "config.h"
#include "logger.h"
#include "web_server.h"
#include "temperature.h"
#include "relay.h"
#include "sensors.h"
#include "time_manager.h"
#include "autofeeder.h"
#include "camera.h"
#include <EEPROM.h>

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

// Переменные определены в temperature.cpp
extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

RTC_DS1307 rtc;
// Используем переменные из temperature.cpp
extern OneWire oneWire;
extern DallasTemperature sensors;
//ESP32Encoder encoder;
Adafruit_BME280 bme;

// Глобальный объект PCF8574
PCF8574Manager pcfManager;

// Флаги состояния компонентов
bool isRtcInitialized = false;
bool isBme280Initialized = false;
bool isDs18b20Initialized = false;
bool isPcf8574Initialized = false;
bool isAutoFeederInitialized = false;
bool isCameraInitialized = false;

void setup()
{
  Serial.begin(115200);
  delay(200);
  initWiFi();
  Serial.println(F("WiFi connected."));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  delay(200);
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50);

  // Инициализация камеры
  if (!initCamera()) {
#ifdef DEBUG_MODE
    Serial.println(F("Camera initialization failed - continuing in DEBUG_MODE"));
    isCameraInitialized = false;
#else
    Serial.println(F("Camera initialization failed!"));
    // Продолжаем работу без камеры
    isCameraInitialized = false;
#endif
  }
  delay(500);

  // Инициализация RTC
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
#ifdef DEBUG_MODE
      Serial.println(F("RTC not found - using mock time in DEBUG_MODE"));
      isRtcInitialized = false;
#else
      Serial.println(F("RTC error!"));
      while (true)
        ;
#endif
    }
    else
    {
      isRtcInitialized = true;
    }
  }
  else
  {
    isRtcInitialized = true;
  }

  if (isRtcInitialized && !rtc.isrunning())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  delay(500);
  
  // Инициализация BME280
  if (!bme.begin(0x76))
  { 
#ifdef DEBUG_MODE
    Serial.println(F("BME280 not found - using mock data in DEBUG_MODE"));
    isBme280Initialized = false;
#else
    Serial.println(F("BME280 not found!"));
    while (true)
      ;
#endif
  }
  else
  {
    isBme280Initialized = true;
  }
  delay(500);
  
  // Инициализация DS18B20
  sensors.begin();
  if (sensors.getDeviceCount() == 0)
  {
#ifdef DEBUG_MODE
    Serial.println(F("DS18B20 sensors not found - using mock temperatures in DEBUG_MODE"));
    isDs18b20Initialized = false;
#else
    Serial.println(F("No sensors found!"));
    //while (true);
#endif
  }
  else
  {
    isDs18b20Initialized = true;
  }

  // Инициализация PCF8574
  if (!pcfManager.begin()) {
#ifdef DEBUG_MODE
    Serial.println(F("PCF8574 initialization failed - continuing in DEBUG_MODE"));
    isPcf8574Initialized = false;
#else
    Serial.println(F("PCF8574 initialization failed!"));
    while (true)
      ;
#endif
  }
  else
  {
    isPcf8574Initialized = true;
  }

  initRelay();
  initRelayTank10();
  initUvLampTank10();
  initUvLampTank20();
  // delay(500);
  // initWiFi();
  // Serial.println(F("WiFi connected." + String(WiFi.localIP())));
  //encoder.attachHalfQuad(ENCODER_CLK, ENCODER_DT);
  //encoder.setCount(0);
  setupWebServer();
  initTimeManager();
  
  // Инициализация EEPROM и логгера
  if (!EEPROM.begin(512)) {
#ifdef DEBUG_MODE
    Serial.println(F("EEPROM initialization failed - continuing in DEBUG_MODE"));
    initLogger(); // Инициализируем без загрузки из EEPROM
#else
    Serial.println(F("EEPROM initialization failed!"));
    initLogger(); // Инициализируем без загрузки из EEPROM
#endif
  } else {
    loadLogsFromEEPROM(); // Загружаем логи из EEPROM
    initLogger(); // Инициализируем логгер (не очищает загруженные логи)
  }
  
  // Инициализация автокормушек
#ifdef DEBUG_MODE
  // В DEBUG_MODE можем работать без RTC
  if (isPcf8574Initialized) {
    initAutoFeeder(&pcfManager, isRtcInitialized ? &rtc : nullptr);
    setupDefaultFeederSchedule();  // Загружаем дефолтное расписание
    isAutoFeederInitialized = true;
    if (!isRtcInitialized) {
      Serial.println(F("Автокормушки инициализированы в DEBUG_MODE без RTC"));
    }
  } else {
    Serial.println(F("Автокормушки не инициализированы - отсутствует PCF8574"));
    isAutoFeederInitialized = false;
  }
#else
  if (isPcf8574Initialized && isRtcInitialized) {
    initAutoFeeder(&pcfManager, &rtc);
    setupDefaultFeederSchedule();  // Загружаем дефолтное расписание
    isAutoFeederInitialized = true;
  } else {
    Serial.println(F("Автокормушки не инициализированы - отсутствуют PCF8574 или RTC"));
    isAutoFeederInitialized = false;
  }
#endif
}

void loop()
{
  static unsigned long lastUpdateFast = 0;
  static unsigned long lastUpdateSlow = 0;

  if (millis() - lastUpdateFast > 100)
  {
    lastUpdateFast = millis();
    
    // Обновление состояния PCF8574
    if (isPcf8574Initialized) {
      pcfManager.update();
    }
    
    // Обновление автокормушек в быстром цикле
    if (isAutoFeederInitialized) {
      updateAutoFeeder();
    }
  }

  // Обновляем состояние реле
#ifdef DEBUG_MODE
  DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
  DateTime now = rtc.now();
#endif
  updateRelay(now);
  updateRelayTank10(now);
  updateUvLampTank10(now);
  updateUvLampTank20(now);
  
  handleWebRequests();

  // Обновление датчиков каждые 2 секунды (для быстрого отклика)
  if (millis() - lastUpdateSlow > 2000)
  {
    requestTemperatures();
    updateRoomSensors();
    lastUpdateSlow = millis();
  }
  
  // Обновление логирования каждые 30 секунд
  static unsigned long lastLogUpdate = 0;
  if (millis() - lastLogUpdate > 30000)
  {
    updateTemperatureLog();
    lastLogUpdate = millis();
  }
}
