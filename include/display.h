#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <RTClib.h>

enum Screen {
  MAIN_MENU,
  REALTIME,
  LOGS_MENU,
  LOGS_TEXT_MENU,
  LOGS_GRAPH_MENU,
  LOGS_INNER_TEXT,
  LOGS_OUTER_TEXT,
  LOGS_ROOM_TEXT,
  LOGS_ROOM_PRESSURE,
  LOGS_GRAPH_INNER,
  LOGS_GRAPH_OUTER,
  LOGS_GRAPH_ROOM,
  LOGS_GRAPH_HUMID,
  LOGS_GRAPH_PRESSURE,
  RELAY_CONTROL_MENU,
  SET_TIME_MENU
};

void initDisplay(LiquidCrystal_I2C* lcdRef, DallasTemperature* sensorRef);
void showScreen(Screen screen);
void updateScreen(Screen screen);
void setRoomData(float temp, float humidity, float pressure);
void drawInnerLogs();
void drawOuterLogs();
void drawRoomLogs();
void drawFooter(const __FlashStringHelper* text);
void resetMenuCache();
void drawSetTimeMenu(DateTime &tempTime, int selectedField);
void drawRelayMenu();

float getRoomTemp();
float getRoomHumidity();
float getRoomPressure();
