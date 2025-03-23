// controls.cpp
#include "controls.h"
#include "config.h"
#include <ESP32Encoder.h>
#include <Arduino.h>
#include "display.h"

extern ESP32Encoder encoder;
extern byte logCount;
extern Screen currentScreen;

int8_t mainMenuPos = 0;
int8_t logsMenuPos = 0;
int16_t logScrollPos = 0;
int32_t lastEncoderPos = 0;

void handleEncoder(Screen& currentScreen) {
  int32_t newPos = encoder.getCount();
  int8_t delta = (newPos - lastEncoderPos) / 4;

  if (delta != 0) {
    lastEncoderPos = newPos;

    switch (currentScreen) {
      case MAIN_MENU:
        mainMenuPos = constrain(mainMenuPos + delta, 0, 1);
        break;
      case LOGS_MENU:
        logsMenuPos = constrain(logsMenuPos + delta, 0, 2);
        break;
      case LOGS_TEXT_MENU:
        logsMenuPos = constrain(logsMenuPos + delta, 0, 3);
        break;
      case LOGS_GRAPH_MENU:
        logsMenuPos = constrain(logsMenuPos + delta, 0, 4);
        break;
      case LOGS_INNER_TEXT:
      case LOGS_OUTER_TEXT:
      case LOGS_ROOM_TEXT:
      case LOGS_GRAPH_INNER:
      case LOGS_GRAPH_OUTER:
      case LOGS_GRAPH_ROOM:
      case LOGS_GRAPH_HUMID:
        logScrollPos = constrain(logScrollPos + delta, 0, max(0, logCount - 20));
        break;
      default:
        break;
    }

    showScreen(currentScreen);
  }
}

void handleButton(Screen& currentScreen) {
  static bool buttonState = HIGH;
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        logsMenuPos = 0;

        switch (currentScreen) {
          case MAIN_MENU:
            currentScreen = (mainMenuPos == 0) ? REALTIME : LOGS_MENU;
            break;

          case LOGS_MENU:
            switch (logsMenuPos) {
              case 0: currentScreen = LOGS_TEXT_MENU; break;
              case 1: currentScreen = LOGS_GRAPH_MENU; break;
              case 2: currentScreen = MAIN_MENU; break;
            }
            break;

          case LOGS_TEXT_MENU:
            switch (logsMenuPos) {
              case 0: currentScreen = LOGS_INNER_TEXT; break;
              case 1: currentScreen = LOGS_OUTER_TEXT; break;
              case 2: currentScreen = LOGS_ROOM_TEXT; break;
              case 3: currentScreen = LOGS_MENU; break;
            }
            logScrollPos = max(0, logCount - 3);
            break;

          case LOGS_GRAPH_MENU:
            switch (logsMenuPos) {
              case 0: currentScreen = LOGS_GRAPH_INNER; break;
              case 1: currentScreen = LOGS_GRAPH_OUTER; break;
              case 2: currentScreen = LOGS_GRAPH_ROOM; break;
              case 3: currentScreen = LOGS_GRAPH_HUMID; break;
              case 4: currentScreen = LOGS_MENU; break;
            }
            logScrollPos = max(0, logCount - 20);
            break;

          case REALTIME:
            currentScreen = MAIN_MENU;
            mainMenuPos = 0;
            resetMenuCache();
            showScreen(currentScreen);
            break;

          case LOGS_INNER_TEXT:
          case LOGS_OUTER_TEXT:
          case LOGS_ROOM_TEXT:
          case LOGS_GRAPH_INNER:
          case LOGS_GRAPH_OUTER:
          case LOGS_GRAPH_ROOM:
          case LOGS_GRAPH_HUMID:
            currentScreen = LOGS_MENU;
            logsMenuPos = 0;
            resetMenuCache();
            showScreen(currentScreen);
            break;

          default:
            currentScreen = MAIN_MENU;
            break;
        }

        resetMenuCache();
        showScreen(currentScreen);
      }
    }
  }

  lastButtonState = reading;
}
