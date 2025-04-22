#pragma once
#include <Arduino.h>
#include <RTClib.h>

// Переменные для работы с временем
extern DateTime tempTime;
extern int selectedTimeField; // 0=day, 1=month, 2=year, 3=hour, 4=minute

// Функции для работы с временем
void initTimeManager();
void updateTimeField(int delta);
void nextTimeField();
bool isLastTimeField();
void saveCurrentTime(RTC_DS1307 &rtc);
void setTempTimeToNow(RTC_DS1307 &rtc); 