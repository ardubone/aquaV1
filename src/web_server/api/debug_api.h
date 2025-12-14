// debug_api.h
#pragma once

// PCF8574 API
void handleDebugPcf8574Status();
void handleDebugPcf8574SetPin();

// Temperature sensors API
void handleDebugTemperatureStatus();
void handleDebugTemperatureSetAddress();
void handleDebugTemperatureLogs();

