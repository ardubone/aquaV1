#pragma once

#include <Arduino.h>

class LimitSwitch {
private:
    uint8_t _pin;
    bool _lastState;

public:
    LimitSwitch(uint8_t pin) : _pin(pin), _lastState(false) {}

    void init() {
        pinMode(_pin, INPUT_PULLUP);
        _lastState = digitalRead(_pin) == LOW;
        Serial.println(F("[LIMIT_SWITCH] Init"));
    }

    void reset() {
        _lastState = digitalRead(_pin) == LOW;
    }

    bool isTriggered() {
        bool currentState = digitalRead(_pin) == LOW;
        
        // Если состояние изменилось на LOW или если концевик уже нажат (LOW)
        bool triggered = (currentState != _lastState && currentState) || currentState;
        
        // Обновляем _lastState только при изменении состояния
        if (currentState != _lastState) {
            _lastState = currentState;
        }
        
        return triggered;
    }
    
    // Инвертированная логика для концевика, так как он подключен через внутренний подтягивающий резистор
    bool isPressed() {
        // LOW означает, что концевик нажат (замкнут)
        return digitalRead(_pin) == LOW;
    }
}; 