#include "temperature.h"
#include "config.h"
#include <EEPROM.h>
#include <string.h>

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Переменные для хранения кэшированных значений
float currentTempLrg = -127.0f;
float currentTempSml = -127.0f;

// Переменные для асинхронного опроса
unsigned long lastRequestTime = 0;
bool waitingForConversion = false;
const unsigned long CONVERSION_TIME = 800; // мс (с запасом для 12 бит)
const unsigned long POLL_INTERVAL = 2000;  // мс (интервал опроса)

// Текущие адреса датчиков (используют только дефолтные из конфига)
// Переменные оставлены для обратной совместимости, но везде используются дефолтные адреса напрямую
DeviceAddress tankLrgSensorAddr;
DeviceAddress tankSmlSensorAddr;

// Helper для печати адреса
void printAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
        if (i < 7) Serial.print(":");
    }
}

void debugScanOneWireBus() {
    Serial.println(F("=== [ONEWIRE] Scan Start ==="));
    
    // ВАЖНО: Не вызываем sensors.begin() здесь, чтобы не сбросить разрешение битности
    // Просто перебираем обнаруженные устройства
    
    uint8_t deviceCount = sensors.getDeviceCount();
    Serial.print(F("Devices found: "));
    Serial.println(deviceCount);
    
    if (deviceCount == 0) {
        Serial.println(F("No devices found on bus!"));
    } else {
        DeviceAddress tempAddr;
        for (uint8_t i = 0; i < deviceCount; i++) {
            if (sensors.getAddress(tempAddr, i)) {
                Serial.print(F("Device "));
                Serial.print(i);
                Serial.print(F(": "));
                printAddress(tempAddr);
                Serial.println();
            } else {
                Serial.print(F("Device "));
                Serial.print(i);
                Serial.println(F(": Unable to get address"));
            }
        }
    }
    Serial.println(F("=== [ONEWIRE] Scan End ==="));
}

// Получает все подключенные датчики
uint8_t getAllConnectedSensors(DeviceAddress* addresses, uint8_t maxCount) {
    uint8_t deviceCount = sensors.getDeviceCount();
    
    if (deviceCount == 0) {
        return 0;
    }
    
    if (deviceCount > maxCount) {
        deviceCount = maxCount;
    }
    
    for (uint8_t i = 0; i < deviceCount; i++) {
        if (!sensors.getAddress(addresses[i], i)) {
            return i; // Ошибка получения адреса
        }
    }
    
    return deviceCount;
}

// Проверяет, существует ли датчик с указанным адресом
bool isAddressValid(const DeviceAddress address) {
    uint8_t deviceCount = sensors.getDeviceCount();
    DeviceAddress tempAddr;
    
    for (uint8_t i = 0; i < deviceCount; i++) {
        if (sensors.getAddress(tempAddr, i)) {
            if (memcmp(tempAddr, address, sizeof(DeviceAddress)) == 0) {
                return true;
            }
        }
    }
    return false;
}

void initTemperatureSensors() {
    pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
    sensors.begin();
    
    // ВАЖНО: Отключаем автоматическое ожидание конвертации
    // Теперь мы сами будем управлять временем ожидания, чтобы не блокировать loop()
    sensors.setWaitForConversion(false);
    
    // Инициализируем переменные дефолтными адресами для обратной совместимости
    // Но везде в коде используем константы из конфига напрямую
    memcpy(tankLrgSensorAddr, TEMP_SENSOR_ADDR_TANK_LRG, sizeof(DeviceAddress));
    memcpy(tankSmlSensorAddr, TEMP_SENSOR_ADDR_TANK_SML, sizeof(DeviceAddress));
    
    Serial.println(F("[TEMP] Используются дефолтные адреса датчиков из конфигурации"));
    
    // Делаем первый запрос сразу
    sensors.requestTemperatures();
    lastRequestTime = millis();
    waitingForConversion = true;
}

void rescanTemperatureSensors() {
    sensors.begin();
    sensors.setWaitForConversion(false);
}

// Функция должна вызываться в главном цикле (loop)
// Она управляет процессом опроса датчиков без блокировок
void requestTemperatures() {
    unsigned long currentTime = millis();

    // Если ждем завершения конвертации
    if (waitingForConversion) {
        // Проверяем, прошло ли достаточно времени
        if (currentTime - lastRequestTime >= CONVERSION_TIME) {
            // Время вышло, читаем данные
            
            // Читаем Lrg
            float tempL = sensors.getTempC(TEMP_SENSOR_ADDR_TANK_LRG);
            if (tempL != DEVICE_DISCONNECTED_C) {
                 currentTempLrg = tempL;
            } else {
                 // Если ошибка чтения, сохраняем маркер ошибки
                 currentTempLrg = -127.0f;
            }

            // Читаем Sml
            float tempS = sensors.getTempC(TEMP_SENSOR_ADDR_TANK_SML);
            if (tempS != DEVICE_DISCONNECTED_C) {
                 currentTempSml = tempS;
            } else {
                 currentTempSml = -127.0f;
            }

            waitingForConversion = false;
            lastRequestTime = currentTime; // Сброс таймера для интервала опроса
        }
    } else {
        // Если не ждем, проверяем, пора ли делать новый запрос
        if (currentTime - lastRequestTime >= POLL_INTERVAL) {
            sensors.requestTemperatures();
            waitingForConversion = true;
            lastRequestTime = currentTime;
        }
    }
}

// Просто возвращает последнее считанное значение из кэша
float getLrgTemperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank20Temperature();
    }
#endif
    return currentTempLrg;
}

// Просто возвращает последнее считанное значение из кэша
float getSmlTemperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank10Temperature();
    }
#endif
    return currentTempSml;
}

// Возвращает строковое представление температуры для UI
String getLrgTemperatureString() {
    float temp = getLrgTemperature();
    if (temp <= -100.0f) {
        return String("Ошибка");
    }
    return String(temp, 1);
}

// Возвращает строковое представление температуры для UI
String getSmlTemperatureString() {
    float temp = getSmlTemperature();
    if (temp <= -100.0f) {
        return String("Ошибка");
    }
    return String(temp, 1);
}

// Обратная совместимость
float getTank20Temperature() {
    return getLrgTemperature();
}

float getTank10Temperature() {
    return getSmlTemperature();
}

bool isSensorConnected(uint8_t index) {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return false;
    }
#endif
    return sensors.getDeviceCount() > index;
}

// Заглушки для обратной совместимости (не используются)
void saveSensorAddressesToEEPROM() {
    // Не используется - всегда используем только дефолтные адреса из конфига
}

bool loadSensorAddressesFromEEPROM() {
    // Не используется - всегда используем только дефолтные адреса из конфига
    return false;
}

bool setSensorAddress(uint8_t tankIndex, DeviceAddress address) {
    // Не используется - всегда используем только дефолтные адреса из конфига
    (void)tankIndex;
    (void)address;
    return false;
}
