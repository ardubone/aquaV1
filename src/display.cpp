#include <Wire.h>
#include <RTClib.h>

#include "display.h"
#include "logger.h"
#include "net.h"
#include "config.h"

#include <functional>

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

extern RTC_DS1307 rtc;
extern DateTime tempTime;
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

void printDate(LiquidCrystal_I2C *lcd, const DateTime &dt)
{
  if (dt.day() < 10)
    lcd->print("0");
  lcd->print(dt.day());
  lcd->print(".");
  if (dt.month() < 10)
    lcd->print("0");
  lcd->print(dt.month());
}

void printTime(LiquidCrystal_I2C *lcd, const DateTime &dt)
{
  if (dt.hour() < 10)
    lcd->print("0");
  lcd->print(dt.hour());
  lcd->print(":");
  if (dt.minute() < 10)
    lcd->print("0");
  lcd->print(dt.minute());
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
  const char *items[] = {"Tank20 Temp", "Tank10 Temp", "Room Data", "Room Pressure", "Back"};
  drawMenu("Text Logs", items, 5, logsMenuPos);
}

void drawLogsGraphMenu()
{
  const char *items[] = {"Tank20 Temp", "Tank10 Temp", "Room Temp", "Humidity", "Room Pressure", "Back"};
  drawMenu("Graph Logs", items, 6, logsMenuPos);
}

void drawTimestamp(const DateTime &ts)
{
  lcd->print("[");
  printDate(lcd, ts);
  lcd->print(" ");
  printTime(lcd, ts);
  lcd->print("]");
}

void drawGenericLogs(
  const char* title,
  const char* valuePrefix,
  std::function<void(LogEntry&, String&)> valueExtractor
) {
  if (logScrollPos != lastLogScroll) {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(title);
    for (uint8_t i = 0; i < 3; i++) {
      int idx = logScrollPos + i;
      if (idx < logCount) {
        lcd->setCursor(0, i + 1);
        drawTimestamp(temperatureLogs[idx].timestamp);
        lcd->print(valuePrefix);
        String valueStr;
        valueExtractor(temperatureLogs[idx], valueStr);
        lcd->print(valueStr);
      }
    }
    drawFooter(F("<Scroll:Encoder>"));
    lastLogScroll = logScrollPos;
  }
}


void drawTank20Logs() {
  drawGenericLogs("Tank20 Temp Logs:", " T20:", [](LogEntry& log, String& out) {
    out = String(log.tank20Temp, 1);
  });
}

void drawTank10Logs() {
  drawGenericLogs("Tank10 Temp Logs:", " T10:", [](LogEntry& log, String& out) {
    out = String(log.tank10Temp, 1);
  });
}

void drawRoomLogs() {
  drawGenericLogs("Room Data Logs:", " Rt:", [](LogEntry& log, String& out) {
    out = String(log.roomTemp, 1) + " Rh:" + String(log.roomHumidity, 0);
  });
}


void drawPressureLogs() {
  drawGenericLogs("Pressure Logs:", " P:", [](LogEntry& log, String& out) {
    out = String(log.roomPressure, 1);
  });
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

  lcd->setCursor(0, 3);
  lcd->print(F("State:"));
  lcd->print(relayState ? "ON " : "OFF");
  printTime(lcd, lastRelayToggleTime);
  lcd->print(" ");
  printDate(lcd, lastRelayToggleTime);
}

void drawRealtime()
{
  static float lastTank20 = NAN;
  static float lastTank10 = NAN;
  static float lastRoomTemp = NAN;
  static float lastHumidity = NAN;
  static float lastPressure = NAN;
  static int lastMinute = -1;
  static bool lastRelayState = false;

  float tank20 = sensors->getTempCByIndex(0);
  float tank10 = sensors->getTempCByIndex(1);
  DateTime now = rtc.now();

  if (tank20 != lastTank20 || tank10 != lastTank10 ||
      roomTemp != lastRoomTemp || roomHumidity != lastHumidity ||
      roomPressure != lastPressure || now.minute() != lastMinute ||
      relayState != lastRelayState)
  {
    lcd->clear();

    lcd->setCursor(0, 0);
    lcd->print(F("I:"));
    lcd->print(tank20, 1);
    lcd->print(F("C O:"));
    lcd->print(tank10, 1);
    lcd->print(F("C"));
    lcd->setCursor(17, 0);
    lcd->print(relayState ? "ON " : "OFF");

    lcd->setCursor(0, 1);
    lcd->print(F("R:"));
    lcd->print(roomTemp, 1);
    lcd->print(F("C H:"));
    lcd->print(roomHumidity, 0);
    lcd->print(F("%"));
    lcd->setCursor(14, 1);
    printTime(lcd, now);

    lcd->setCursor(0, 2);
    lcd->print(F("P:"));
    lcd->print(roomPressure, 1);
    lcd->print(F("mmHg"));
    lcd->setCursor(14, 2);
    printDate(lcd, now);

    lcd->setCursor(0, 3);
    lcd->print(F("IP: "));
    lcd->print(getWiFiIP());

    // Обновляем кэш
    lastTank20 = tank20;
    lastTank10 = tank10;
    lastRoomTemp = roomTemp;
    lastHumidity = roomHumidity;
    lastPressure = roomPressure;
    lastMinute = now.minute();
    lastRelayState = relayState;
  }
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

void drawWiFiStatus()
{
  lcd->clear();

  if (isWiFiConnected())
  {
    lcd->setCursor(0, 0);
    lcd->print(F("Wi-Fi: Connected"));

    lcd->setCursor(0, 1);
    lcd->print(F("SSID: "));
    lcd->print(getWiFiSSID());

    lcd->setCursor(0, 2);
    lcd->print(F("IP: "));
    lcd->print(getWiFiIP());
  }
  else
  {
    lcd->setCursor(0, 0);
    lcd->print(F("Wi-Fi: Not Connected"));
  }

  lcd->setCursor(0, 3);
  lcd->print(F("<Press to return>"));
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

  case LOGS_TANK20_TEXT:
    drawTank20Logs();
    break;
  case LOGS_TANK10_TEXT:
    drawTank10Logs();
    break;
  case LOGS_ROOM_TEXT:
    drawRoomLogs();
    break;
  case LOGS_GRAPH_TANK20:
    drawGraphTank20();
    break;
  case LOGS_GRAPH_TANK10:
    drawGraphTank10();
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

    // раз в 2 секунды — обновить экран
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

  int totalSteps = 7;
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

float getTank20Temp()
{
  #ifdef DEBUG_MODE
  return 25.0; // Моковое значение для отладки
  #else
  return sensors->getTempC(tank20SensorAddr);
  #endif
}

float getTank10Temp()
{
  #ifdef DEBUG_MODE
  return 24.0; // Моковое значение для отладки
  #else
  return sensors->getTempC(tank10SensorAddr);
  #endif
}

void drawGraphTank20()
{
  for (int i = 0; i < logCount; i++)
  {
    graphBuffer[i] = temperatureLogs[i].tank20Temp;
  }
  drawGraphLine("T20:", graphBuffer, logCount);
}

void drawGraphTank10()
{
  for (int i = 0; i < logCount; i++)
  {
    graphBuffer[i] = temperatureLogs[i].tank10Temp;
  }
  drawGraphLine("T10:", graphBuffer, logCount);
}
