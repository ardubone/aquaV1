#include "wifi.h"
#include <WiFi.h>
#include "secrets.h"

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 10000;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(500);
  }
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

const char* getWiFiSSID() {
  return WiFi.SSID().c_str();
}

const char* getWiFiIP() {
  return WiFi.localIP().toString().c_str();
}
