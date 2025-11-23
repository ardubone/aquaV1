#pragma once

#include <Arduino.h>

// Цвета для индикации
enum AtomLedColor {
    ATOM_LED_OFF = 0,      // Выключено
    ATOM_LED_RED = 1,      // Красный - оба света выкл ручной
    ATOM_LED_GREEN = 2,    // Зеленый - оба света вкл ручной
    ATOM_LED_BLUE = 3,     // Голубой - авто режим
    ATOM_LED_YELLOW = 4    // Желтый - один из светов не в авто режиме
};

// Инициализация LED Atom
void initAtomLed();

// Установить цвет LED
void setAtomLedColor(AtomLedColor color);

// Получить текущий цвет LED
AtomLedColor getAtomLedColor();

