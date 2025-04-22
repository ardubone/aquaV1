#include "time_manager.h"

// Переменные для работы с временем
DateTime tempTime;
int selectedTimeField = 0; // 0=day, 1=month, 2=year, 3=hour, 4=minute

void initTimeManager() {
  selectedTimeField = 0;
}

void updateTimeField(int delta) {
  switch (selectedTimeField) {
    case 0: // День
      tempTime = tempTime + TimeSpan(delta, 0, 0, 0);
      break;
    case 1: // Месяц (грубо)
      tempTime = tempTime + TimeSpan(delta * 30, 0, 0, 0);
      break;
    case 2: // Год
      tempTime = DateTime(tempTime.year() + delta, tempTime.month(), tempTime.day(), 
                          tempTime.hour(), tempTime.minute());
      break;
    case 3: // Часы
      tempTime = tempTime + TimeSpan(0, delta, 0, 0);
      break;
    case 4: // Минуты
      tempTime = tempTime + TimeSpan(0, 0, delta, 0);
      break;
  }
}

void nextTimeField() {
  selectedTimeField++;
  if (selectedTimeField > 4) {
    selectedTimeField = 0;
  }
}

bool isLastTimeField() {
  return selectedTimeField == 4;
}

void saveCurrentTime(RTC_DS1307 &rtc) {
  rtc.adjust(tempTime);
}

void setTempTimeToNow(RTC_DS1307 &rtc) {
  tempTime = rtc.now();
  selectedTimeField = 0;
} 