// logger.h
#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "temperature.h"

// Константы интервалов
#define SAMPLE_INTERVAL_MS 30000  // 30 секунд - интервал измерений
#define MINUTE_INTERVAL_SECONDS 600  // 10 минут
#define HOUR_INTERVAL_SECONDS 3600  // 1 час

// Конфигурация хранения
#define EEPROM_HOURS_COUNT 24  // Последние 24 часа в EEPROM
#define SPIFFS_MAX_HOURS 72  // До 3 суток в SPIFFS
#define CRITICAL_THRESHOLD_PERCENT 10.0  // Порог критичности (10%)
#define MAX_CRITICAL_LOGS 50  // Максимальное количество критических записей

// Размеры буферов
#define SAMPLES_PER_MINUTE_INTERVAL 20  // 20 измерений по 30 сек = 10 минут
#define MINUTE_INTERVALS_PER_HOUR 6  // 6 интервалов по 10 мин = 1 час

// Устаревшие константы (для обратной совместимости)
#define MAX_LOGS EEPROM_HOURS_COUNT
#define LOG_INTERVAL_MINUTES 10
#define MAX_TEMP_BUFFER SAMPLES_PER_MINUTE_INTERVAL
#define INITIALIZATION_DELAY 15000

// Оптимизированные структуры с упаковкой для экономии памяти
// LogEntry: 5*float(4) + DateTime(4) + uint8_t(1) + bool(1) = 26 байт
// CriticalLogEntry: 4*float(4) + DateTime(4) + uint8_t(1) + float(4) = 25 байт
#pragma pack(push, 1)
struct LogEntry {
    float tank20Temp;
    float tank10Temp;
    float roomTemp;
    float roomHumidity;
    float roomPressure;
    DateTime timestamp;
    uint8_t samplesCount;  // Количество измерений в интервале
    bool isCritical;  // Флаг критической записи
};

struct CriticalLogEntry {
    float tank20Temp;
    float tank10Temp;
    float roomHumidity;
    float previousAvg;  // Предыдущее среднее значение
    DateTime timestamp;
    uint8_t parameterType;  // 0=tank20, 1=tank10, 2=humidity
    float changePercent;  // Процент изменения
};
#pragma pack(pop)

// Буфер для 10-минутных интервалов (20 измерений по 30 сек)
struct MinuteBuffer {
    float tank20Temp[SAMPLES_PER_MINUTE_INTERVAL];
    float tank10Temp[SAMPLES_PER_MINUTE_INTERVAL];
    float roomTemp[SAMPLES_PER_MINUTE_INTERVAL];
    float roomHumidity[SAMPLES_PER_MINUTE_INTERVAL];
    float roomPressure[SAMPLES_PER_MINUTE_INTERVAL];
    uint8_t count;
    DateTime intervalStart;
};

// Буфер для часовых интервалов (6 интервалов по 10 мин)
struct HourBuffer {
    float tank20Temp[MINUTE_INTERVALS_PER_HOUR];
    float tank10Temp[MINUTE_INTERVALS_PER_HOUR];
    float roomTemp[MINUTE_INTERVALS_PER_HOUR];
    float roomHumidity[MINUTE_INTERVALS_PER_HOUR];
    float roomPressure[MINUTE_INTERVALS_PER_HOUR];
    uint8_t count;
    DateTime intervalStart;
};

// Устаревшая структура (для обратной совместимости)
struct TempBuffer {
    float tank20Temp[MAX_TEMP_BUFFER];
    float tank10Temp[MAX_TEMP_BUFFER];
    float roomTemp[MAX_TEMP_BUFFER];
    float roomHumidity[MAX_TEMP_BUFFER];
    float roomPressure[MAX_TEMP_BUFFER];
    uint8_t count;
    DateTime intervalStart;
};

// Основные массивы логов
extern LogEntry temperatureLogs[EEPROM_HOURS_COUNT];
extern byte logCount;
extern bool isInitialized;

// Буферы для агрегации
extern MinuteBuffer minuteBuffer;
extern HourBuffer hourBuffer;

// Критические логи
extern CriticalLogEntry criticalLogs[MAX_CRITICAL_LOGS];
extern byte criticalLogCount;

// Устаревшие переменные (для обратной совместимости)
extern TempBuffer tempBuffer;

// Функции валидации
bool isValidTemperature(float temp);
bool isValidHumidity(float humidity);
bool isValidPressure(float pressure);

// Основные функции логирования
void updateTemperatureLog();
void aggregateToMinute();
void aggregateToHour();
void checkCriticalChanges(float tank20Temp, float tank10Temp, float humidity, DateTime timestamp);
void addCriticalLog(float tank20Temp, float tank10Temp, float humidity, 
                    float previousAvg, DateTime timestamp, uint8_t paramType, float changePercent);
void clearLogs();
LogEntry getLastLog();

// Функции работы с EEPROM
void loadLogsFromEEPROM();
void saveLogsToEEPROM();

// Функции работы с SPIFFS
void initLogger();
void loadLogsFromSPIFFS();
void saveLogsToSPIFFS();
void loadCriticalLogsFromSPIFFS();
void saveCriticalLogsToSPIFFS();

// Функции получения данных
LogEntry* getLogs(uint16_t* count);
CriticalLogEntry* getCriticalLogs(uint16_t* count);
