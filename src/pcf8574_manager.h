#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "../include/config.h"

class PCF8574Manager {
private:
    uint8_t address;
    bool isInitialized;
    uint8_t portState;  // Текущее состояние портов
    uint8_t lastPortState;  // Предыдущее состояние портов для детектирования изменений
    
    // Поиск индекса конфигурации по физическому номеру пина
    int8_t findPinIndex(uint8_t physicalPin);
    
public:
    PCF8574Manager(uint8_t addr = PCF8574_ADDRESS);
    bool begin();
    bool setPin(uint8_t physicalPin, bool state);
    bool getPin(uint8_t physicalPin);
    uint8_t getAllPins();  // Получить состояние всех пинов
    bool setUvLampTank10(bool state);  // Управление UV лампой Tank10 (пин 0)
    bool getUvLampTank10State();  // Получить состояние UV лампы Tank10 (пин 0)
    bool setUvLampTank20(bool state);  // Управление UV лампой Tank20 (пин 1)
    bool getUvLampTank20State();  // Получить состояние UV лампы Tank20 (пин 1)
    
    // Методы для кормушки Tank10
    bool setFeederRelayTank10(bool state);  // Управление реле кормушки Tank10 (пин 2)
    bool getFeederRelayTank10State();  // Получить состояние реле кормушки Tank10
    bool getFeederLimitTank10();  // Получить состояние концевика кормушки Tank10 (пин 3)
    
    // Методы для кормушки Tank20
    bool setFeederRelayTank20(bool state);  // Управление реле кормушки Tank20 (пин 4)
    bool getFeederRelayTank20State();  // Получить состояние реле кормушки Tank20
    bool getFeederLimitTank20();  // Получить состояние концевика кормушки Tank20 (пин 5)
    
    void update();  // Обновление состояний (вызывать в loop)
}; 