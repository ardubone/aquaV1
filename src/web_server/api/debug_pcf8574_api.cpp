// debug_pcf8574_api.cpp
#include "debug_pcf8574_api.h"
#include "../../../include/web_server.h"
#include "../../../include/config.h"
#include "../../pcf8574_manager.h"

extern WebServer server;
extern PCF8574Manager pcfManager;

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

