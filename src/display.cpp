#include "display.h"
#include "logger.h"
#include <Wire.h>
#include <RTClib.h>
#include "wifi.h"

static LiquidCrystal_I2C *lcd;
static DallasTemperature *sensors;

static float roomTemp = 0.0;
static float roomHumidity = 0.0;
static float roomPressure = 0.0;
static float graphBuffer[100]; // Общий буфер для графиков

static int8_t lastSelected = -1;
static int8_t lastTop = -1;
static uint8_t lastHighlight = 255;
static int16_t lastLogScroll = -1;

extern int8_t logsMenuPos;
extern int8_t mainMenuPos;
extern int16_t logScrollPos;

extern RTC_DS1307 rtc;    // rtc из main.cpp
extern DateTime tempTime; // глобальная временная переменная
extern int selectedTimeField;

extern int8_t relayMenuPos;
extern bool relayState;

extern DateTime lastRelayToggleTime;

void drawGraphInner();
void drawGraphOuter();
void drawGraphRoomTemp();
void drawGraphHumidity();
void drawGraphPressure();
void drawWiFiStatus();

void resetMenuCache()
{
  lastSelected = -1;
  lastTop = -1;
  lastHighlight = 255;
  lastLogScroll = -1;
}

void initDisplay(LiquidCrystal_I2C *lcdPtr, DallasTemperature *sensorsPtr)
{
  lcd = lcdPtr;
  sensors = sensorsPtr;
  lcd->init();
  lcd->backlight();
}

void setRoomData(float temp, float humidity, float pressure)
{
  roomTemp = temp;
  roomHumidity = humidity;
  roomPressure = pressure;
}

void drawFooter(const __FlashStringHelper *text)
{
  lcd->setCursor(0, 3);
  lcd->print(text);
  uint8_t len = strlen_P((PGM_P)text);
  for (int i = len; i < 20; i++)
    lcd->print(" ");
}

void drawMenu(const char *title, const char *items[], uint8_t count, uint8_t selected)
{
  static int8_t lastTop = -1;
  static uint8_t lastHighlight = 255;

  int8_t top = selected;
  if (top != lastTop || lastHighlight != 0)
  {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(F("== "));
    lcd->print(title);
    lcd->print(F(" =="));
    for (uint8_t i = 0; i < 3; i++)
    {
      int idx = top + i;
      lcd->setCursor(0, i + 1);
      if (idx < count)
      {
        lcd->print((i == 0) ? ">" : " ");
        lcd->print(items[idx]);
        uint8_t len = strlen(items[idx]);
        for (uint8_t j = len + 1; j < 20; j++)
          lcd->print(" ");
      }
      else
      {
        lcd->print("                    ");
      }
    }

    lastTop = top;
    lastHighlight = 0;
  }
}

void drawLogsMenu()
{
  const char *items[] = {"Text Logs", "Graph Logs", "Back"};
  drawMenu("Logs Menu", items, 3, logsMenuPos);
}

void drawLogsTextMenu()
{
  const char *items[] = {"Inner Temp", "Outer Temp", "Room Data", "Room Pressure", "Back"};
  drawMenu("Text Logs", items, 5, logsMenuPos);
}

void drawLogsGraphMenu()
{
  const char *items[] = {"Inner Temp", "Outer Temp", "Room Temp", "Humidity", "Room Pressure", "Back"};
  drawMenu("Graph Logs", items, 6, logsMenuPos);
}

void drawTimestamp(const DateTime &ts)
{
  lcd->print("[");
  if (ts.day() < 10)
    lcd->print("0");
  lcd->print(ts.day());
  lcd->print(".");
  if (ts.month() < 10)
    lcd->print("0");
  lcd->print(ts.month());
  lcd->print(" ");
  if (ts.hour() < 10)
    lcd->print("0");
  lcd->print(ts.hour());
  lcd->print(":");
  if (ts.minute() < 10)
    lcd->print("0");
  lcd->print(ts.minute());
  lcd->print("]");
}

