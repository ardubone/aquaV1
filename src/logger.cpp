// logger.cpp
#include "logger.h"
#include <DallasTemperature.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <FS.h>

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

// Буферы для агрегации
MinuteBuffer minuteBuffer;
HourBuffer hourBuffer;

// Критические логи
CriticalLogEntry criticalLogs[MAX_CRITICAL_LOGS];
byte criticalLogCount = 0;

// Устаревшие переменные (для обратной совместимости)
TempBuffer tempBuffer;

// Внутренние переменные
static unsigned long lastSampleTime = 0;
static const uint16_t EEPROM_LOG_START = 0;
static bool spiffsInitialized = false;
static float lastHourAvgTank20 = NAN;
static float lastHourAvgTank10 = NAN;
static float lastHourAvgHumidity = NAN;

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
    
    isInitialized = true;
    Serial.println(F("[LOGGER] Система логирования инициализирована"));
}

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
    float tank20Temp = getTank20Temperature();
    float tank10Temp = getTank10Temperature();
    float roomTemp = getRoomTemp();
    float roomHumidity = getRoomHumidity();
    float roomPressure = getRoomPressure();
    
    // Добавляем данные в буфер 30-секундных измерений
    if (minuteBuffer.count < SAMPLES_PER_MINUTE_INTERVAL) {
        if (isValidTemperature(tank20Temp)) {
            minuteBuffer.tank20Temp[minuteBuffer.count] = tank20Temp;
        } else {
            minuteBuffer.tank20Temp[minuteBuffer.count] = NAN;
        }
        
        if (isValidTemperature(tank10Temp)) {
            minuteBuffer.tank10Temp[minuteBuffer.count] = tank10Temp;
        } else {
            minuteBuffer.tank10Temp[minuteBuffer.count] = NAN;
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
    float sumTank20 = 0, sumTank10 = 0, sumRoomTemp = 0, sumRoomHumidity = 0, sumRoomPressure = 0;
    int validTank20 = 0, validTank10 = 0, validRoomTemp = 0, validRoomHumidity = 0, validRoomPressure = 0;
    
    for (int i = 0; i < minuteBuffer.count; i++) {
        if (isValidTemperature(minuteBuffer.tank20Temp[i])) {
            sumTank20 += minuteBuffer.tank20Temp[i];
            validTank20++;
        }
        if (isValidTemperature(minuteBuffer.tank10Temp[i])) {
            sumTank10 += minuteBuffer.tank10Temp[i];
            validTank10++;
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
    float avgTank20 = validTank20 > 0 ? sumTank20 / validTank20 : NAN;
    float avgTank10 = validTank10 > 0 ? sumTank10 / validTank10 : NAN;
    float avgRoomTemp = validRoomTemp > 0 ? sumRoomTemp / validRoomTemp : NAN;
    float avgRoomHumidity = validRoomHumidity > 0 ? sumRoomHumidity / validRoomHumidity : NAN;
    float avgRoomPressure = validRoomPressure > 0 ? sumRoomPressure / validRoomPressure : NAN;
    
    // Добавляем в часовой буфер
    if (hourBuffer.count == 0) {
        hourBuffer.intervalStart = minuteBuffer.intervalStart;
    }
    
    if (hourBuffer.count < MINUTE_INTERVALS_PER_HOUR) {
        hourBuffer.tank20Temp[hourBuffer.count] = avgTank20;
        hourBuffer.tank10Temp[hourBuffer.count] = avgTank10;
        hourBuffer.roomTemp[hourBuffer.count] = avgRoomTemp;
        hourBuffer.roomHumidity[hourBuffer.count] = avgRoomHumidity;
        hourBuffer.roomPressure[hourBuffer.count] = avgRoomPressure;
        hourBuffer.count++;
    }
    
    // Проверяем, заполнен ли часовой буфер или прошёл час
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
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
    float sumTank20 = 0, sumTank10 = 0, sumRoomTemp = 0, sumRoomHumidity = 0, sumRoomPressure = 0;
    int validTank20 = 0, validTank10 = 0, validRoomTemp = 0, validRoomHumidity = 0, validRoomPressure = 0;
    
    for (int i = 0; i < hourBuffer.count; i++) {
        if (isValidTemperature(hourBuffer.tank20Temp[i])) {
            sumTank20 += hourBuffer.tank20Temp[i];
            validTank20++;
        }
        if (isValidTemperature(hourBuffer.tank10Temp[i])) {
            sumTank10 += hourBuffer.tank10Temp[i];
            validTank10++;
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
    newLog.timestamp = hourBuffer.intervalStart;
    newLog.samplesCount = hourBuffer.count;
    newLog.tank20Temp = validTank20 > 0 ? sumTank20 / validTank20 : NAN;
    newLog.tank10Temp = validTank10 > 0 ? sumTank10 / validTank10 : NAN;
    newLog.roomTemp = validRoomTemp > 0 ? sumRoomTemp / validRoomTemp : NAN;
    newLog.roomHumidity = validRoomHumidity > 0 ? sumRoomHumidity / validRoomHumidity : NAN;
    newLog.roomPressure = validRoomPressure > 0 ? sumRoomPressure / validRoomPressure : NAN;
    newLog.isCritical = false;
    
    // Проверяем критические изменения
    float avgTank20 = newLog.tank20Temp;
    float avgTank10 = newLog.tank10Temp;
    float avgHumidity = newLog.roomHumidity;
    
    checkCriticalChanges(avgTank20, avgTank10, avgHumidity, newLog.timestamp);
    
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
    if (validTank20 > 0) lastHourAvgTank20 = avgTank20;
    if (validTank10 > 0) lastHourAvgTank10 = avgTank10;
    if (validRoomHumidity > 0) lastHourAvgHumidity = avgHumidity;
    
    // Очищаем часовой буфер
    hourBuffer.count = 0;
}

void checkCriticalChanges(float tank20Temp, float tank10Temp, float humidity, DateTime timestamp) {
    // Проверяем температуру Tank20
    if (isValidTemperature(tank20Temp) && !isnan(lastHourAvgTank20) && lastHourAvgTank20 != 0) {
        float changePercent = abs((tank20Temp - lastHourAvgTank20) / lastHourAvgTank20 * 100.0);
        if (changePercent > CRITICAL_THRESHOLD_PERCENT) {
            addCriticalLog(tank20Temp, NAN, NAN, lastHourAvgTank20, timestamp, 0, changePercent);
            Serial.print(F("[LOGGER] КРИТИЧЕСКОЕ изменение Tank20: "));
            Serial.print(changePercent);
            Serial.println(F("%"));
        }
    }
    
    // Проверяем температуру Tank10
    if (isValidTemperature(tank10Temp) && !isnan(lastHourAvgTank10) && lastHourAvgTank10 != 0) {
        float changePercent = abs((tank10Temp - lastHourAvgTank10) / lastHourAvgTank10 * 100.0);
        if (changePercent > CRITICAL_THRESHOLD_PERCENT) {
            addCriticalLog(NAN, tank10Temp, NAN, lastHourAvgTank10, timestamp, 1, changePercent);
            Serial.print(F("[LOGGER] КРИТИЧЕСКОЕ изменение Tank10: "));
            Serial.print(changePercent);
            Serial.println(F("%"));
        }
    }
    
    // Проверяем влажность
    if (isValidHumidity(humidity) && !isnan(lastHourAvgHumidity) && lastHourAvgHumidity != 0) {
        float changePercent = abs((humidity - lastHourAvgHumidity) / lastHourAvgHumidity * 100.0);
        if (changePercent > CRITICAL_THRESHOLD_PERCENT) {
            addCriticalLog(NAN, NAN, humidity, lastHourAvgHumidity, timestamp, 2, changePercent);
            Serial.print(F("[LOGGER] КРИТИЧЕСКОЕ изменение влажности: "));
            Serial.print(changePercent);
            Serial.println(F("%"));
        }
    }
}

void addCriticalLog(float tank20Temp, float tank10Temp, float humidity, 
                    float previousAvg, DateTime timestamp, uint8_t paramType, float changePercent) {
    if (criticalLogCount >= MAX_CRITICAL_LOGS) {
        // Сдвигаем все записи влево, удаляя самую старую
        for (int i = 1; i < MAX_CRITICAL_LOGS; i++) {
            criticalLogs[i-1] = criticalLogs[i];
        }
        criticalLogCount = MAX_CRITICAL_LOGS - 1;
    }
    
    CriticalLogEntry entry;
    entry.tank20Temp = tank20Temp;
    entry.tank10Temp = tank10Temp;
    entry.roomHumidity = humidity;
    entry.previousAvg = previousAvg;
    entry.timestamp = timestamp;
    entry.parameterType = paramType;
    entry.changePercent = changePercent;
    
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
    if (requiredSize > 512) {
        Serial.println(F("[LOGGER] Данные не помещаются в EEPROM, уменьшаем logCount"));
        byte maxLogs = (512 - sizeof(logCount)) / sizeof(LogEntry);
        if (logCount > maxLogs) {
            logCount = maxLogs;
        }
        requiredSize = sizeof(logCount) + (logCount * sizeof(LogEntry));
    }
    
    EEPROM.put(addr, logCount);
    addr += sizeof(logCount);
    
    for (byte i = 0; i < logCount; i++) {
        EEPROM.put(addr, temperatureLogs[i]);
        addr += sizeof(LogEntry);
    }
    
    if (!EEPROM.commit()) {
        Serial.println(F("[LOGGER] Ошибка сохранения в EEPROM"));
    }
}

void loadLogsFromEEPROM() {
#ifdef DEBUG_MODE
    return;
#endif

    uint16_t addr = EEPROM_LOG_START;
    byte loadedCount = 0;
    
    EEPROM.get(addr, loadedCount);
    
    // Проверяем валидность загруженного количества
    if (loadedCount > EEPROM_HOURS_COUNT) {
        Serial.println(F("[LOGGER] Некорректное количество логов в EEPROM, сбрасываем"));
        logCount = 0;
        return;
    }
    
    addr += sizeof(loadedCount);
    logCount = loadedCount;
    
    for (byte i = 0; i < logCount; i++) {
        EEPROM.get(addr, temperatureLogs[i]);
        addr += sizeof(LogEntry);
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
    
    // Записываем количество записей
    file.write((uint8_t*)&logCount, sizeof(logCount));
    
    // Записываем все записи
    for (byte i = 0; i < logCount; i++) {
        file.write((uint8_t*)&temperatureLogs[i], sizeof(LogEntry));
    }
    
    file.close();
}

void loadLogsFromSPIFFS() {
    if (!spiffsInitialized) return;
    
    if (!SPIFFS.exists(SPIFFS_HOURLY_FILE)) {
        Serial.println(F("[LOGGER] Файл логов в SPIFFS не найден"));
        return;
    }
    
    File file = SPIFFS.open(SPIFFS_HOURLY_FILE, "r");
    if (!file) {
        Serial.println(F("[LOGGER] Ошибка открытия файла для чтения"));
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
        
        // Загружаем записи (но не перезаписываем EEPROM данные)
        // SPIFFS используется только для истории старше 24 часов
        Serial.print(F("[LOGGER] Загружено из SPIFFS: "));
        Serial.print(loadedCount);
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
    minuteBuffer.count = 0;
    hourBuffer.count = 0;
    
    // Очищаем EEPROM
    saveLogsToEEPROM();
    
    // Очищаем SPIFFS
    if (spiffsInitialized) {
        SPIFFS.remove(SPIFFS_HOURLY_FILE);
        SPIFFS.remove(SPIFFS_CRITICAL_FILE);
    }
}

LogEntry getLastLog() {
    return logCount > 0 ? temperatureLogs[logCount-1] : LogEntry();
}

LogEntry* getLogs(uint16_t* count) {
    *count = logCount;
    return temperatureLogs;
}

CriticalLogEntry* getCriticalLogs(uint16_t* count) {
    *count = criticalLogCount;
    return criticalLogs;
}

// Устаревшая функция для обратной совместимости
void aggregateLogs() {
    // Перенаправляем на новую логику
    if (minuteBuffer.count > 0) {
        aggregateToMinute();
    }
}
