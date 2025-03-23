#include "controls.h"
#include <Encoder.h>

extern Encoder encoder;
extern byte logCount;

int8_t mainMenuPos = 0;
int8_t logsMenuPos = 0;
int16_t logScrollPos = 0;
unsigned long lastButtonPress = 0;
long lastEncoderPos = 0;

void handleEncoder(Screen& currentScreen) {
  int32_t newPos = encoder.read();
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
  static bool lastState = true;
  bool currentState = digitalRead(4);

  if (lastState && !currentState && (millis() - lastButtonPress > 200)) {
    lastButtonPress = millis();

    switch (currentScreen) {
      case MAIN_MENU:
        currentScreen = (mainMenuPos == 0) ? REALTIME : LOGS_MENU;
        logsMenuPos = 0;
        break;

      case LOGS_MENU:
        switch (logsMenuPos) {
          case 0: currentScreen = LOGS_TEXT_MENU;  break;
          case 1: currentScreen = LOGS_GRAPH_MENU; break;
          case 2: currentScreen = MAIN_MENU;       break;
        }
        logsMenuPos = 0;
        break;

      case LOGS_TEXT_MENU:
        switch (logsMenuPos) {
          case 0: currentScreen = LOGS_INNER_TEXT; break;
          case 1: currentScreen = LOGS_OUTER_TEXT; break;
          case 2: currentScreen = LOGS_ROOM_TEXT;  break;
          case 3: currentScreen = LOGS_MENU;       break;
        }
        logScrollPos = max(0, logCount - 3);
        break;

      case LOGS_GRAPH_MENU:
        switch (logsMenuPos) {
          case 0: currentScreen = LOGS_GRAPH_INNER; break;
          case 1: currentScreen = LOGS_GRAPH_OUTER; break;
          case 2: currentScreen = LOGS_GRAPH_ROOM;  break;
          case 3: currentScreen = LOGS_GRAPH_HUMID; break;
          case 4: currentScreen = LOGS_MENU;        break;
        }
        logScrollPos = max(0, logCount - 20);
        break;

      case REALTIME:
      case LOGS_INNER_TEXT:
      case LOGS_OUTER_TEXT:
      case LOGS_ROOM_TEXT:
      case LOGS_GRAPH_INNER:
      case LOGS_GRAPH_OUTER:
      case LOGS_GRAPH_ROOM:
      case LOGS_GRAPH_HUMID:
        currentScreen = LOGS_MENU;
        logsMenuPos = 0;
        break;

      default:
        currentScreen = MAIN_MENU;
        break;
    }

    showScreen(currentScreen);
  }

  lastState = currentState;
}