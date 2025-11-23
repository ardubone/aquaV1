// logger.cpp
#include "logger.h"
#include <DallasTemperature.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <FS.h>
#include <string.h>

#include "config.h"
#include "sensors.h"

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

extern RTC_DS1307 rtc;
extern DallasTemperature sensors;
extern Adafruit_BME280 bme;

extern DeviceAddress tank20SensorAddr;
extern DeviceAddress tank10SensorAddr;

// Основные массивы логов
LogEntry temperatureLogs[EEPROM_HOURS_COUNT];
byte logCount = 0;
bool isInitialized = false;

// 10-минутные интервалы текущего часа (в RAM)
MinuteLogEntry minuteLogs[MINUTE_LOGS_PER_HOUR];
byte minuteLogCount = 0;
uint32_t currentHourStart = 0;  // Unix timestamp начала текущего часа

// Буферы для агрегации
MinuteBuffer minuteBuffer;
HourBuffer hourBuffer;

// Критические логи
CriticalLogEntry criticalLogs[MAX_CRITICAL_LOGS];
byte criticalLogCount = 0;

// Статический массив для хранения логов из SPIFFS (72 часа)
static LogEntry spiffsLogs[SPIFFS_MAX_HOURS];
static byte spiffsLogCount = 0;


// Внутренние переменные
static unsigned long lastSampleTime = 0;
static const uint16_t EEPROM_LOG_START = 0;
static bool spiffsInitialized = false;
static int16_t lastHourAvgTankLrg = 0x7FFF;  // NAN значение для int16_t (аквариум L)
static int16_t lastHourAvgTankSml = 0x7FFF;  // NAN значение для int16_t (аквариум S)
static uint16_t lastHourAvgHumidity = 0xFFFF;  // NAN значение для uint16_t

// Константы для NAN значений
#define INT16_NAN 0x7FFF
#define UINT16_NAN 0xFFFF

// Пути к файлам SPIFFS (SPIFFS не поддерживает директории)
const char* SPIFFS_HOURLY_FILE = "/hourly_logs.bin";
const char* SPIFFS_CRITICAL_FILE = "/critical_logs.bin";

void initLogger() {
    Serial.println(F("[LOGGER] Инициализация системы логирования"));
    
    // Инициализация SPIFFS
    // Пытаемся смонтировать SPIFFS
    if (!SPIFFS.begin(false)) {
        Serial.println(F("[LOGGER] Ошибка монтирования SPIFFS, пытаемся отформатировать..."));
        // Если монтирование не удалось, форматируем
        if (SPIFFS.format()) {
            Serial.println(F("[LOGGER] SPIFFS отформатирован"));
            // Пытаемся смонтировать снова
            if (SPIFFS.begin(false)) {
                Serial.println(F("[LOGGER] SPIFFS успешно смонтирован после форматирования"));
                spiffsInitialized = true;
            } else {
                Serial.println(F("[LOGGER] КРИТИЧЕСКАЯ ОШИБКА: SPIFFS не может быть смонтирован"));
                spiffsInitialized = false;
            }
        } else {
            Serial.println(F("[LOGGER] КРИТИЧЕСКАЯ ОШИБКА: Не удалось отформатировать SPIFFS"));
            spiffsInitialized = false;
        }
    } else {
        Serial.println(F("[LOGGER] SPIFFS успешно смонтирован"));
        spiffsInitialized = true;
    }
    
    // Загружаем данные из EEPROM
    loadLogsFromEEPROM();
    
    // Загружаем данные из SPIFFS
    if (spiffsInitialized) {
        loadLogsFromSPIFFS();
        loadCriticalLogsFromSPIFFS();
    }
    
    // Инициализация буферов
    minuteBuffer.count = 0;
    hourBuffer.count = 0;
    minuteLogCount = 0;
    currentHourStart = 0;
    
    isInitialized = true;
    Serial.println(F("[LOGGER] Система логирования инициализирована"));
}

// Функции конвертации теперь inline в logger.h

bool isValidTemperature(float temp) {
    return !isnan(temp) && temp > -50 && temp < 100;
}

