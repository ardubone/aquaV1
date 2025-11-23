#pragma once

#include <Arduino.h>

// Инициализация кнопки Atom
void initAtomButton();

// Обработка кнопки (вызывать в loop)
void updateAtomButton();

// Получить событие одиночного клика (true если был клик, сбрасывается после чтения)
bool getAtomButtonSingleClick();

// Получить событие двойного клика (true если был двойной клик, сбрасывается после чтения)
bool getAtomButtonDoubleClick();

