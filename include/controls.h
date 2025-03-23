// controls.h
#pragma once
#include <Arduino.h>
#include "display.h"

extern int8_t mainMenuPos;
extern int8_t logsMenuPos;
extern int16_t logScrollPos;
extern int32_t lastEncoderPos;
// extern bool buttonPressed;
extern unsigned long lastButtonPress;

void handleEncoder(Screen& currentScreen);
void handleButton(Screen& currentScreen);