bool isValidHumidity(float humidity) {
    return !isnan(humidity) && humidity >= 0 && humidity <= 100;
}

bool isValidPressure(float pressure) {
    return !isnan(pressure) && pressure > 0 && pressure < 2000;
}

void updateTemperatureLog() {
    if (!isInitialized) return;
    
    unsigned long currentTime = millis();
    
    // Проверяем, прошло ли 30 секунд с последнего измерения
    if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) {
        return;
    }
    lastSampleTime = currentTime;
    
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    
    // Инициализация буфера, если он пуст
    if (minuteBuffer.count == 0) {
        minuteBuffer.intervalStart = now;
    }
    
    // Получаем данные с датчиков
    float tankLrgTemp = getLrgTemperature();
    float tankSmlTemp = getSmlTemperature();
    float roomTemp = getRoomTemp();
    float roomHumidity = getRoomHumidity();
    float roomPressure = getRoomPressure();
    
    // Добавляем данные в буфер 30-секундных измерений
    if (minuteBuffer.count < SAMPLES_PER_MINUTE_INTERVAL) {
        if (isValidTemperature(tankLrgTemp)) {
            minuteBuffer.tankLrgTemp[minuteBuffer.count] = tankLrgTemp;
        } else {
            minuteBuffer.tankLrgTemp[minuteBuffer.count] = NAN;
        }
        
        if (isValidTemperature(tankSmlTemp)) {
            minuteBuffer.tankSmlTemp[minuteBuffer.count] = tankSmlTemp;
        } else {
            minuteBuffer.tankSmlTemp[minuteBuffer.count] = NAN;
        }
        
        if (isValidTemperature(roomTemp)) {
            minuteBuffer.roomTemp[minuteBuffer.count] = roomTemp;
        } else {
            minuteBuffer.roomTemp[minuteBuffer.count] = NAN;
        }
        
        if (isValidHumidity(roomHumidity)) {
            minuteBuffer.roomHumidity[minuteBuffer.count] = roomHumidity;
        } else {
            minuteBuffer.roomHumidity[minuteBuffer.count] = NAN;
        }
        
        if (isValidPressure(roomPressure)) {
            minuteBuffer.roomPressure[minuteBuffer.count] = roomPressure;
        } else {
            minuteBuffer.roomPressure[minuteBuffer.count] = NAN;
        }
        
        minuteBuffer.count++;
    }
    
    // Проверяем, заполнен ли буфер или прошло 10 минут
    time_t intervalSeconds = now.unixtime() - minuteBuffer.intervalStart.unixtime();
    if (minuteBuffer.count >= SAMPLES_PER_MINUTE_INTERVAL || 
        intervalSeconds >= MINUTE_INTERVAL_SECONDS) {
        aggregateToMinute();
    }
}

