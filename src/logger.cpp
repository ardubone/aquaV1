// logger.cpp
#include "logger.h"
#include <DallasTemperature.h>
#include <RTClib.h>
#include "display.h" // добавлен include для геттеров

extern RTC_DS1307 rtc;
extern DallasTemperature sensors;

extern DeviceAddress innerSensorAddr;
extern DeviceAddress outerSensorAddr;

LogEntry temperatureLogs[MAX_LOGS];
byte logCount = 0;
static unsigned long lastLogTime = millis();

void updateTemperatureLog() {
  if (millis() - lastLogTime >= 30000) {
    sensors.requestTemperatures();
    float innerTemp = sensors.getTempC(innerSensorAddr);
    float outerTemp = sensors.getTempC(outerSensorAddr);

    if (logCount >= 15) {
      memmove(&temperatureLogs[0], &temperatureLogs[1], sizeof(LogEntry) * (MAX_LOGS - 1));
      logCount = MAX_LOGS - 1;
    }

    DateTime now = rtc.now();
    temperatureLogs[logCount++] = {
      innerTemp,
      outerTemp,
      getRoomTemp(),
      getRoomHumidity(),
      now
    };

    lastLogTime = millis();
  }
}
