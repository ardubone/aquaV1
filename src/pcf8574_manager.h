#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "../include/config.h"

class PCF8574Manager {
private:
    uint8_t address;
    bool isInitialized;
    uint8_t portState;  // Текущее состояние портов
    
public:
    PCF8574Manager(uint8_t addr = PCF8574_ADDRESS);
    bool begin();
    bool setPin(uint8_t pin, bool state);
    bool getPin(uint8_t pin);
    void update();
}; 