void aggregateToMinute() {
    if (minuteBuffer.count == 0) return;
    
    // Вычисляем средние значения за 10-минутный интервал
    float sumTankLrg = 0, sumTankSml = 0, sumRoomTemp = 0, sumRoomHumidity = 0, sumRoomPressure = 0;
    int validTankLrg = 0, validTankSml = 0, validRoomTemp = 0, validRoomHumidity = 0, validRoomPressure = 0;
    
    for (int i = 0; i < minuteBuffer.count; i++) {
        if (isValidTemperature(minuteBuffer.tankLrgTemp[i])) {
            sumTankLrg += minuteBuffer.tankLrgTemp[i];
            validTankLrg++;
        }
        if (isValidTemperature(minuteBuffer.tankSmlTemp[i])) {
            sumTankSml += minuteBuffer.tankSmlTemp[i];
            validTankSml++;
        }
        if (isValidTemperature(minuteBuffer.roomTemp[i])) {
            sumRoomTemp += minuteBuffer.roomTemp[i];
            validRoomTemp++;
        }
        if (isValidHumidity(minuteBuffer.roomHumidity[i])) {
            sumRoomHumidity += minuteBuffer.roomHumidity[i];
            validRoomHumidity++;
        }
        if (isValidPressure(minuteBuffer.roomPressure[i])) {
            sumRoomPressure += minuteBuffer.roomPressure[i];
            validRoomPressure++;
        }
    }
    
    // Создаем 10-минутную запись
    float avgTankLrg = validTankLrg > 0 ? sumTankLrg / validTankLrg : NAN;
    float avgTankSml = validTankSml > 0 ? sumTankSml / validTankSml : NAN;
    float avgRoomTemp = validRoomTemp > 0 ? sumRoomTemp / validRoomTemp : NAN;
    float avgRoomHumidity = validRoomHumidity > 0 ? sumRoomHumidity / validRoomHumidity : NAN;
    float avgRoomPressure = validRoomPressure > 0 ? sumRoomPressure / validRoomPressure : NAN;
    
    // Определяем начало текущего часа
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    uint32_t hourStart = now.unixtime() - (now.unixtime() % 3600);
    
    // Если начался новый час, сохраняем последний час и очищаем minuteLogs
    if (currentHourStart != 0 && currentHourStart != hourStart) {
        // Переход на новый час - сохраняем последний час в EEPROM через aggregateToHour
        if (hourBuffer.count > 0) {
            aggregateToHour();
        }
        minuteLogCount = 0;
    }
    currentHourStart = hourStart;
    
    // Сохраняем 10-минутный интервал в minuteLogs (только для текущего часа)
    if (minuteLogCount < MINUTE_LOGS_PER_HOUR) {
        MinuteLogEntry minuteLog;
        minuteLog.tankLrgTemp = floatToInt16(avgTankLrg);
        minuteLog.tankSmlTemp = floatToInt16(avgTankSml);
        minuteLog.roomTemp = floatToInt16(avgRoomTemp);
        minuteLog.roomHumidity = floatToUint16(avgRoomHumidity);
        minuteLog.roomPressure = floatToUint16(avgRoomPressure);
        minuteLog.timestamp = dateTimeToUint32(minuteBuffer.intervalStart);
        minuteLog.samplesCount = minuteBuffer.count;
        minuteLogs[minuteLogCount++] = minuteLog;
    }
    
    // Добавляем в часовой буфер
    if (hourBuffer.count == 0) {
        hourBuffer.intervalStart = minuteBuffer.intervalStart;
    }
    
    if (hourBuffer.count < MINUTE_INTERVALS_PER_HOUR) {
        hourBuffer.tankLrgTemp[hourBuffer.count] = avgTankLrg;
        hourBuffer.tankSmlTemp[hourBuffer.count] = avgTankSml;
        hourBuffer.roomTemp[hourBuffer.count] = avgRoomTemp;
        hourBuffer.roomHumidity[hourBuffer.count] = avgRoomHumidity;
        hourBuffer.roomPressure[hourBuffer.count] = avgRoomPressure;
        hourBuffer.count++;
    }
    
    // Проверяем, заполнен ли часовой буфер или прошёл час
    time_t intervalSeconds = now.unixtime() - hourBuffer.intervalStart.unixtime();
    if (hourBuffer.count >= MINUTE_INTERVALS_PER_HOUR || 
        intervalSeconds >= HOUR_INTERVAL_SECONDS) {
        aggregateToHour();
    }
    
    // Очищаем минутный буфер
    minuteBuffer.count = 0;
}

