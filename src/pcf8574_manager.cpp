#include "pcf8574_manager.h"

PCF8574Manager::PCF8574Manager(uint8_t addr) 
    : address(addr), isInitialized(false), portState(0xFF) {
}

bool PCF8574Manager::begin() {
    if (isInitialized) return true;
    
    Serial.print("Trying to initialize PCF8574 at address 0x");
    Serial.println(address, HEX);
    
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.print("PCF8574 initialization failed! Error code: ");
        Serial.println(error);
        return false;
    }
    
    Serial.println("PCF8574 initialized successfully!");
    
    // Инициализация пинов согласно конфигурации
    for (uint8_t i = 0; i < PCF8574_PIN_COUNT; i++) {
        if (pcf8574Pins[i].isOutput) {
            portState &= ~(1 << pcf8574Pins[i].pin);  // Устанавливаем пин как выход (0)
        }
    }
    
    // Записываем начальное состояние
    Wire.beginTransmission(address);
    Wire.write(portState);
    if (Wire.endTransmission() != 0) {
        Serial.println("Failed to set initial PCF8574 state!");
        return false;
    }
    
    isInitialized = true;
    return true;
}

bool PCF8574Manager::setPin(uint8_t pin, bool state) {
    if (!isInitialized) return false;
    if (pin >= PCF8574_PIN_COUNT) return false;
    if (!pcf8574Pins[pin].isOutput) return false;
    
    if (state) {
        portState |= (1 << pcf8574Pins[pin].pin);
    } else {
        portState &= ~(1 << pcf8574Pins[pin].pin);
    }
    
    Wire.beginTransmission(address);
    Wire.write(portState);
    return Wire.endTransmission() == 0;
}

bool PCF8574Manager::getPin(uint8_t pin) {
    if (!isInitialized) return false;
    if (pin >= PCF8574_PIN_COUNT) return false;
    if (pcf8574Pins[pin].isOutput) return false;
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        uint8_t data = Wire.read();
        return (data & (1 << pcf8574Pins[pin].pin)) != 0;
    }
    return false;
}

void PCF8574Manager::update() {
    if (!isInitialized) return;
    // Здесь можно добавить дополнительную логику обновления состояний
} 