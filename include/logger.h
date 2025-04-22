// logger.h
#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "temperature.h"

#define MAX_LOGS 144  // 24 часа * 6 интервалов в час
#define LOG_INTERVAL_MINUTES 10
#define MAX_TEMP_BUFFER 10  // Максимальное количество логов для агрегации
#define INITIALIZATION_DELAY 15000  // 15 секунд задержки при старте

struct LogEntry {
    float tank20Temp;
    float tank10Temp;
    float roomTemp;
    float roomHumidity;
    float roomPressure;
    DateTime timestamp;
    uint8_t samplesCount;  // Количество измерений в интервале
};

struct TempBuffer {
    float tank20Temp[MAX_TEMP_BUFFER];
    float tank10Temp[MAX_TEMP_BUFFER];
    float roomTemp[MAX_TEMP_BUFFER];
    float roomHumidity[MAX_TEMP_BUFFER];
    float roomPressure[MAX_TEMP_BUFFER];
    uint8_t count;
    DateTime intervalStart;
};

extern LogEntry temperatureLogs[MAX_LOGS];
extern byte logCount;
extern TempBuffer tempBuffer;
extern bool isInitialized;

bool isValidTemperature(float temp);
bool isValidHumidity(float humidity);
bool isValidPressure(float pressure);

void updateTemperatureLog();
void clearLogs();
LogEntry getLastLog();
void loadLogsFromEEPROM();
void aggregateLogs();
void initLogger();
