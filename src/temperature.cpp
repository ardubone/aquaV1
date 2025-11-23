#include "temperature.h"
#include "config.h"
#include <EEPROM.h>
#include <string.h>

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Дефолтные адреса датчиков
DeviceAddress defaultTank20SensorAddr = {0x28, 0x29, 0x1F, 0x52, 0x00, 0x00, 0x00, 0xF2};
DeviceAddress defaultTank10SensorAddr = {0x28, 0x80, 0xF2, 0x53, 0x00, 0x00, 0x00, 0x81};

// Текущие адреса датчиков (загружаются из EEPROM или используют дефолтные)
DeviceAddress tank20SensorAddr;
DeviceAddress tank10SensorAddr;

// Адрес в EEPROM для хранения адресов датчиков (после логов)
#define EEPROM_SENSOR_ADDRESSES_START 496
#define EEPROM_SENSOR_ADDRESSES_SIZE 17  // 2 адреса по 8 байт + флаг валидности 1 байт

void initTemperatureSensors() {
    sensors.begin();
    // Включаем блокирующий режим для корректного чтения температуры
    sensors.setWaitForConversion(true);
    
    // Загружаем адреса из EEPROM
    loadSensorAddressesFromEEPROM();
}

void requestTemperatures() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return; // Не запрашиваем температуру, если датчики не инициализированы
    }
#endif
    sensors.requestTemperatures();
    // setWaitForConversion(true) уже установлен, поэтому ждем завершения конвертации автоматически
}

float getTank20Temperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank20Temperature();
    }
#endif
    return sensors.getTempC(tank20SensorAddr);
}

float getTank10Temperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank10Temperature();
    }
#endif
    return sensors.getTempC(tank10SensorAddr);
}

bool isSensorConnected(uint8_t index) {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return false;
    }
#endif
    return sensors.getDeviceCount() > index;
}

uint8_t getAllConnectedSensors(DeviceAddress* addresses, uint8_t maxCount) {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return 0;
    }
#endif
    uint8_t deviceCount = sensors.getDeviceCount();
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

void saveSensorAddressesToEEPROM() {
    uint16_t addr = EEPROM_SENSOR_ADDRESSES_START;
    
    // Сохраняем флаг валидности
    uint8_t validFlag = 0xAA; // Магическое число для проверки валидности
    EEPROM.put(addr, validFlag);
    addr += sizeof(validFlag);
    
    // Сохраняем адреса
    EEPROM.put(addr, tank20SensorAddr);
    addr += sizeof(DeviceAddress);
    EEPROM.put(addr, tank10SensorAddr);
    
    if (!EEPROM.commit()) {
        Serial.println(F("[TEMP] Ошибка сохранения адресов датчиков в EEPROM"));
    } else {
        Serial.println(F("[TEMP] Адреса датчиков сохранены в EEPROM"));
    }
}

void loadSensorAddressesFromEEPROM() {
    uint16_t addr = EEPROM_SENSOR_ADDRESSES_START;
    
    // Проверяем флаг валидности
    uint8_t validFlag = 0;
    EEPROM.get(addr, validFlag);
    addr += sizeof(validFlag);
    
    if (validFlag != 0xAA) {
        // Данные невалидны, используем дефолтные адреса
        memcpy(tank20SensorAddr, defaultTank20SensorAddr, sizeof(DeviceAddress));
        memcpy(tank10SensorAddr, defaultTank10SensorAddr, sizeof(DeviceAddress));
        Serial.println(F("[TEMP] Используются дефолтные адреса датчиков"));
        return;
    }
    
    // Загружаем адреса
    EEPROM.get(addr, tank20SensorAddr);
    addr += sizeof(DeviceAddress);
    EEPROM.get(addr, tank10SensorAddr);
    
    Serial.println(F("[TEMP] Адреса датчиков загружены из EEPROM"));
}

bool setSensorAddress(uint8_t tankIndex, DeviceAddress address) {
    if (tankIndex == 0) {
        // Tank20
        memcpy(tank20SensorAddr, address, sizeof(DeviceAddress));
    } else if (tankIndex == 1) {
        // Tank10
        memcpy(tank10SensorAddr, address, sizeof(DeviceAddress));
    } else {
        return false;
    }
    
    saveSensorAddressesToEEPROM();
    return true;
} 