void aggregateToHour() {
    if (hourBuffer.count == 0) return;
    
    // Вычисляем средние значения за часовой интервал
    float sumTankLrg = 0, sumTankSml = 0, sumRoomTemp = 0, sumRoomHumidity = 0, sumRoomPressure = 0;
    int validTankLrg = 0, validTankSml = 0, validRoomTemp = 0, validRoomHumidity = 0, validRoomPressure = 0;
    
    for (int i = 0; i < hourBuffer.count; i++) {
        if (isValidTemperature(hourBuffer.tankLrgTemp[i])) {
            sumTankLrg += hourBuffer.tankLrgTemp[i];
            validTankLrg++;
        }
        if (isValidTemperature(hourBuffer.tankSmlTemp[i])) {
            sumTankSml += hourBuffer.tankSmlTemp[i];
            validTankSml++;
        }
        if (isValidTemperature(hourBuffer.roomTemp[i])) {
            sumRoomTemp += hourBuffer.roomTemp[i];
            validRoomTemp++;
        }
        if (isValidHumidity(hourBuffer.roomHumidity[i])) {
            sumRoomHumidity += hourBuffer.roomHumidity[i];
            validRoomHumidity++;
        }
        if (isValidPressure(hourBuffer.roomPressure[i])) {
            sumRoomPressure += hourBuffer.roomPressure[i];
            validRoomPressure++;
        }
    }
    
    // Создаем часовую запись
    LogEntry newLog;
    newLog.timestamp = dateTimeToUint32(hourBuffer.intervalStart);
    newLog.samplesCount = hourBuffer.count;
    
    float avgTankLrg = validTankLrg > 0 ? sumTankLrg / validTankLrg : NAN;
    float avgTankSml = validTankSml > 0 ? sumTankSml / validTankSml : NAN;
    float avgRoomTemp = validRoomTemp > 0 ? sumRoomTemp / validRoomTemp : NAN;
    float avgRoomHumidity = validRoomHumidity > 0 ? sumRoomHumidity / validRoomHumidity : NAN;
    float avgRoomPressure = validRoomPressure > 0 ? sumRoomPressure / validRoomPressure : NAN;
    
    newLog.tankLrgTemp = floatToInt16(avgTankLrg);
    newLog.tankSmlTemp = floatToInt16(avgTankSml);
    newLog.roomTemp = floatToInt16(avgRoomTemp);
    newLog.roomHumidity = floatToUint16(avgRoomHumidity);
    newLog.roomPressure = floatToUint16(avgRoomPressure);
    
    // Проверяем критические изменения
    checkCriticalChanges(avgTankLrg, avgTankSml, avgRoomHumidity, hourBuffer.intervalStart);
    
    // Добавляем в EEPROM (кольцевой буфер)
    if (logCount >= EEPROM_HOURS_COUNT) {
        // Сдвигаем все записи влево, удаляя самую старую
        for (int i = 1; i < EEPROM_HOURS_COUNT; i++) {
            temperatureLogs[i-1] = temperatureLogs[i];
        }
        logCount = EEPROM_HOURS_COUNT - 1;
    }
    temperatureLogs[logCount++] = newLog;
    
    // Сохраняем в EEPROM
    saveLogsToEEPROM();
    
    // Сохраняем в SPIFFS
    if (spiffsInitialized) {
        saveLogsToSPIFFS();
    }
    
    // Обновляем средние значения для проверки критичности
    if (validTankLrg > 0) lastHourAvgTankLrg = floatToInt16(avgTankLrg);
    if (validTankSml > 0) lastHourAvgTankSml = floatToInt16(avgTankSml);
    if (validRoomHumidity > 0) lastHourAvgHumidity = floatToUint16(avgRoomHumidity);
    
    // Очищаем часовой буфер
    hourBuffer.count = 0;
}

