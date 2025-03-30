#pragma once
#include <Arduino.h>
#include <WiFi.h>

void initWiFi();
bool isWiFiConnected();
String getWiFiSSID();
String getWiFiIP();

