#include "net_display.h"
#include "net.h"

void displayWiFiStatus(LiquidCrystal_I2C* lcd) {
  if (isWiFiConnected()) {
    lcd->print(getWiFiSSID());
    lcd->print(" ");
    lcd->print(getWiFiIP());
  } else {
    lcd->print("WiFi disconnected");
  }
} 