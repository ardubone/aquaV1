#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <RTClib.h>
#include <functional>

#include "logger.h"

enum Screen
{
  MAIN_MENU,
  LOGS_MENU,
  LOGS_TEXT_MENU,
  LOGS_GRAPH_MENU,
  LOGS_TANK20_TEXT,
  LOGS_TANK10_TEXT,
  LOGS_ROOM_TEXT,
  LOGS_ROOM_PRESSURE,
  LOGS_GRAPH_TANK20,
  LOGS_GRAPH_TANK10,
  LOGS_GRAPH_ROOM,
  LOGS_GRAPH_HUMID,
  LOGS_GRAPH_PRESSURE,
  REALTIME,
  SET_TIME_MENU,
  RELAY_CONTROL_MENU,
  WIFI_STATUS_MENU
};

void initDisplay(LiquidCrystal_I2C *lcdRef, DallasTemperature *sensorRef);
void showScreen(Screen screen);
void updateScreen(Screen screen);
void setRoomData(float temp, float humidity, float pressure);
void drawRoomLogs();
void drawFooter(const __FlashStringHelper *text);
void resetMenuCache();
void drawSetTimeMenu(DateTime &tempTime, int selectedField);
void drawRelayMenu();
void printDate(LiquidCrystal_I2C* lcd, const DateTime& dt);
void printTime(LiquidCrystal_I2C* lcd, const DateTime& dt);
void drawGenericLogs(
  const char* title,
  const char* valuePrefix,
  std::function<void(LogEntry&, String&)> valueExtractor
);
void drawTank20Logs();
void drawTank10Logs();
void drawGraphTank20();
void drawGraphTank10();

float getRoomTemp();
float getRoomHumidity();
float getRoomPressure();
float getTank20Temp();
float getTank10Temp();