void drawInnerLogs()
{
  if (logScrollPos != lastLogScroll)
  {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(F("Inner Temp Logs:"));
    for (uint8_t i = 0; i < 3; i++)
    {
      int idx = logScrollPos + i;
      if (idx < logCount)
      {
        lcd->setCursor(0, i + 1);
        drawTimestamp(temperatureLogs[idx].timestamp);
        lcd->print(" I:");
        lcd->print(temperatureLogs[idx].innerTemp, 1);
      }
    }
    drawFooter(F("<Scroll:Encoder>"));
    lastLogScroll = logScrollPos;
  }
}

void drawOuterLogs()
{
  if (logScrollPos != lastLogScroll)
  {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(F("Outer Temp Logs:"));
    for (uint8_t i = 0; i < 3; i++)
    {
      int idx = logScrollPos + i;
      if (idx < logCount)
      {
        lcd->setCursor(0, i + 1);
        drawTimestamp(temperatureLogs[idx].timestamp);
        lcd->print(" O:");
        lcd->print(temperatureLogs[idx].outerTemp, 1);
      }
    }
    drawFooter(F("<Scroll:Encoder>"));
    lastLogScroll = logScrollPos;
  }
}

void drawRoomLogs()
{
  if (logScrollPos != lastLogScroll)
  {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(F("Room Data Logs:"));
    for (uint8_t i = 0; i < 3; i++)
    {
      int idx = logScrollPos + i;
      if (idx < logCount)
      {
        lcd->setCursor(0, i + 1);
        drawTimestamp(temperatureLogs[idx].timestamp);
        lcd->print(" Rt:");
        lcd->print(temperatureLogs[idx].roomTemp, 1);
        lcd->print(" Rh:");
        lcd->print(temperatureLogs[idx].roomHumidity, 0);
      }
    }
    drawFooter(F("<Scroll:Encoder>"));
    lastLogScroll = logScrollPos;
  }
}

void drawPressureLogs()
{
  if (logScrollPos != lastLogScroll)
  {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(F("Pressure Logs:"));
    for (uint8_t i = 0; i < 3; i++)
    {
      int idx = logScrollPos + i;
      if (idx < logCount)
      {
        lcd->setCursor(0, i + 1);
        drawTimestamp(temperatureLogs[idx].timestamp);
        lcd->print(" P:");
        lcd->print(temperatureLogs[idx].roomPressure, 1);
      }
    }
    drawFooter(F("<Scroll:Encoder>"));
    lastLogScroll = logScrollPos;
  }
}

void drawRelayMenu()
{
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(F("Relay Control"));

  lcd->setCursor(0, 1);
  lcd->print(relayMenuPos == 0 ? ">" : " ");
  lcd->print(F(" Toggle Relay "));
  lcd->print(relayState ? "[OFF]" : "[ON]");

  lcd->setCursor(0, 2);
  lcd->print(relayMenuPos == 1 ? ">" : " ");
  lcd->print(F(" Back"));

  // Строка 3: текущее состояние + время
  lcd->setCursor(0, 3);
  lcd->print(F("State:"));
  lcd->print(relayState ? "ON " : "OFF");

  // Время (часы:минуты)
  if (lastRelayToggleTime.hour() < 10)
    lcd->print("0");
  lcd->print(lastRelayToggleTime.hour());
  lcd->print(":");
  if (lastRelayToggleTime.minute() < 10)
    lcd->print("0");
  lcd->print(lastRelayToggleTime.minute());
  lcd->print(" ");

  // Дата (дд.мм)
  if (lastRelayToggleTime.day() < 10)
    lcd->print("0");
  lcd->print(lastRelayToggleTime.day());
  lcd->print(".");
  if (lastRelayToggleTime.month() < 10)
    lcd->print("0");
  lcd->print(lastRelayToggleTime.month());
}

