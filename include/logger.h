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
#define EEPROM_SIZE 1024  // Размер EEPROM в байтах
#define EEPROM_HOURS_COUNT 24  // Последние 24 часа в EEPROM
#define SPIFFS_MAX_HOURS 72  // До 3 суток в SPIFFS
#define CRITICAL_THRESHOLD_PERCENT 10.0  // Порог критичности (10%)
#define MAX_CRITICAL_LOGS 50  // Максимальное количество критических записей
#define MINUTE_LOGS_PER_HOUR 6  // 6 интервалов по 10 минут в часе

// Размеры буферов
#define SAMPLES_PER_MINUTE_INTERVAL 20  // 20 измерений по 30 сек = 10 минут
#define MINUTE_INTERVALS_PER_HOUR 6  // 6 интервалов по 10 мин = 1 час

// Устаревшие константы (для обратной совместимости)
#define MAX_LOGS EEPROM_HOURS_COUNT
#define LOG_INTERVAL_MINUTES 10
#define MAX_TEMP_BUFFER SAMPLES_PER_MINUTE_INTERVAL
#define INITIALIZATION_DELAY 15000

// Оптимизированные структуры с упаковкой для экономии памяти
// LogEntry: 5*int16_t(2) + uint16_t(2) + uint16_t(2) + uint32_t(4) + uint8_t(1) = 15 байт
// CriticalLogEntry: 3*int16_t(2) + uint16_t(2) + int16_t(2) + uint32_t(4) + uint8_t(1) + uint16_t(2) = 17 байт
// MinuteLogEntry: аналогична LogEntry, но для 10-минутных интервалов
#pragma pack(push, 1)
struct LogEntry {
    int16_t tankLrgTemp;      // Температура аквариума L * 10 (точность 0.1°C, диапазон -3276.8 до 3276.7°C)
    int16_t tankSmlTemp;      // Температура аквариума S * 10
    int16_t roomTemp;        // Температура * 10
    uint16_t roomHumidity;   // Влажность * 10 (точность 0.1%, диапазон 0-6553.5%)
    uint16_t roomPressure;   // Давление * 10 (точность 0.1 мм рт.ст., диапазон 0-6553.5)
    uint32_t timestamp;      // Unix timestamp
    uint8_t samplesCount;    // Количество измерений в интервале
};

struct MinuteLogEntry {
    int16_t tankLrgTemp;      // Температура аквариума L * 10
    int16_t tankSmlTemp;      // Температура аквариума S * 10
    int16_t roomTemp;        // Температура * 10
    uint16_t roomHumidity;   // Влажность * 10
    uint16_t roomPressure;   // Давление * 10
    uint32_t timestamp;      // Unix timestamp
    uint8_t samplesCount;    // Количество измерений в интервале
};

struct CriticalLogEntry {
    int16_t tankLrgTemp;      // Температура аквариума L * 10 (или NAN если не применимо)
    int16_t tankSmlTemp;      // Температура аквариума S * 10 (или NAN если не применимо)
    uint16_t roomHumidity;   // Влажность * 10 (или NAN если не применимо)
    int16_t previousAvg;     // Предыдущее среднее значение * 10
    uint32_t timestamp;      // Unix timestamp
    uint8_t parameterType;    // 0=аквариум L, 1=аквариум S, 2=humidity
    uint16_t changePercent;  // Процент изменения * 10 (точность 0.1%)
};
#pragma pack(pop)

// Буфер для 10-минутных интервалов (20 измерений по 30 сек)
struct MinuteBuffer {
    float tankLrgTemp[SAMPLES_PER_MINUTE_INTERVAL];  // Температура аквариума L
    float tankSmlTemp[SAMPLES_PER_MINUTE_INTERVAL];  // Температура аквариума S
    float roomTemp[SAMPLES_PER_MINUTE_INTERVAL];
    float roomHumidity[SAMPLES_PER_MINUTE_INTERVAL];
    float roomPressure[SAMPLES_PER_MINUTE_INTERVAL];
    uint8_t count;
    DateTime intervalStart;
};

// Буфер для часовых интервалов (6 интервалов по 10 мин)
struct HourBuffer {
    float tankLrgTemp[MINUTE_INTERVALS_PER_HOUR];  // Температура аквариума L
    float tankSmlTemp[MINUTE_INTERVALS_PER_HOUR];  // Температура аквариума S
    float roomTemp[MINUTE_INTERVALS_PER_HOUR];
    float roomHumidity[MINUTE_INTERVALS_PER_HOUR];
    float roomPressure[MINUTE_INTERVALS_PER_HOUR];
    uint8_t count;
    DateTime intervalStart;
};

// Основные массивы логов
extern LogEntry temperatureLogs[EEPROM_HOURS_COUNT];
extern byte logCount;
extern bool isInitialized;

// 10-минутные интервалы текущего часа (в RAM)
extern MinuteLogEntry minuteLogs[MINUTE_LOGS_PER_HOUR];
extern byte minuteLogCount;
extern uint32_t currentHourStart;  // Unix timestamp начала текущего часа

// Буферы для агрегации
extern MinuteBuffer minuteBuffer;
extern HourBuffer hourBuffer;

// Критические логи
extern CriticalLogEntry criticalLogs[MAX_CRITICAL_LOGS];
extern byte criticalLogCount;

// Функции валидации
bool isValidTemperature(float temp);
bool isValidHumidity(float humidity);
bool isValidPressure(float pressure);

// Основные функции логирования
void updateTemperatureLog();
void aggregateToMinute();
void aggregateToHour();
void checkCriticalChanges(float tankLrgTemp, float tankSmlTemp, float humidity, DateTime timestamp);
void addCriticalLog(float tankLrgTemp, float tankSmlTemp, float humidity, 
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
MinuteLogEntry* getCurrentHourMinuteLogs(uint8_t* count);  // 10-минутные интервалы текущего часа
LogEntry* getLogsFromSPIFFS(uint16_t* count);  // Логи за 72 часа из SPIFFS

// Вспомогательные функции конвертации (inline для доступности везде)
inline float int16ToFloat(int16_t value) {
    const int16_t INT16_NAN = 0x7FFF;
    if (value == INT16_NAN) return NAN;
    return value / 10.0f;
}

inline int16_t floatToInt16(float value) {
    const int16_t INT16_NAN = 0x7FFF;
    if (isnan(value)) return INT16_NAN;
    if (value < -3276.7f) value = -3276.7f;
    if (value > 3276.7f) value = 3276.7f;
    return (int16_t)(value * 10.0f + 0.5f);
}

inline float uint16ToFloat(uint16_t value) {
    const uint16_t UINT16_NAN = 0xFFFF;
    if (value == UINT16_NAN) return NAN;
    return value / 10.0f;
}

inline uint16_t floatToUint16(float value) {
    const uint16_t UINT16_NAN = 0xFFFF;
    if (isnan(value)) return UINT16_NAN;
    if (value < 0) value = 0;
    if (value > 6553.5f) value = 6553.5f;
    return (uint16_t)(value * 10.0f + 0.5f);
}

inline DateTime uint32ToDateTime(uint32_t timestamp) {
    return DateTime(timestamp);
}

inline uint32_t dateTimeToUint32(DateTime dt) {
    return dt.unixtime();
}
