#pragma once
#include <Arduino.h>
#include <WiFi.h>

void initWiFi();
bool isWiFiConnected();
const char* getWiFiSSID();
const char* getWiFiIP();
