#pragma once

#include <Arduino.h>
#include <RTClib.h>

// Максимальное количество логов на кормушку (неделя + запас)
#define MAX_FEEDING_LOGS 20

// Тип кормления
enum FeedingType {
    FEEDING_AUTO = 0,    // Автоматическое кормление
    FEEDING_MANUAL = 1   // Ручное кормление
};

// Структура лога кормления
struct FeedingLog {
    DateTime timestamp;              // Дата и время начала кормления
    FeedingType type;                // Тип кормления (авто/ручное)
    DateTime relayOnTime;            // Время включения питания
    DateTime relayOffTime;           // Время выключения питания
    bool limitTriggered;              // Сработал ли концевик
    DateTime limitTriggerTime;        // Время срабатывания концевика
};

// Класс для логирования кормлений
class FeederLogger {
private:
    FeedingLog _logs[MAX_FEEDING_LOGS];
    uint8_t _logCount;
    uint32_t _lastCheckedDayStart;  // Unix timestamp начала последнего проверенного дня

public:
    FeederLogger() : _logCount(0), _lastCheckedDayStart(0) {}

    // Добавить лог кормления
    void addLog(const DateTime& timestamp, FeedingType type, 
                const DateTime& relayOnTime, const DateTime& relayOffTime,
                bool limitTriggered, const DateTime& limitTriggerTime);

    // Очистить старые записи (старше 7 дней)
    void cleanOldLogs(const DateTime& now);

    // Получить количество логов
    uint8_t getLogCount() const { return _logCount; }

    // Получить лог по индексу
    const FeedingLog& getLog(uint8_t index) const;

    // Очистить все логи
    void clearLogs();

    // Ежедневная проверка и очистка (вызывать раз в день)
    void dailyCleanup(const DateTime& now);
};

