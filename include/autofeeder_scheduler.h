#pragma once

#include <RTClib.h>
#include "mosfet.h"

struct ScheduleTime {
    uint8_t hour;
    uint8_t minute;
    uint8_t dayOfWeek;
    uint8_t repeatCount; // Количество повторений
    uint8_t currentRepeat; // Текущее повторение
    bool isCompleted; // Флаг выполнения
    unsigned long lastActivationTime; // Время последней активации
    const unsigned long REPEAT_INTERVAL = 60000; // Интервал между повторениями в мс (1 минута)
};

class AutoFeederScheduler {
private:
    static const uint8_t MAX_SCHEDULES = 20; // Максимальное количество расписаний
    RTC_DS1307* _rtc;
    Mosfet* _mosfet;
    ScheduleTime _schedule[MAX_SCHEDULES];
    uint8_t _scheduleCount;
    bool _wasActivated;
    uint8_t _currentDay; // Текущий день недели

public:
    AutoFeederScheduler() : _rtc(nullptr), _mosfet(nullptr), _scheduleCount(0), _wasActivated(false), _currentDay(0) {}

    void init(RTC_DS1307* rtc, Mosfet* mosfet) {
        _rtc = rtc;
        _mosfet = mosfet;
        _currentDay = _rtc->now().dayOfTheWeek();
        Serial.println(F("[AUTOFEEDER] Инициализация планировщика"));
    }

    void addSchedule(uint8_t hour, uint8_t minute, uint8_t dayOfWeek, uint8_t repeatCount = 1) {
        if (_scheduleCount >= MAX_SCHEDULES) {
            Serial.println(F("[AUTOFEEDER] Достигнут лимит расписаний"));
            return;
        }
        
        Serial.print(F("[AUTOFEEDER] Добавление расписания: "));
        Serial.print(hour);
        Serial.print(F(":"));
        Serial.print(minute);
        Serial.print(F(" День недели: "));
        Serial.print(dayOfWeek);
        Serial.print(F(" Повторений: "));
        Serial.println(repeatCount);
        
        ScheduleTime& sched = _schedule[_scheduleCount];
        sched.hour = hour;
        sched.minute = minute;
        sched.dayOfWeek = dayOfWeek;
        sched.repeatCount = repeatCount;
        sched.currentRepeat = 0;
        sched.isCompleted = false;
        sched.lastActivationTime = 0;
        
        _scheduleCount++;
    }

    bool shouldActivate() {
        if (!_rtc || !_mosfet) return false;
        
        DateTime now = _rtc->now();
        uint8_t dayOfWeek = now.dayOfTheWeek();
        unsigned long currentMillis = millis();

        // Проверяем смену дня
        if (dayOfWeek != _currentDay) {
            _currentDay = dayOfWeek;
            // Сбрасываем все расписания при смене дня
            for (uint8_t i = 0; i < _scheduleCount; i++) {
                _schedule[i].isCompleted = false;
                _schedule[i].currentRepeat = 0;
                _schedule[i].lastActivationTime = 0;
            }
            Serial.print(F("[AUTOFEEDER] Новый день: "));
            Serial.println(dayOfWeek);
        }

        for (uint8_t i = 0; i < _scheduleCount; i++) {
            ScheduleTime& sched = _schedule[i];
            
            // Пропускаем расписания не для текущего дня
            if (sched.dayOfWeek != dayOfWeek) continue;

            // Проверка времени для активации
            bool isTimeMatch = sched.hour == now.hour() && sched.minute == now.minute();

            // Проверка интервала между повторениями
            bool canRepeat = sched.currentRepeat > 0 && 
                           (currentMillis - sched.lastActivationTime) >= sched.REPEAT_INTERVAL;

            if (!sched.isCompleted && (isTimeMatch || canRepeat)) {
                if (!_wasActivated && !_mosfet->isOn()) {
                    Serial.print(F("[AUTOFEEDER] Активация по расписанию "));
                    Serial.print(i + 1);
                    Serial.print(F(". Повторение "));
                    Serial.print(sched.currentRepeat + 1);
                    Serial.print(F(" из "));
                    Serial.println(sched.repeatCount);
                    
                    _wasActivated = true;
                    sched.currentRepeat++;
                    sched.lastActivationTime = currentMillis;
                    
                    if (sched.currentRepeat >= sched.repeatCount) {
                        sched.isCompleted = true;
                        Serial.print(F("[AUTOFEEDER] Расписание "));
                        Serial.print(i + 1);
                        Serial.println(F(" выполнено"));
                    }
                    
                    return true;
                }
            } else if (!isTimeMatch && !canRepeat) {
                _wasActivated = false;
            }
        }
        return false;
    }
}; 