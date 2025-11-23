#include "atom_led.h"
#include <FastLED.h>

#define ATOM_LED_PIN 27
#define NUM_LEDS 1
#define LED_TYPE SK6812
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
static AtomLedColor currentColor = ATOM_LED_OFF;

void initAtomLed() {
    FastLED.addLeds<LED_TYPE, ATOM_LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(50);  // Устанавливаем яркость (0-255)
    
    setAtomLedColor(ATOM_LED_OFF);
    Serial.println(F("[ATOM_LED] Инициализация SK6812 LED на GPIO 27"));
}

void setAtomLedColor(AtomLedColor color) {
    currentColor = color;
    
    CRGB ledColor = CRGB::Black;  // По умолчанию выключено
    
    switch (color) {
        case ATOM_LED_OFF:
            ledColor = CRGB::Black;
            break;
        case ATOM_LED_RED:
            ledColor = CRGB::Red;
            break;
        case ATOM_LED_GREEN:
            ledColor = CRGB::Green;
            break;
        case ATOM_LED_BLUE:
            ledColor = CRGB::Blue;
            break;
        case ATOM_LED_YELLOW:
            ledColor = CRGB::Yellow;
            break;
    }
    
    leds[0] = ledColor;
    FastLED.show();
}

AtomLedColor getAtomLedColor() {
    return currentColor;
}