void checkCriticalChanges(float tankLrgTemp, float tankSmlTemp, float humidity, DateTime timestamp) {
    // Проверяем температуру аквариума L (большой)
    if (isValidTemperature(tankLrgTemp) && lastHourAvgTankLrg != INT16_NAN) {
        float lastAvg = int16ToFloat(lastHourAvgTankLrg);
        if (lastAvg != 0) {
            float changePercent = abs((tankLrgTemp - lastAvg) / lastAvg * 100.0);
            if (changePercent > CRITICAL_THRESHOLD_PERCENT) {
                addCriticalLog(tankLrgTemp, NAN, NAN, lastAvg, timestamp, 0, changePercent);
                Serial.print(F("[LOGGER] КРИТИЧЕСКОЕ изменение аквариума L: "));
                Serial.print(changePercent);
                Serial.println(F("%"));
            }
        }
    }
    
    // Проверяем температуру аквариума S (малый)
    if (isValidTemperature(tankSmlTemp) && lastHourAvgTankSml != INT16_NAN) {
        float lastAvg = int16ToFloat(lastHourAvgTankSml);
        if (lastAvg != 0) {
            float changePercent = abs((tankSmlTemp - lastAvg) / lastAvg * 100.0);
            if (changePercent > CRITICAL_THRESHOLD_PERCENT) {
                addCriticalLog(NAN, tankSmlTemp, NAN, lastAvg, timestamp, 1, changePercent);
                Serial.print(F("[LOGGER] КРИТИЧЕСКОЕ изменение аквариума S: "));
                Serial.print(changePercent);
                Serial.println(F("%"));
            }
        }
    }
    
    // Проверяем влажность
    if (isValidHumidity(humidity) && lastHourAvgHumidity != UINT16_NAN) {
        float lastAvg = uint16ToFloat(lastHourAvgHumidity);
        if (lastAvg != 0) {
            float changePercent = abs((humidity - lastAvg) / lastAvg * 100.0);
            if (changePercent > CRITICAL_THRESHOLD_PERCENT) {
                addCriticalLog(NAN, NAN, humidity, lastAvg, timestamp, 2, changePercent);
                Serial.print(F("[LOGGER] КРИТИЧЕСКОЕ изменение влажности: "));
                Serial.print(changePercent);
                Serial.println(F("%"));
            }
        }
    }
}

void addCriticalLog(float tankLrgTemp, float tankSmlTemp, float humidity, 
                    float previousAvg, DateTime timestamp, uint8_t paramType, float changePercent) {
    if (criticalLogCount >= MAX_CRITICAL_LOGS) {
        // Сдвигаем все записи влево, удаляя самую старую
        for (int i = 1; i < MAX_CRITICAL_LOGS; i++) {
            criticalLogs[i-1] = criticalLogs[i];
        }
        criticalLogCount = MAX_CRITICAL_LOGS - 1;
    }
    
    CriticalLogEntry entry;
    entry.tankLrgTemp = isValidTemperature(tankLrgTemp) ? floatToInt16(tankLrgTemp) : INT16_NAN;
    entry.tankSmlTemp = isValidTemperature(tankSmlTemp) ? floatToInt16(tankSmlTemp) : INT16_NAN;
    entry.roomHumidity = isValidHumidity(humidity) ? floatToUint16(humidity) : UINT16_NAN;
    entry.previousAvg = floatToInt16(previousAvg);
    entry.timestamp = dateTimeToUint32(timestamp);
    entry.parameterType = paramType;
    entry.changePercent = floatToUint16(changePercent);
    
    criticalLogs[criticalLogCount++] = entry;
    
    // Сохраняем в SPIFFS
    if (spiffsInitialized) {
        saveCriticalLogsToSPIFFS();
    }
}

void saveLogsToEEPROM() {
#ifdef DEBUG_MODE
    return;
#endif

    uint16_t addr = EEPROM_LOG_START;
    
    // Проверяем размер данных
    uint16_t requiredSize = sizeof(logCount) + (logCount * sizeof(LogEntry));
    if (requiredSize > EEPROM_SIZE) {
        Serial.println(F("[LOGGER] Данные не помещаются в EEPROM, уменьшаем logCount"));
        byte maxLogs = (EEPROM_SIZE - sizeof(logCount)) / sizeof(LogEntry);
        if (logCount > maxLogs) {
            logCount = maxLogs;
        }
        requiredSize = sizeof(logCount) + (logCount * sizeof(LogEntry));
    }
    
    EEPROM.put(addr, logCount);
    addr += sizeof(logCount);
    
    // Сохраняем логи в прямом порядке (массив уже отсортирован)
    for (byte i = 0; i < logCount && i < EEPROM_HOURS_COUNT; i++) {
        EEPROM.put(addr, temperatureLogs[i]);
        addr += sizeof(LogEntry);
    }
    
    if (!EEPROM.commit()) {
        Serial.println(F("[LOGGER] Ошибка сохранения в EEPROM"));
    } else {
        Serial.print(F("[LOGGER] Сохранено в EEPROM: "));
        Serial.print(logCount);
        Serial.println(F(" записей"));
    }
}