void drawRealtime()
{
  float inner = sensors->getTempCByIndex(0);
  float outer = sensors->getTempCByIndex(1);

  DateTime now = rtc.now();

  lcd->clear();

  // Строка 0: температуры + иконка времени
  lcd->setCursor(0, 0);
  lcd->print(F("I:"));
  lcd->print(inner, 1);
  lcd->print(F("C O:"));
  lcd->print(outer, 1);
  lcd->print(F("C"));

  lcd->setCursor(17, 0);
  lcd->print(relayState ? "ON " : "OFF");

  // Строка 1: комната и влажность + время
  lcd->setCursor(0, 1);
  lcd->print(F("R:"));
  lcd->print(roomTemp, 1);
  lcd->print(F("C H:"));
  lcd->print(roomHumidity, 0);
  lcd->print(F("%"));

  lcd->setCursor(14, 1);
  if (now.hour() < 10)
    lcd->print("0");
  lcd->print(now.hour());
  lcd->print(":");
  if (now.minute() < 10)
    lcd->print("0");
  lcd->print(now.minute());

  // Строка 2: давление + дата
  lcd->setCursor(0, 2);
  lcd->print(F("P:"));
  lcd->print(roomPressure, 1);
  lcd->print(F("mmHg"));

  lcd->setCursor(14, 2);
  if (now.day() < 10)
    lcd->print("0");
  lcd->print(now.day());
  lcd->print(".");
  if (now.month() < 10)
    lcd->print("0");
  lcd->print(now.month());

  // Строка 3: подвал
  drawFooter(F("<Press to return>"));
}

void drawSetTimeMenu(DateTime &tempTime, int selectedField)
{
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(F("Set Date & Time"));

  // Строка даты
  lcd->setCursor(0, 1);
  lcd->print(F("Date: "));
  if (selectedField == 0)
    lcd->print(">");
  else
    lcd->print(" ");
  if (tempTime.day() < 10)
    lcd->print("0");
  lcd->print(tempTime.day());
  lcd->print(".");
  if (selectedField == 1)
    lcd->print(">");
  else
    lcd->print(" ");
  if (tempTime.month() < 10)
    lcd->print("0");
  lcd->print(tempTime.month());
  lcd->print(".");
  if (selectedField == 2)
    lcd->print(">");
  else
    lcd->print(" ");
  lcd->print(tempTime.year());

  // Строка времени
  lcd->setCursor(0, 2);
  lcd->print(F("Time: "));
  if (selectedField == 3)
    lcd->print(">");
  else
    lcd->print(" ");
  if (tempTime.hour() < 10)
    lcd->print("0");
  lcd->print(tempTime.hour());
  lcd->print(":");
  if (selectedField == 4)
    lcd->print(">");
  else
    lcd->print(" ");
  if (tempTime.minute() < 10)
    lcd->print("0");
  lcd->print(tempTime.minute());

  // Инструкция
  lcd->setCursor(0, 3);
  if (selectedField < 4)
    lcd->print(F("<Press:Next Field>"));
  else
    lcd->print(F("<Press:Save Time>"));
}

void drawWiFiStatus() {
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(F("Wi-Fi Status:"));

  lcd->setCursor(0, 1);
  if (isWiFiConnected()) {
    lcd->print(F("Connected to:"));
    lcd->setCursor(0, 2);
    lcd->print(getWiFiSSID());

    lcd->setCursor(0, 3);
    lcd->print(getWiFiIP());
  } else {
    lcd->print(F("Not Connected"));
  }

  drawFooter(F("<Press to return>"));
}

void showScreen(Screen screen)
{
  resetMenuCache();
  switch (screen)
  {
  case MAIN_MENU:
  {
    const char *mainMenuItems[] = {"Realtime", "Logs", "Set Time", "Relay Control", "WiFi Status"};
    drawMenu("Main Menu", mainMenuItems, 5, mainMenuPos);
    break;
  }
  case LOGS_MENU:
    drawLogsMenu();
    break;
  case LOGS_TEXT_MENU:
    drawLogsTextMenu();
    break;
  case LOGS_GRAPH_MENU:
    drawLogsGraphMenu();
    break;

  case LOGS_INNER_TEXT:
    drawInnerLogs();
    break;
  case LOGS_OUTER_TEXT:
    drawOuterLogs();
    break;
  case LOGS_ROOM_TEXT:
    drawRoomLogs();
    break;
  case LOGS_GRAPH_INNER:
    drawGraphInner();
    break;
  case LOGS_GRAPH_OUTER:
    drawGraphOuter();
    break;
  case LOGS_GRAPH_ROOM:
    drawGraphRoomTemp();
    break;
  case LOGS_GRAPH_HUMID:
    drawGraphHumidity();
    break;
  case LOGS_ROOM_PRESSURE:
    drawPressureLogs();
    break;
  case LOGS_GRAPH_PRESSURE:
    drawGraphPressure();
    break;
  case REALTIME:
    drawRealtime();
    break;
  case SET_TIME_MENU:
    tempTime = rtc.now();
    selectedTimeField = 0;
    drawSetTimeMenu(tempTime, selectedTimeField);
    break;
  case RELAY_CONTROL_MENU:
    drawRelayMenu();
    break;
  case WIFI_STATUS_MENU:
    drawWiFiStatus();
    break;
  }
}

