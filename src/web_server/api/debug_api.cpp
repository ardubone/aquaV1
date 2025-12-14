// debug_api.cpp
#include "debug_api.h"
#include "../../../include/web_server.h"
#include "../../../include/config.h"
#include "../../../include/temperature.h"
#include "../../pcf8574_manager.h"
#include <EEPROM.h>

extern WebServer server;
extern PCF8574Manager pcfManager;

// PCF8574 API
void handleDebugPcf8574Status() {
    if (!isPcf8574Initialized) {
        server.send(500, "application/json", "{\"error\":\"PCF8574 not initialized\"}");
        return;
    }
    
    uint8_t allPins = pcfManager.getAllPins();
    
    String json = "{";
    json += "\"pins\":[";
    
    for (uint8_t i = 0; i < PCF8574_PIN_COUNT; i++) {
        if (i > 0) json += ",";
        
        uint8_t pin = pcf8574Pins[i].pin;
        bool pinState;
        
        if (pcf8574Pins[i].isOutput) {
            // Для выходов используем специальные методы для точного состояния
            // Для MOSFET кормушек методы уже возвращают инвертированное состояние
            if (pin == 0) {
                pinState = pcfManager.getUvLampTank10State();
            } else if (pin == 1) {
                pinState = pcfManager.getUvLampTank20State();
            } else if (pin == 2) {
                // getFeederRelayTank10State уже возвращает инвертированное состояние
                pinState = pcfManager.getFeederRelayTank10State();
            } else if (pin == 4) {
                // getFeederRelayTank20State уже возвращает инвертированное состояние
                pinState = pcfManager.getFeederRelayTank20State();
            } else {
                // Для других выходов используем общий метод
                pinState = (allPins & (1 << pin)) != 0;
            }
        } else {
            // Для входов читаем состояние и инвертируем (LOW = нажат/активен)
            if (pin == 3) {
                pinState = pcfManager.getFeederLimitTank10();
            } else if (pin == 5) {
                pinState = pcfManager.getFeederLimitTank20();
            } else {
                pinState = !((allPins & (1 << pin)) != 0);
            }
        }
        
        json += "{";
        json += "\"pin\":" + String(pin) + ",";
        json += "\"name\":\"" + String(pcf8574Pins[i].name) + "\",";
        json += "\"isOutput\":" + String(pcf8574Pins[i].isOutput ? "true" : "false") + ",";
        json += "\"state\":" + String(pinState ? "true" : "false");
        json += "}";
    }
    
    json += "]}";
    server.send(200, "application/json", json);
}

void handleDebugPcf8574SetPin() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    if (!isPcf8574Initialized) {
        server.send(500, "application/json", "{\"error\":\"PCF8574 not initialized\"}");
        return;
    }
    
    uint8_t pin = server.arg("pin").toInt();
    bool state = server.arg("state") == "true" || server.arg("state") == "1";
    
    // Проверяем, что пин существует и является выходом
    bool pinFound = false;
    bool isOutput = false;
    
    for (uint8_t i = 0; i < PCF8574_PIN_COUNT; i++) {
        if (pcf8574Pins[i].pin == pin) {
            pinFound = true;
            isOutput = pcf8574Pins[i].isOutput;
            break;
        }
    }
    
    if (!pinFound) {
        server.send(400, "application/json", "{\"error\":\"Pin not found\"}");
        return;
    }
    
    if (!isOutput) {
        server.send(400, "application/json", "{\"error\":\"Pin is input, cannot set state\"}");
        return;
    }
    
    bool result;
    // Для MOSFET кормушек используем специальные методы с инверсией
    if (pin == PCF8574_FEEDER_RELAY_TANK10) {
        result = pcfManager.setFeederRelayTank10(state);
    } else if (pin == PCF8574_FEEDER_RELAY_TANK20) {
        result = pcfManager.setFeederRelayTank20(state);
    } else {
        // Для других выходов используем прямой метод
        result = pcfManager.setPin(pin, state);
    }
    
    if (result) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "application/json", "{\"error\":\"Failed to set pin\"}");
    }
}

