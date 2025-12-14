#include "pcf8574_manager.h"

PCF8574Manager::PCF8574Manager(uint8_t addr) 
    : address(addr), isInitialized(false), portState(0xFF), lastPortState(0xFF) {
}

bool PCF8574Manager::begin() {
    if (isInitialized) return true;
    
    Serial.print(F("Trying to initialize PCF8574 at address 0x"));
    Serial.println(address, HEX);
    
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.print(F("PCF8574 initialization failed! Error code: "));
        Serial.println(error);
        return false;
    }
    
    Serial.println(F("PCF8574 initialized successfully!"));
    
    // Инициализация пинов согласно конфигурации
    // Для входов оставляем HIGH (1) с подтяжкой
    portState = 0xFF;  // Все пины HIGH

    // UV лампы (выходы) по умолчанию выключены: LOW
    for (uint8_t i = 0; i < PCF8574_PIN_COUNT; i++) {
        if (pcf8574Pins[i].isOutput) {
            if (pcf8574Pins[i].pin != PCF8574_FEEDER_RELAY_TANK10 && 
                pcf8574Pins[i].pin != PCF8574_FEEDER_RELAY_TANK20) {
                portState &= ~(1 << pcf8574Pins[i].pin);  // UV лампы - LOW (выключено)
            }
        }
    }

    // Кормушки: гарантируем OFF на старте в зависимости от логики управления
    // invert=true  => OFF=HIGH
    // invert=false => OFF=LOW
    #if !PCF8574_FEEDER_INVERT_LOGIC
        portState &= ~(1 << PCF8574_FEEDER_RELAY_TANK10);
        portState &= ~(1 << PCF8574_FEEDER_RELAY_TANK20);
    #endif
    
    // Записываем начальное состояние
    Wire.beginTransmission(address);
    Wire.write(portState);
    if (Wire.endTransmission() != 0) {
        Serial.println(F("Failed to set initial PCF8574 state!"));
        return false;
    }
    
    lastPortState = portState;
    
    // Читаем начальное состояние входов
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        lastPortState = Wire.read();
    }
    
    Serial.print(F("PCF8574 pins configured: "));
    for (uint8_t i = 0; i < PCF8574_PIN_COUNT; i++) {
        Serial.print(pcf8574Pins[i].name);
        Serial.print(F("(P"));
        Serial.print(pcf8574Pins[i].pin);
        Serial.print(pcf8574Pins[i].isOutput ? F(" OUT) ") : F(" IN) "));
    }
    Serial.println();
    
    isInitialized = true;
    return true;
}

int8_t PCF8574Manager::findPinIndex(uint8_t physicalPin) {
    for (uint8_t i = 0; i < PCF8574_PIN_COUNT; i++) {
        if (pcf8574Pins[i].pin == physicalPin) {
            return i;
        }
    }
    return -1;
}

bool PCF8574Manager::setPin(uint8_t physicalPin, bool state) {
    if (!isInitialized) return false;
    if (physicalPin > 7) return false;  // PCF8574 имеет только 8 пинов (0-7)
    
    int8_t idx = findPinIndex(physicalPin);
    if (idx < 0) return false;  // Пин не найден в конфигурации
    if (!pcf8574Pins[idx].isOutput) return false;  // Пин настроен как вход
    
    if (state) {
        portState |= (1 << physicalPin);
    } else {
        portState &= ~(1 << physicalPin);
    }
    
    Wire.beginTransmission(address);
    Wire.write(portState);
    uint8_t error = Wire.endTransmission();
    return error == 0;
}

bool PCF8574Manager::getPin(uint8_t physicalPin) {
    if (!isInitialized) return false;
    if (physicalPin > 7) return false;  // PCF8574 имеет только 8 пинов (0-7)
    
    int8_t idx = findPinIndex(physicalPin);
    if (idx < 0) return false;  // Пин не найден в конфигурации
    if (pcf8574Pins[idx].isOutput) return false;  // Пин настроен как выход
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        uint8_t data = Wire.read();
        return (data & (1 << physicalPin)) != 0;
    }
    return false;
}

uint8_t PCF8574Manager::getAllPins() {
    if (!isInitialized) return 0;
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        uint8_t data = Wire.read();
        lastPortState = data;
        return data;
    }
    // Если чтение не удалось, не вносим ложные LOW: возвращаем последнее известное состояние
    Serial.println(F("[PCF8574] WARN: getAllPins read failed, using lastPortState"));
    return lastPortState;
}