void updateScreen(Screen screen)
{
  static unsigned long lastUpdate = 0;
  static unsigned long lastTempRequest = 0;

  if (screen == REALTIME)
  {
    unsigned long now = millis();

    // раз в 1.5 секунды — запросить температуру
    if (now - lastTempRequest > 1500)
    {
      sensors->requestTemperatures();
      lastTempRequest = now;
    }

    // раз в 1 секунду — обновить экран
    if (now - lastUpdate > 2000)
    {
      drawRealtime();
      lastUpdate = now;
    }
  }
}

byte customChars[8][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0: Пусто
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // 1: Линия внизу (0x04 = 00100)
    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, // 2: Линия в центре
    {0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00}, // 3: Выше центра
    {0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 4
    {0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 5
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00}, // 6: Ниже центра
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00}  // 7
};

const char *getBar(float value, float minVal, float maxVal)
{
  const char *bars[] = {"\x00", "\x01", "\x02", "\x03", "\x04", "\x05", "\x06", "\x07"};

  // int level = 0;
  // if (maxVal > minVal)
  // {
  //   float norm = (value - minVal) / (maxVal - minVal);
  //   level = constrain((int)(norm * 7), 0, 7);
  // }

  // Рассчитываем позицию линии с шагом 1 градус
  int totalSteps = 7; // 7 доступных уровней для линии
  int step = constrain(value - minVal, 0, maxVal - minVal);
  int level = map(step, 0, maxVal - minVal, 0, totalSteps);

  return bars[level];
}

void drawGraphLine(const char *label, float values[], int count)
{
  float minVal = values[0];
  float maxVal = values[0];
  for (int i = 1; i < count; i++)
  {
    if (values[i] < minVal)
      minVal = values[i];
    if (values[i] > maxVal)
      maxVal = values[i];
  }

  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(label);
  lcd->print(" ");
  lcd->print(values[count - 1], 1);

  lcd->setCursor(0, 1);
  for (int i = max(0, count - 10); i < count; i++)
  {
    lcd->print(getBar(values[i], minVal, maxVal));
  }

  lcd->setCursor(0, 2);
  lcd->print(F("Min:"));
  lcd->print(minVal, 1);
  lcd->print(F(" Max:"));
  lcd->print(maxVal, 1);

  drawFooter(F("<Press to return>"));
}

void drawGraphInner()
{
  for (int i = 0; i < logCount; i++)
  {
    graphBuffer[i] = temperatureLogs[i].innerTemp;
  }
  drawGraphLine("I:", graphBuffer, logCount);
}

void drawGraphOuter()
{
  for (int i = 0; i < logCount; i++)
  {
    graphBuffer[i] = temperatureLogs[i].outerTemp;
  }
  drawGraphLine("O:", graphBuffer, logCount);
}

void drawGraphRoomTemp()
{
  for (int i = 0; i < logCount; i++)
  {
    graphBuffer[i] = temperatureLogs[i].roomTemp;
  }
  drawGraphLine("Rt:", graphBuffer, logCount);
}

void drawGraphHumidity()
{
  for (int i = 0; i < logCount; i++)
  {
    graphBuffer[i] = temperatureLogs[i].roomHumidity;
  }
  drawGraphLine("Rh:", graphBuffer, logCount);
}

void drawGraphPressure()
{
  for (int i = 0; i < logCount; i++)
  {
    graphBuffer[i] = temperatureLogs[i].roomPressure;
  }
  drawGraphLine("P:", graphBuffer, logCount);
}

float getRoomTemp()
{
  return roomTemp;
}

float getRoomHumidity()
{
  return roomHumidity;
}

float getRoomPressure()
{
  return roomPressure;
}