// Temperature sensors API
void handleDebugTemperatureStatus() {
    uint8_t deviceCount = sensors.getDeviceCount();
    if (deviceCount == 0) {
        rescanTemperatureSensors();
        deviceCount = sensors.getDeviceCount();
    }
    
    // Используем константы из config.h напрямую
    // extern DeviceAddress defaultTankLrgSensorAddr; // Удалено
    // extern DeviceAddress defaultTankSmlSensorAddr; // Удалено
    
    bool tankLrgConnected = isAddressValid(tankLrgSensorAddr);
    bool tankSmlConnected = isAddressValid(tankSmlSensorAddr);
    
    String json = "{";
    
    // Текущие адреса (всегда возвращаем дефолтные из конфига)
    json += "\"tankLrgAddress\":[";
    for (uint8_t i = 0; i < 8; i++) {
        if (i > 0) json += ",";
        json += String(tankLrgSensorAddr[i]);
    }
    json += "],";
    
    json += "\"tankSmlAddress\":[";
    for (uint8_t i = 0; i < 8; i++) {
        if (i > 0) json += ",";
        json += String(tankSmlSensorAddr[i]);
    }
    json += "],";
    
    // Получаем все подключенные датчики
    DeviceAddress addresses[16]; // Максимум 16 датчиков
    uint8_t sensorCount = getAllConnectedSensors(addresses, 16);
    
    // Текущие температуры
    float tankLrgTemp = getLrgTemperature();
    float tankSmlTemp = getSmlTemperature();
    
    json += "\"tankLrgTemp\":" + String(tankLrgTemp) + ",";
    json += "\"tankSmlTemp\":" + String(tankSmlTemp) + ",";
    json += "\"tankLrgConnected\":" + String(tankLrgConnected ? "true" : "false") + ",";
    json += "\"tankSmlConnected\":" + String(tankSmlConnected ? "true" : "false") + ",";
    json += "\"addrValidLrg\":" + String(tankLrgConnected ? "true" : "false") + ",";
    json += "\"addrValidSml\":" + String(tankSmlConnected ? "true" : "false") + ",";
    json += "\"dsInit\":" + String(isDs18b20Initialized ? "true" : "false") + ",";
    json += "\"deviceCount\":" + String(deviceCount) + ",";
    json += "\"poll\":" + getTemperatureStatusJSON() + ",";
    
    // Список всех датчиков
    json += "\"sensors\":[";
    for (uint8_t i = 0; i < sensorCount; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"address\":[";
        for (uint8_t j = 0; j < 8; j++) {
            if (j > 0) json += ",";
            json += String(addresses[i][j]);
        }
        json += "],";
        // Читаем температуру для конкретного адреса (уже запрошена выше)
        float temp = sensors.getTempC(addresses[i]);
        json += "\"temp\":" + String(temp);
        json += "}";
    }
    json += "]";
    
    json += "}";
    server.send(200, "application/json", json);
}

void handleDebugTemperatureSetAddress() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    if (!isDs18b20Initialized) {
        server.send(500, "application/json", "{\"error\":\"DS18B20 not initialized\"}");
        return;
    }
    
    String tankLrgStr = server.arg("tankLrg");
    String tankSmlStr = server.arg("tankSml");
    
    if (tankLrgStr.length() != 16 || tankSmlStr.length() != 16) {
        server.send(400, "application/json", "{\"error\":\"Invalid address format\"}");
        return;
    }
    
    // Преобразуем строки в адреса
    DeviceAddress tankLrgAddr, tankSmlAddr;
    
    for (uint8_t i = 0; i < 8; i++) {
        String byteStr = tankLrgStr.substring(i * 2, i * 2 + 2);
        tankLrgAddr[i] = strtoul(byteStr.c_str(), nullptr, 16);
        
        byteStr = tankSmlStr.substring(i * 2, i * 2 + 2);
        tankSmlAddr[i] = strtoul(byteStr.c_str(), nullptr, 16);
    }
    
    // Устанавливаем адреса
    if (!setSensorAddress(0, tankLrgAddr) || !setSensorAddress(1, tankSmlAddr)) {
        server.send(500, "application/json", "{\"error\":\"Failed to set sensor addresses\"}");
        return;
    }
    
    server.send(200, "application/json", "{\"success\":true}");
}

void handleDebugTemperatureLogs() {
    String json = getTemperatureLogsJSON();
    server.send(200, "application/json", json);
}
