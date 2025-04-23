#pragma once

#include <Arduino.h>

class Mosfet {
private:
    uint8_t _pin;
    bool _isOn;

public:
    Mosfet(uint8_t pin) : _pin(pin), _isOn(false) {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }

    void init() {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
        Serial.print(F("[MOSFET] Init pin "));
        Serial.println(_pin);
    }

    void turnOn() {
        digitalWrite(_pin, HIGH);
        _isOn = true;
        Serial.print(F("[MOSFET] On, pin: "));
        Serial.println(_pin);
    }

    void turnOff() {
        digitalWrite(_pin, LOW);
        _isOn = false;
        Serial.print(F("[MOSFET] Off, pin: "));
        Serial.println(_pin);
    }

    bool isOn() const {
        return digitalRead(_pin) == HIGH;
    }
}; 