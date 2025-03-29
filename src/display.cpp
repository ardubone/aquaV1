#include "display.h"
#include "logger.h"
#include <Wire.h>

static LiquidCrystal_I2C *lcd;
static DallasTemperature *sensors;

static float roomTemp = 0.0;
static float roomHumidity = 0.0;
static float graphBuffer[100]; // Общий буфер для графиков

static int8_t lastSelected = -1;
static int8_t lastTop = -1;
static uint8_t lastHighlight = 255;
static int16_t lastLogScroll = -1;

extern int8_t logsMenuPos;
extern int8_t mainMenuPos;
extern int16_t logScrollPos;

void drawGraphInner();
void drawGraphOuter();
void drawGraphRoomTemp();
void drawGraphHumidity();

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
  //lcd->clear();
}

void setRoomData(float temp, float humidity)
{
  roomTemp = temp;
  roomHumidity = humidity;
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
  const char *items[] = {"Inner Temp", "Outer Temp", "Room Data", "Back"};
  drawMenu("Text Logs", items, 4, logsMenuPos);
}

void drawLogsGraphMenu()
{
  const char *items[] = {"Inner Temp", "Outer Temp", "Room Temp", "Humidity", "Back"};
  drawMenu("Graph Logs", items, 5, logsMenuPos);
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

void drawRealtime()
{
  float inner = sensors->getTempCByIndex(0);
  float outer = sensors->getTempCByIndex(1);

  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(F("Inner Temp: "));
  lcd->print(inner, 1);
  lcd->print("C");

  lcd->setCursor(0, 1);
  lcd->print(F("Out Temp:   "));
  lcd->print(outer, 1);
  lcd->print("C");

  lcd->setCursor(0, 2);
  lcd->print(F("Room: "));
  lcd->print(roomTemp, 1);
  lcd->print("C ");
  lcd->print(roomHumidity, 0);
  lcd->print("%");

  drawFooter(F("<Press to return>"));
}

void showScreen(Screen screen)
{
  resetMenuCache();
  switch (screen)
  {
  case MAIN_MENU:
  {
    const char *mainMenuItems[] = {"Realtime", "Logs"};
    drawMenu("Main Menu", mainMenuItems, 2, mainMenuPos);
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
  case REALTIME:
    drawRealtime();
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
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 0: Пусто
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F}, // 1: ▂
  {0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x1F}, // 2: ▃
  {0x00,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F}, // 3: ▄
  {0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F}, // 4: ▅
  {0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F}, // 5: ▆
  {0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}, // 6: ▇
  {0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}  // 7: █
};

const char *getBar(float value, float minVal, float maxVal)
{
  //const char *bars[] = {" ", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
  const char *bars[] = {
  "\x00",
  "\x01",
  "\x02", 
  "\x03",
  "\x04",
  "\x05",
  "\x06",
  "\x07"
};
  int level = 0;
  if (maxVal > minVal)
  {
    float norm = (value - minVal) / (maxVal - minVal);
    level = constrain((int)(norm * 7), 0, 7);
  }
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

float getRoomTemp()
{
  return roomTemp;
}

float getRoomHumidity()
{
  return roomHumidity;
}
