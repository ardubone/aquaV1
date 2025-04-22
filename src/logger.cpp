// logger.cpp
#include "logger.h"
#include <DallasTemperature.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>

#include "config.h"

extern RTC_DS1307 rtc;
extern DallasTemperature sensors;
extern Adafruit_BME280 bme;

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

LogEntry temperatureLogs[MAX_LOGS];
byte logCount = 0;
TempBuffer tempBuffer;
bool isInitialized = false;
static unsigned long lastLogTime = millis();
static const uint16_t EEPROM_LOG_START = 0;

void initLogger() {
    clearLogs();
    isInitialized = true;
}

void saveLogsToEEPROM() {
    uint16_t addr = EEPROM_LOG_START;
    EEPROM.put(addr, logCount);
    addr += sizeof(logCount);
    
    for (byte i = 0; i < logCount; i++) {
        EEPROM.put(addr, temperatureLogs[i]);
        addr += sizeof(LogEntry);
    }
}

void loadLogsFromEEPROM() {
    uint16_t addr = EEPROM_LOG_START;
    EEPROM.get(addr, logCount);
    addr += sizeof(logCount);
    
    for (byte i = 0; i < logCount; i++) {
        EEPROM.get(addr, temperatureLogs[i]);
        addr += sizeof(LogEntry);
    }
}

bool isValidTemperature(float temp) {
    return temp > -50 && temp < 100;
}

bool isValidHumidity(float humidity) {
    return humidity >= 0 && humidity <= 100;
}

bool isValidPressure(float pressure) {
    return pressure > 0 && pressure < 2000;
}

void aggregateLogs() {
    if (tempBuffer.count == 0) return;

    LogEntry newLog;
    newLog.timestamp = tempBuffer.intervalStart;
    newLog.samplesCount = tempBuffer.count;

    // Средние значения
    float sumTank20 = 0, sumTank10 = 0;
    int validTank20 = 0, validTank10 = 0;

    for (int i = 0; i < tempBuffer.count; i++) {
        if (isValidTemperature(tempBuffer.tank20Temp[i])) {
            sumTank20 += tempBuffer.tank20Temp[i];
            validTank20++;
        }
        if (isValidTemperature(tempBuffer.tank10Temp[i])) {
            sumTank10 += tempBuffer.tank10Temp[i];
            validTank10++;
        }
    }

    newLog.tank20Temp = validTank20 > 0 ? sumTank20 / validTank20 : NAN;
    newLog.tank10Temp = validTank10 > 0 ? sumTank10 / validTank10 : NAN;

    // Добавляем в лог
    if (logCount >= MAX_LOGS) {
        for (int i = 1; i < MAX_LOGS; i++) {
            temperatureLogs[i-1] = temperatureLogs[i];
        }
        logCount--;
    }
    temperatureLogs[logCount++] = newLog;

    // Очищаем буфер
    tempBuffer.count = 0;
}

void updateTemperatureLog() {
    if (!isInitialized) return;

    DateTime now = rtc.now();
    
    if (tempBuffer.count == 0) {
        tempBuffer.intervalStart = now;
    }

    float tank20Temp = getTank20Temperature();
    float tank10Temp = getTank10Temperature();

    if (isValidTemperature(tank20Temp)) {
        tempBuffer.tank20Temp[tempBuffer.count] = tank20Temp;
    }
    if (isValidTemperature(tank10Temp)) {
        tempBuffer.tank10Temp[tempBuffer.count] = tank10Temp;
    }

    tempBuffer.count++;

    if (tempBuffer.count >= MAX_TEMP_BUFFER || 
        (now.unixtime() - tempBuffer.intervalStart.unixtime()) >= LOG_INTERVAL_MINUTES * 60) {
        aggregateLogs();
    }
}

void clearLogs() {
    logCount = 0;
    tempBuffer.count = 0;
}

LogEntry getLastLog() {
    return logCount > 0 ? temperatureLogs[logCount-1] : LogEntry();
}