void loadLogsFromEEPROM() {
#ifdef DEBUG_MODE
    return;
#endif

    uint16_t addr = EEPROM_LOG_START;
    byte loadedCount = 0;
    
    EEPROM.get(addr, loadedCount);
    addr += sizeof(loadedCount);
    
    // Проверяем валидность загруженного количества
    if (loadedCount > EEPROM_HOURS_COUNT) {
        Serial.println(F("[LOGGER] Некорректное количество логов в EEPROM, сбрасываем"));
        logCount = 0;
        return;
    }
    
    logCount = loadedCount;
    
    // Загружаем логи в прямом порядке
    for (byte i = 0; i < logCount && i < EEPROM_HOURS_COUNT; i++) {
        EEPROM.get(addr, temperatureLogs[i]);
        addr += sizeof(LogEntry);
        
        // Валидация данных
        if (temperatureLogs[i].timestamp == 0 || temperatureLogs[i].timestamp > 2000000000) {
            Serial.println(F("[LOGGER] Обнаружена поврежденная запись в EEPROM"));
            logCount = i;
            break;
        }
    }
    
    Serial.print(F("[LOGGER] Загружено из EEPROM: "));
    Serial.println(logCount);
}

void saveLogsToSPIFFS() {
    if (!spiffsInitialized) return;
    
    File file = SPIFFS.open(SPIFFS_HOURLY_FILE, "w");
    if (!file) {
        Serial.println(F("[LOGGER] Ошибка открытия файла для записи"));
        return;
    }
    
    // Объединяем логи из EEPROM и SPIFFS, сохраняем последние 72 часа
    // Сначала загружаем существующие логи из SPIFFS
    loadLogsFromSPIFFS();
    
    // Добавляем новую запись
    if (spiffsLogCount >= SPIFFS_MAX_HOURS) {
        // Сдвигаем все записи влево, удаляя самую старую
        for (int i = 1; i < SPIFFS_MAX_HOURS; i++) {
            spiffsLogs[i-1] = spiffsLogs[i];
        }
        spiffsLogCount = SPIFFS_MAX_HOURS - 1;
    }
    
    // Добавляем последнюю запись из EEPROM
    if (logCount > 0) {
        spiffsLogs[spiffsLogCount++] = temperatureLogs[logCount - 1];
    }
    
    // Записываем количество записей
    file.write((uint8_t*)&spiffsLogCount, sizeof(spiffsLogCount));
    
    // Записываем все записи
    for (byte i = 0; i < spiffsLogCount; i++) {
        file.write((uint8_t*)&spiffsLogs[i], sizeof(LogEntry));
    }
    
    file.close();
    Serial.print(F("[LOGGER] Сохранено в SPIFFS: "));
    Serial.print(spiffsLogCount);
    Serial.println(F(" записей"));
}

void loadLogsFromSPIFFS() {
    if (!spiffsInitialized) return;
    
    if (!SPIFFS.exists(SPIFFS_HOURLY_FILE)) {
        Serial.println(F("[LOGGER] Файл логов в SPIFFS не найден"));
        spiffsLogCount = 0;
        return;
    }
    
    File file = SPIFFS.open(SPIFFS_HOURLY_FILE, "r");
    if (!file) {
        Serial.println(F("[LOGGER] Ошибка открытия файла для чтения"));
        spiffsLogCount = 0;
        return;
    }
    
    // Читаем количество записей
    byte loadedCount = 0;
    if (file.available() >= sizeof(loadedCount)) {
        file.read((uint8_t*)&loadedCount, sizeof(loadedCount));
        
        // Ограничиваем количество загружаемых записей
        if (loadedCount > SPIFFS_MAX_HOURS) {
            loadedCount = SPIFFS_MAX_HOURS;
        }
        
        spiffsLogCount = loadedCount;
        
        // Загружаем записи
        for (byte i = 0; i < spiffsLogCount; i++) {
            if (file.available() >= sizeof(LogEntry)) {
                file.read((uint8_t*)&spiffsLogs[i], sizeof(LogEntry));
                
                // Валидация данных
                if (spiffsLogs[i].timestamp == 0 || spiffsLogs[i].timestamp > 2000000000) {
                    Serial.println(F("[LOGGER] Обнаружена поврежденная запись в SPIFFS"));
                    spiffsLogCount = i;
                    break;
                }
            }
        }
        
        Serial.print(F("[LOGGER] Загружено из SPIFFS: "));
        Serial.print(spiffsLogCount);
        Serial.println(F(" записей (используется для истории)"));
    }
    
    file.close();
}

