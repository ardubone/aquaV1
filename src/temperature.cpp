#include "temperature.h"
#include "config.h"
#include <EEPROM.h>
#include <string.h>

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// float currentTempLrg = -127.0f;
// float currentTempSml = -127.0f;

void printAddress(const DeviceAddress deviceAddress);

// unsigned long lastRequestTime = 0;
// bool waitingForConversion = false;
// const unsigned long CONVERSION_TIME = 800;
// const unsigned long POLL_INTERVAL = 2000;

const uint8_t TEMP_LOG_CAP = 100;
String tempLogs[TEMP_LOG_CAP];
uint8_t tempLogHead = 0;
uint8_t tempLogCount = 0;

// static uint8_t owSearch(DeviceAddress* addresses, uint8_t max) {
//     oneWire.reset_search();
//     uint8_t count = 0;
//     uint8_t addr[8];
//     while (count < max && oneWire.search(addr)) {
//         if (OneWire::crc8(addr, 7) != addr[7]) {
//             continue;
//         }
//         memcpy(addresses[count], addr, sizeof(DeviceAddress));
//         count++;
//     }
//     return count;
// }

// static void assignActiveSensorAddresses() { /* отключено */ }

// Текущие адреса датчиков (используют только дефолтные из конфига)
// Переменные оставлены для обратной совместимости, но везде используются дефолтные адреса напрямую
DeviceAddress tankLrgSensorAddr;
DeviceAddress tankSmlSensorAddr;

// Helper для печати адреса
void printAddress(const DeviceAddress deviceAddress) {
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
    
    sensors.setWaitForConversion(true);
    
    // Инициализируем переменные дефолтными адресами для обратной совместимости
    // Но везде в коде используем константы из конфига напрямую
    memcpy(tankLrgSensorAddr, TEMP_SENSOR_ADDR_TANK_LRG, sizeof(DeviceAddress));
    memcpy(tankSmlSensorAddr, TEMP_SENSOR_ADDR_TANK_SML, sizeof(DeviceAddress));
    
    Serial.println(F("[TEMP] Используются дефолтные адреса датчиков из конфигурации"));
    {
        uint8_t deviceCount = sensors.getDeviceCount();
        String msg = String("[TEMP] devices=") + String(deviceCount);
        Serial.println(msg);
        tempLogs[tempLogHead] = msg;
        tempLogHead = (tempLogHead + 1) % TEMP_LOG_CAP;
        if (tempLogCount < TEMP_LOG_CAP) tempLogCount++;
        if (deviceCount > 0) {
            DeviceAddress addr;
            for (uint8_t i = 0; i < deviceCount; i++) {
                if (sensors.getAddress(addr, i)) {
                    Serial.print(F("[TEMP] dev[")); Serial.print(i); Serial.print(F("]="));
                    printAddress(addr);
                    Serial.println();
                }
            }
        }
        Serial.print(F("[TEMP] default Lrg="));
        printAddress(TEMP_SENSOR_ADDR_TANK_LRG);
        Serial.println();
        Serial.print(F("[TEMP] default Sml="));
        printAddress(TEMP_SENSOR_ADDR_TANK_SML);
        Serial.println();
        bool vL = isAddressValid(TEMP_SENSOR_ADDR_TANK_LRG);
        bool vS = isAddressValid(TEMP_SENSOR_ADDR_TANK_SML);
        String vmsg = String("[TEMP] valid Lrg=") + String(vL ? "1" : "0") + String(", Sml=") + String(vS ? "1" : "0");
        Serial.println(vmsg);
        tempLogs[tempLogHead] = vmsg;
        tempLogHead = (tempLogHead + 1) % TEMP_LOG_CAP;
        if (tempLogCount < TEMP_LOG_CAP) tempLogCount++;
    }
    // assignActiveSensorAddresses();
    
    sensors.requestTemperatures();
}

void rescanTemperatureSensors() {
    sensors.begin();
    sensors.setWaitForConversion(true);

    // Hybrid fallback: если дефолтные адреса не найдены, временно привязываем к найденным датчикам
    DeviceAddress addresses[2];
    const uint8_t found = getAllConnectedSensors(addresses, 2);

    const bool lrgValid = isAddressValid(tankLrgSensorAddr);
    const bool smlValid = isAddressValid(tankSmlSensorAddr);

    if (!lrgValid && found >= 1) {
        if (setSensorAddress(0, addresses[0])) {
            Serial.println(F("[TEMP] fallback: Lrg -> dev[0]"));
        }
    }

    // Если на шине только 1 датчик — не назначаем его как Sml, чтобы не путать каналы
    if (!smlValid && found >= 2) {
        if (setSensorAddress(1, addresses[1])) {
            Serial.println(F("[TEMP] fallback: Sml -> dev[1]"));
        }
    }
}

void requestTemperatures() {
    static unsigned long lastRescanMs = 0;
    const unsigned long nowMs = millis();

    // Если шина "упала" (0 устройств), периодически пробуем переинициализацию
    if (sensors.getDeviceCount() == 0 && (nowMs - lastRescanMs) >= DS18B20_RESCAN_INTERVAL_MS) {
        lastRescanMs = nowMs;
        rescanTemperatureSensors();
    }

    sensors.requestTemperatures();
}

float getLrgTemperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank20Temperature();
    }
#endif
    return sensors.getTempC(tankLrgSensorAddr);
}

float getSmlTemperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank10Temperature();
    }
#endif
    return sensors.getTempC(tankSmlSensorAddr);
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
    if (tankIndex == 0) {
        memcpy(tankLrgSensorAddr, address, sizeof(DeviceAddress));
        Serial.print(F("[TEMP] set Lrg addr="));
        printAddress(tankLrgSensorAddr);
        Serial.println();
        return true;
    } else if (tankIndex == 1) {
        memcpy(tankSmlSensorAddr, address, sizeof(DeviceAddress));
        Serial.print(F("[TEMP] set Sml addr="));
        printAddress(tankSmlSensorAddr);
        Serial.println();
        return true;
    }
    return false;
}

String getTemperatureLogsJSON() {
    String json = "{\"logs\":[";
    for (uint8_t i = 0; i < tempLogCount; i++) {
        uint8_t idx = (tempLogHead + TEMP_LOG_CAP - tempLogCount + i) % TEMP_LOG_CAP;
        if (i > 0) json += ",";
        json += "\"" + tempLogs[idx] + "\"";
    }
    json += "]}";
    return json;
}

String getTemperatureStatusJSON() {
    String json = "{";
    json += "\"waitingForConversion\":false,";
    json += "\"lastRequestAgeMs\":0,";
    json += "\"convTimeMs\":0,";
    json += "\"pollIntervalMs\":0";
    json += "}";
    return json;
}
