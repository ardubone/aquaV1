// controls.cpp
#include "controls.h"
#include "config.h"
#include <ESP32Encoder.h>
#include <Arduino.h>
#include <RTClib.h>
#include "display.h"
#include "relay.h"
#include "time_manager.h"

extern ESP32Encoder encoder;
extern byte logCount;
extern Screen currentScreen;
extern RTC_DS1307 rtc;

int8_t mainMenuPos = 0;
int8_t logsMenuPos = 0;
int16_t logScrollPos = 0;
int32_t lastEncoderPos = 0;

int8_t relayMenuPos = 0;

void handleEncoder(Screen &currentScreen)
{
  int32_t newPos = encoder.getCount();
  int8_t delta = (newPos - lastEncoderPos) / 2;

  if (delta != 0)
  {
    lastEncoderPos = newPos;

    switch (currentScreen)
    {
    case MAIN_MENU:
      mainMenuPos = constrain(mainMenuPos + delta, 0, 4);
      showScreen(currentScreen);
      break;

    case LOGS_MENU:
      logsMenuPos = constrain(logsMenuPos + delta, 0, 2);
      showScreen(currentScreen);
      break;

    case RELAY_CONTROL_MENU:
      relayMenuPos = constrain(relayMenuPos + delta, 0, 1); // 0=Toggle, 1=Back
      drawRelayMenu();
      break;

    case LOGS_TEXT_MENU:
      logsMenuPos = constrain(logsMenuPos + delta, 0, 4);
      showScreen(currentScreen);
      break;

    case LOGS_GRAPH_MENU:
      logsMenuPos = constrain(logsMenuPos + delta, 0, 5);
      showScreen(currentScreen);
      break;

    case LOGS_TANK20_TEXT:
    case LOGS_TANK10_TEXT:
    case LOGS_ROOM_TEXT:
    case LOGS_ROOM_PRESSURE:
    case LOGS_GRAPH_TANK20:
    case LOGS_GRAPH_TANK10:
    case LOGS_GRAPH_ROOM:
    case LOGS_GRAPH_HUMID:
    case LOGS_GRAPH_PRESSURE:
      logScrollPos = constrain(logScrollPos + delta, 0, max(0, logCount - 20));
      showScreen(currentScreen);
      break;
    case SET_TIME_MENU:
      updateTimeField(delta);
      drawSetTimeMenu(tempTime, selectedTimeField);
      break;

    default:
      break;
    }
  }
}

void handleButton(Screen &currentScreen)
{
  static bool buttonState = HIGH;
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (reading != buttonState)
    {
      buttonState = reading;
      if (buttonState == LOW)
      {
        switch (currentScreen)
        {
        case MAIN_MENU:
          if (mainMenuPos == 0)
          {
            currentScreen = REALTIME;
          }
          else if (mainMenuPos == 1)
          {
            currentScreen = LOGS_MENU;
            logsMenuPos = 0;
          }
          else if (mainMenuPos == 2)
          {
            setTempTimeToNow(rtc);
            currentScreen = SET_TIME_MENU;
          }
          else if (mainMenuPos == 3)
          {
            currentScreen = RELAY_CONTROL_MENU;
          }
          else if (mainMenuPos == 4)
          {
            currentScreen = WIFI_STATUS_MENU;
          }

          showScreen(currentScreen);
          break;

        case LOGS_MENU:
          switch (logsMenuPos)
          {
          case 0:
            currentScreen = LOGS_TEXT_MENU;
            break;
          case 1:
            currentScreen = LOGS_GRAPH_MENU;
            break;
          case 2:
            currentScreen = MAIN_MENU;
            break;
          }
          logsMenuPos = 0;
          showScreen(currentScreen);
          break;

        case LOGS_TEXT_MENU:
          switch (logsMenuPos)
          {
          case 0:
            currentScreen = LOGS_TANK20_TEXT;
            logScrollPos = max(0, logCount - 20);
            break;
          case 1:
            currentScreen = LOGS_TANK10_TEXT;
            logScrollPos = max(0, logCount - 20);
            break;
          case 2:
            currentScreen = LOGS_ROOM_TEXT;
            logScrollPos = max(0, logCount - 20);
            break;
          case 3:
            currentScreen = LOGS_ROOM_PRESSURE;
            logScrollPos = max(0, logCount - 20);
            break;
          case 4:
            currentScreen = LOGS_MENU;
            logsMenuPos = 0;
            break;
          }
          showScreen(currentScreen);
          break;

        case LOGS_GRAPH_MENU:
          switch (logsMenuPos)
          {
          case 0:
            currentScreen = LOGS_GRAPH_TANK20;
            logScrollPos = max(0, logCount - 20);
            break;
          case 1:
            currentScreen = LOGS_GRAPH_TANK10;
            logScrollPos = max(0, logCount - 20);
            break;
          case 2:
            currentScreen = LOGS_GRAPH_ROOM;
            logScrollPos = max(0, logCount - 20);
            break;
          case 3:
            currentScreen = LOGS_GRAPH_HUMID;
            logScrollPos = max(0, logCount - 20);
            break;
          case 4:
            currentScreen = LOGS_GRAPH_PRESSURE;
            logScrollPos = max(0, logCount - 20);
            break;
          case 5:
            currentScreen = LOGS_MENU;
            logsMenuPos = 0;
            break;
          }
          showScreen(currentScreen);
          break;

        case REALTIME:
          currentScreen = MAIN_MENU;
          mainMenuPos = 0;
          showScreen(currentScreen);
          break;

        case LOGS_TANK20_TEXT:
        case LOGS_TANK10_TEXT:
        case LOGS_ROOM_TEXT:
        case LOGS_ROOM_PRESSURE:
          currentScreen = LOGS_TEXT_MENU;
          logsMenuPos = 0;
          showScreen(currentScreen);
          break;
        case LOGS_GRAPH_TANK20:
        case LOGS_GRAPH_TANK10:
        case LOGS_GRAPH_ROOM:
        case LOGS_GRAPH_HUMID:
        case LOGS_GRAPH_PRESSURE:
          currentScreen = LOGS_GRAPH_MENU;
          logsMenuPos = 0;
          showScreen(currentScreen);
          break;

        case SET_TIME_MENU:
          nextTimeField();
          if (isLastTimeField())
          {
            saveCurrentTime(rtc);
            currentScreen = MAIN_MENU;
            mainMenuPos = 0;
            showScreen(currentScreen);
          }
          else
          {
            drawSetTimeMenu(tempTime, selectedTimeField);
          }
          break;

        case RELAY_CONTROL_MENU:
          if (relayMenuPos == 0)
          {
            toggleRelay(rtc.now());
          }
          else if (relayMenuPos == 1)
          {
            currentScreen = MAIN_MENU;
            mainMenuPos = 0;
          }
          drawRelayMenu();
          break;
        case WIFI_STATUS_MENU:
          currentScreen = MAIN_MENU;
          mainMenuPos = 0;
          showScreen(currentScreen);
          break;

        default:
          currentScreen = MAIN_MENU;
          showScreen(currentScreen);
          break;
        }
      }
    }
  }

  lastButtonState = reading;
}