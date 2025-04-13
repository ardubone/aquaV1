// logger.cpp
#include "logger.h"
#include <DallasTemperature.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>

#include "display.h"
#include "config.h"

extern RTC_DS1307 rtc;
extern DallasTemperature sensors;
extern Adafruit_BME280 bme;

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

LogEntry temperatureLogs[MAX_LOGS];
byte logCount = 0;
TempBuffer tempBuffer = {0};
bool isInitialized = false;
static unsigned long lastLogTime = millis();
static const uint16_t EEPROM_LOG_START = 0;

void initLogger() {
    isInitialized = false;
    delay(INITIALIZATION_DELAY);
    
    // Проверяем датчики несколько раз
    for (int i = 0; i < 3; i++) {
        sensors.requestTemperatures();
        float tank20Temp = sensors.getTempC(tank20SensorAddr);
        float tank10Temp = sensors.getTempC(tank10SensorAddr);
        
        if (isValidTemperature(tank20Temp) && isValidTemperature(tank10Temp)) {
            isInitialized = true;
            break;
        }
        delay(1000);
    }
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
    return temp > -50.0f && temp < 85.0f && temp != -127.0f; // -127.0f - значение при ошибке датчика, 85.0f - максимальная рабочая температура
}

bool isValidHumidity(float humidity) {
    return humidity >= 0.0f && humidity <= 100.0f;
}

bool isValidPressure(float pressure) {
    return pressure > 800.0f && pressure < 1200.0f;
}

void aggregateLogs() {
    if (tempBuffer.count == 0) return;

    float avgTank20Temp = 0;
    float avgTank10Temp = 0;
    float avgRoomTemp = 0;
    float avgRoomHumidity = 0;
    float avgRoomPressure = 0;

    for (uint8_t i = 0; i < tempBuffer.count; i++) {
        avgTank20Temp += tempBuffer.tank20Temp[i];
        avgTank10Temp += tempBuffer.tank10Temp[i];
        avgRoomTemp += tempBuffer.roomTemp[i];
        avgRoomHumidity += tempBuffer.roomHumidity[i];
        avgRoomPressure += tempBuffer.roomPressure[i];
    }

    avgTank20Temp /= tempBuffer.count;
    avgTank10Temp /= tempBuffer.count;
    avgRoomTemp /= tempBuffer.count;
    avgRoomHumidity /= tempBuffer.count;
    avgRoomPressure /= tempBuffer.count;

    if (logCount >= MAX_LOGS) {
        memmove(&temperatureLogs[0], &temperatureLogs[1], sizeof(LogEntry) * (MAX_LOGS - 1));
        logCount = MAX_LOGS - 1;
    }

    temperatureLogs[logCount] = {
        avgTank20Temp,
        avgTank10Temp,
        avgRoomTemp,
        avgRoomHumidity,
        avgRoomPressure,
        tempBuffer.intervalStart,
        tempBuffer.count
    };
    
    logCount++;
    tempBuffer.count = 0;
    saveLogsToEEPROM();
}

void updateTemperatureLog() {
    if (!isInitialized) {
        initLogger();
        return;
    }

    if (millis() - lastLogTime < 30000) { // Каждые 30 секунд
        return;
    }

    sensors.requestTemperatures();
    float tank20Temp = sensors.getTempC(tank20SensorAddr);
    float tank10Temp = sensors.getTempC(tank10SensorAddr);
    float roomTemp = getRoomTemp();
    float roomHumidity = getRoomHumidity();
    float roomPressure = getRoomPressure();

    if (!isValidTemperature(tank20Temp) || !isValidTemperature(tank10Temp) || 
        !isValidTemperature(roomTemp) || !isValidHumidity(roomHumidity) || 
        !isValidPressure(roomPressure)) {
        return;
    }

    DateTime now = rtc.now();
    
    if (tempBuffer.count == 0) {
        tempBuffer.intervalStart = now;
    }

    if (tempBuffer.count < MAX_TEMP_BUFFER) {
        tempBuffer.tank20Temp[tempBuffer.count] = tank20Temp;
        tempBuffer.tank10Temp[tempBuffer.count] = tank10Temp;
        tempBuffer.roomTemp[tempBuffer.count] = roomTemp;
        tempBuffer.roomHumidity[tempBuffer.count] = roomHumidity;
        tempBuffer.roomPressure[tempBuffer.count] = roomPressure;
        tempBuffer.count++;
    }

    // Проверяем, прошло ли 10 минут с начала интервала
    if ((now.unixtime() - tempBuffer.intervalStart.unixtime()) >= (LOG_INTERVAL_MINUTES * 60)) {
        aggregateLogs();
    }

    lastLogTime = millis();
}

void clearLogs() {
    logCount = 0;
    tempBuffer.count = 0;
    memset(temperatureLogs, 0, sizeof(temperatureLogs));
    saveLogsToEEPROM();
}

LogEntry getLastLog() {
    if (logCount == 0) {
        DateTime emptyTime(0, 0, 0, 0, 0, 0);
        return LogEntry{0, 0, 0, 0, 0, emptyTime, 0};
    }
    return temperatureLogs[logCount - 1];
}