// Методы для кормушки Tank10
bool PCF8574Manager::setFeederRelayTank10(bool state) {
    // Логика зависит от конфигурации
    #if PCF8574_FEEDER_INVERT_LOGIC
        // Инвертируем логику: HIGH в коде = LOW на выходе PCF8574 = включено через подтяжку
        // Это нужно потому что PCF8574 не может обеспечить достаточный ток в режиме HIGH
        bool pcfState = !state;
    #else
        // Прямая логика: HIGH в коде = HIGH на выходе PCF8574 = включено
        bool pcfState = state;
    #endif
    
    Serial.print(F("[PCF8574] setFeederRelayTank10: код="));
    Serial.print(state ? F("HIGH") : F("LOW"));
    Serial.print(F(", PCF8574="));
    Serial.print(pcfState ? F("HIGH") : F("LOW"));
    Serial.print(F(", пин="));
    Serial.println(PCF8574_FEEDER_RELAY_TANK10);
    bool result = setPin(PCF8574_FEEDER_RELAY_TANK10, pcfState);
    // Проверяем реальное состояние после записи
    delay(10);  // Небольшая задержка для стабилизации
    uint8_t actualState = getAllPins();
    bool actualPinState = (actualState & (1 << PCF8574_FEEDER_RELAY_TANK10)) != 0;
    Serial.print(F("[PCF8574] Реальное состояние пина после записи: "));
    Serial.print(actualPinState ? F("HIGH") : F("LOW"));
    Serial.print(F(" (напряжение должно быть ~"));
    Serial.print(actualPinState ? F("5V") : F("0V"));
    Serial.println(F(")"));
    return result;
}

bool PCF8574Manager::getFeederRelayTank10State() {
    if (!isInitialized) return false;
    bool pcfState = (portState & (1 << PCF8574_FEEDER_RELAY_TANK10)) != 0;
    #if PCF8574_FEEDER_INVERT_LOGIC
        // Инвертируем обратно для отображения состояния
        return !pcfState;
    #else
        return pcfState;
    #endif
}

bool PCF8574Manager::getFeederLimitTank10() {
    // Концевик подключен к земле, поэтому LOW = нажат
    uint8_t pinState = getAllPins();
    return !(pinState & (1 << PCF8574_FEEDER_LIMIT_TANK10));
}

// Методы для кормушки Tank20
bool PCF8574Manager::setFeederRelayTank20(bool state) {
    // Логика зависит от конфигурации
    #if PCF8574_FEEDER_INVERT_LOGIC
        // Инвертируем логику: HIGH в коде = LOW на выходе PCF8574 = включено через подтяжку
        bool pcfState = !state;
    #else
        // Прямая логика: HIGH в коде = HIGH на выходе PCF8574 = включено
        bool pcfState = state;
    #endif
    
    Serial.print(F("[PCF8574] setFeederRelayTank20: код="));
    Serial.print(state ? F("HIGH") : F("LOW"));
    Serial.print(F(", PCF8574="));
    Serial.print(pcfState ? F("HIGH") : F("LOW"));
    Serial.print(F(", пин="));
    Serial.println(PCF8574_FEEDER_RELAY_TANK20);
    bool result = setPin(PCF8574_FEEDER_RELAY_TANK20, pcfState);
    // Проверяем реальное состояние после записи
    delay(10);  // Небольшая задержка для стабилизации
    uint8_t actualState = getAllPins();
    bool actualPinState = (actualState & (1 << PCF8574_FEEDER_RELAY_TANK20)) != 0;
    Serial.print(F("[PCF8574] Реальное состояние пина после записи: "));
    Serial.print(actualPinState ? F("HIGH") : F("LOW"));
    Serial.print(F(" (напряжение должно быть ~"));
    Serial.print(actualPinState ? F("5V") : F("0V"));
    Serial.println(F(")"));
    return result;
}

bool PCF8574Manager::getFeederRelayTank20State() {
    if (!isInitialized) return false;
    bool pcfState = (portState & (1 << PCF8574_FEEDER_RELAY_TANK20)) != 0;
    #if PCF8574_FEEDER_INVERT_LOGIC
        // Инвертируем обратно для отображения состояния
        return !pcfState;
    #else
        return pcfState;
    #endif
}

bool PCF8574Manager::getFeederLimitTank20() {
    // Концевик подключен к земле, поэтому LOW = нажат
    uint8_t pinState = getAllPins();
    return !(pinState & (1 << PCF8574_FEEDER_LIMIT_TANK20));
}

bool PCF8574Manager::setUvLampTank10(bool state) {
    return setPin(0, state);  // UV лампа Tank10 на пине 0
}

bool PCF8574Manager::getUvLampTank10State() {
    if (!isInitialized) return false;
    return (portState & (1 << 0)) != 0;
}

bool PCF8574Manager::setUvLampTank20(bool state) {
    return setPin(1, state);  // UV лампа Tank20 на пине 1
}

bool PCF8574Manager::getUvLampTank20State() {
    if (!isInitialized) return false;
    return (portState & (1 << 1)) != 0;
}

void PCF8574Manager::update() {
    if (!isInitialized) return;
    
    // Читаем текущее состояние всех пинов
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        uint8_t currentState = Wire.read();
        lastPortState = currentState;

        // Если из-за помех \"слетели\" выходы, восстанавливаем их из portState (с ограничением по частоте)
        static unsigned long lastRefreshMs = 0;
        const unsigned long nowMs = millis();

        uint8_t outputMask = 0;
        for (uint8_t i = 0; i < PCF8574_PIN_COUNT; i++) {
            if (pcf8574Pins[i].isOutput) {
                outputMask |= (1 << pcf8574Pins[i].pin);
            }
        }

        if ((((uint8_t)(currentState ^ portState)) & outputMask) != 0 &&
            (nowMs - lastRefreshMs) >= PCF8574_REFRESH_INTERVAL_MS) {
            lastRefreshMs = nowMs;
            Wire.beginTransmission(address);
            Wire.write(portState);
            Wire.endTransmission();
        }
    }
} 