void saveCriticalLogsToSPIFFS() {
    if (!spiffsInitialized) return;
    
    File file = SPIFFS.open(SPIFFS_CRITICAL_FILE, "w");
    if (!file) {
        Serial.println(F("[LOGGER] Ошибка открытия файла критических логов"));
        return;
    }
    
    // Записываем количество записей
    file.write((uint8_t*)&criticalLogCount, sizeof(criticalLogCount));
    
    // Записываем все записи
    for (byte i = 0; i < criticalLogCount; i++) {
        file.write((uint8_t*)&criticalLogs[i], sizeof(CriticalLogEntry));
    }
    
    file.close();
    Serial.print(F("[LOGGER] Сохранено критических логов: "));
    Serial.println(criticalLogCount);
}

void loadCriticalLogsFromSPIFFS() {
    if (!spiffsInitialized) return;
    
    if (!SPIFFS.exists(SPIFFS_CRITICAL_FILE)) {
        Serial.println(F("[LOGGER] Файл критических логов не найден"));
        return;
    }
    
    File file = SPIFFS.open(SPIFFS_CRITICAL_FILE, "r");
    if (!file) {
        Serial.println(F("[LOGGER] Ошибка открытия файла критических логов"));
        return;
    }
    
    // Читаем количество записей
    if (file.available() >= sizeof(criticalLogCount)) {
        file.read((uint8_t*)&criticalLogCount, sizeof(criticalLogCount));
        
        if (criticalLogCount > MAX_CRITICAL_LOGS) {
            criticalLogCount = MAX_CRITICAL_LOGS;
        }
        
        // Загружаем записи
        for (byte i = 0; i < criticalLogCount; i++) {
            if (file.available() >= sizeof(CriticalLogEntry)) {
                file.read((uint8_t*)&criticalLogs[i], sizeof(CriticalLogEntry));
            }
        }
        
        Serial.print(F("[LOGGER] Загружено критических логов: "));
        Serial.println(criticalLogCount);
    }
    
    file.close();
}

void clearLogs() {
    logCount = 0;
    criticalLogCount = 0;
    minuteLogCount = 0;
    spiffsLogCount = 0;
    minuteBuffer.count = 0;
    hourBuffer.count = 0;
    currentHourStart = 0;
    
    // Очищаем EEPROM
    saveLogsToEEPROM();
    
    // Очищаем SPIFFS
    if (spiffsInitialized) {
        SPIFFS.remove(SPIFFS_HOURLY_FILE);
        SPIFFS.remove(SPIFFS_CRITICAL_FILE);
    }
}

LogEntry getLastLog() {
    if (logCount > 0) {
        return temperatureLogs[logCount-1];
    }
    LogEntry empty;
    memset(&empty, 0, sizeof(LogEntry));
    return empty;
}

LogEntry* getLogs(uint16_t* count) {
    *count = logCount;
    return temperatureLogs;
}

CriticalLogEntry* getCriticalLogs(uint16_t* count) {
    *count = criticalLogCount;
    return criticalLogs;
}

MinuteLogEntry* getCurrentHourMinuteLogs(uint8_t* count) {
    *count = minuteLogCount;
    return minuteLogs;
}

LogEntry* getLogsFromSPIFFS(uint16_t* count) {
    *count = spiffsLogCount;
    return spiffsLogs;
}

// Устаревшая функция для обратной совместимости
void aggregateLogs() {
    // Перенаправляем на новую логику
    if (minuteBuffer.count > 0) {
        aggregateToMinute();
    }
}
