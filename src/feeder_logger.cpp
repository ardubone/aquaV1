#include "feeder_logger.h"

void FeederLogger::addLog(const DateTime& timestamp, FeedingType type,
                         const DateTime& relayOnTime, const DateTime& relayOffTime,
                         bool limitTriggered, const DateTime& limitTriggerTime) {
    if (_logCount >= MAX_FEEDING_LOGS) {
        // Сдвигаем все записи влево, удаляя самую старую
        for (uint8_t i = 1; i < MAX_FEEDING_LOGS; i++) {
            _logs[i - 1] = _logs[i];
        }
        _logCount = MAX_FEEDING_LOGS - 1;
    }

    FeedingLog& newLog = _logs[_logCount];
    newLog.timestamp = timestamp;
    newLog.type = type;
    newLog.relayOnTime = relayOnTime;
    newLog.relayOffTime = relayOffTime;
    newLog.limitTriggered = limitTriggered;
    newLog.limitTriggerTime = limitTriggerTime;

    _logCount++;
}

void FeederLogger::cleanOldLogs(const DateTime& now) {
    uint8_t writeIndex = 0;
    const uint32_t sevenDaysSeconds = 7 * 24 * 60 * 60; // 7 дней в секундах
    uint32_t nowUnix = now.unixtime();

    for (uint8_t i = 0; i < _logCount; i++) {
        uint32_t logUnix = _logs[i].timestamp.unixtime();
        if (nowUnix - logUnix < sevenDaysSeconds) {
            // Лог не старше 7 дней, сохраняем его
            if (writeIndex != i) {
                _logs[writeIndex] = _logs[i];
            }
            writeIndex++;
        }
    }

    _logCount = writeIndex;
}

const FeedingLog& FeederLogger::getLog(uint8_t index) const {
    if (index >= _logCount) {
        static FeedingLog emptyLog = {};
        return emptyLog;
    }
    return _logs[index];
}

void FeederLogger::clearLogs() {
    _logCount = 0;
}

void FeederLogger::dailyCleanup(const DateTime& now) {
    uint8_t currentDay = now.day();
    
    // Проверяем, сменился ли день
    if (_lastCheckedDay != currentDay) {
        _lastCheckedDay = currentDay;
        cleanOldLogs(now);
    }
}

