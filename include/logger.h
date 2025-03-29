// logger.h
#pragma once
#include <Arduino.h>
#include <RTClib.h>

#define MAX_LOGS 15

struct LogEntry {
  float innerTemp;
  float outerTemp;
  float roomTemp;
  float roomHumidity;
  float roomPressure;
  DateTime timestamp;
};

extern LogEntry temperatureLogs[MAX_LOGS];
extern byte logCount;

void updateTemperatureLog();
