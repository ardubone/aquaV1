#pragma once

#include <Arduino.h>

class Button {
private:
    uint8_t _pin;
    bool _lastState;
    unsigned long _lastDebounceTime;
    const unsigned long _debounceDelay = 50;

public:
    Button(uint8_t pin) : _pin(pin), _lastState(false), _lastDebounceTime(0) {}

    void init() {
        pinMode(_pin, INPUT_PULLUP);
        Serial.println("[BUTTON] Инициализация кнопки");
    }

    bool isPressed() {
        bool currentState = digitalRead(_pin) == LOW;
        
        if (currentState != _lastState) {
            _lastDebounceTime = millis();
        }

        if ((millis() - _lastDebounceTime) > _debounceDelay) {
            if (currentState) {
                _lastState = currentState;
                return true;
            }
        }

        _lastState = currentState;
        return false;
    }
}; 