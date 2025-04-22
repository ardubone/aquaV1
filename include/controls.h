// controls.h
#pragma once
#include <Arduino.h>
// #include "display.h"
#include <RTClib.h>
extern DateTime lastRelayToggleTime;
extern bool relayManualOverride;

extern int8_t mainMenuPos;
extern int8_t logsMenuPos;
extern int16_t logScrollPos;
extern int32_t lastEncoderPos;
// extern bool buttonPressed;
extern unsigned long lastButtonPress;
extern bool relayState;
extern int8_t relayMenuPos;


void handleEncoder(int& currentScreen);
void handleButton(int& currentScreen);
