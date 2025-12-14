#pragma once

#include <RTClib.h>
#include "config.h"

#ifdef DEBUG_MODE
#include "../src/debug_mocks.h"
#endif

// Callback функция для активации кормушки
typedef bool (*FeederActivateCallback)();

struct ScheduleTime {
    uint8_t hour;
    uint8_t minute;
    uint8_t dayOfWeek;
    uint8_t repeatCount; // Количество повторений
    uint8_t currentRepeat; // Текущее повторение
    bool isCompleted; // Флаг выполнения
    unsigned long lastActivationTime; // Время последней активации
    static const unsigned long REPEAT_INTERVAL; // Интервал между повторениями в мс (1 минута)
};

class AutoFeederScheduler {
private:
    static const uint8_t MAX_SCHEDULES = 20; // Максимальное количество расписаний
    RTC_DS1307* _rtc;
    FeederActivateCallback _activateCallback;
    ScheduleTime _schedule[MAX_SCHEDULES];
    uint8_t _scheduleCount;
    bool _wasActivated;
    uint8_t _currentDay; // Текущий день недели

public:
    AutoFeederScheduler() : _rtc(nullptr), _activateCallback(nullptr), _scheduleCount(0), _wasActivated(false), _currentDay(0) {}

    void init(RTC_DS1307* rtc, FeederActivateCallback activateCallback) {
        _rtc = rtc;
        _activateCallback = activateCallback;
#ifdef DEBUG_MODE
        // В DEBUG_MODE используем мок-время если RTC не инициализирован
        if (_rtc && isRtcInitialized) {
            _currentDay = _rtc->now().dayOfTheWeek();
        } else {
            _currentDay = getMockTime().dayOfTheWeek();
            Serial.println(F("[AUTOFEEDER] Планировщик работает в DEBUG_MODE без RTC"));
        }
#else
        if (_rtc) {
            _currentDay = _rtc->now().dayOfTheWeek();
        }
#endif
        Serial.println(F("[AUTOFEEDER] Инициализация планировщика"));
    }

    bool shouldActivate() {
        if (!_activateCallback) return false;
        
#ifdef DEBUG_MODE
        DateTime now;
        if (_rtc && isRtcInitialized) {
            now = _rtc->now();
        } else {
            now = getMockTime();
        }
#else
        if (!_rtc) return false;
        DateTime now = _rtc->now();
#endif
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
                           (currentMillis - sched.lastActivationTime) >= ScheduleTime::REPEAT_INTERVAL;
            
            // Защита от повторного запуска в ту же минуту (cooldown)
            bool cooldownPassed = (sched.lastActivationTime == 0) || 
                                 (currentMillis - sched.lastActivationTime) >= AUTOFEEDER_START_COOLDOWN_MS;

            if (!sched.isCompleted && (isTimeMatch || canRepeat) && cooldownPassed) {
                if (!_wasActivated) {
                    Serial.print(F("[AUTOFEEDER] Активация по расписанию "));
                    Serial.print(i + 1);
                    Serial.print(F(". Повторение "));
                    Serial.print(sched.currentRepeat + 1);
                    Serial.print(F(" из "));
                    Serial.println(sched.repeatCount);
                    
                    // Вызываем callback для активации
                    if (_activateCallback && _activateCallback()) {
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
                }
            } else if (!isTimeMatch && !canRepeat) {
                _wasActivated = false;
            }
        }
        return false;
    }

    // Добавить расписание
    bool addSchedule(uint8_t hour, uint8_t minute, uint8_t dayOfWeek, uint8_t repeatCount = 1) {
        if (_scheduleCount >= MAX_SCHEDULES) {
            Serial.println(F("[AUTOFEEDER] Достигнут лимит расписаний"));
            return false;
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
        return true;
    }

    // Удалить расписание по индексу
    bool removeSchedule(uint8_t index) {
        if (index >= _scheduleCount) {
            return false;
        }
        
        // Сдвигаем все записи влево (копируем поля вручную из-за const поля)
        for (uint8_t i = index; i < _scheduleCount - 1; i++) {
            _schedule[i].hour = _schedule[i + 1].hour;
            _schedule[i].minute = _schedule[i + 1].minute;
            _schedule[i].dayOfWeek = _schedule[i + 1].dayOfWeek;
            _schedule[i].repeatCount = _schedule[i + 1].repeatCount;
            _schedule[i].currentRepeat = _schedule[i + 1].currentRepeat;
            _schedule[i].isCompleted = _schedule[i + 1].isCompleted;
            _schedule[i].lastActivationTime = _schedule[i + 1].lastActivationTime;
        }
        _scheduleCount--;
        return true;
    }

    // Получить количество расписаний
    uint8_t getScheduleCount() const {
        return _scheduleCount;
    }

    // Получить расписание по индексу
    const ScheduleTime* getSchedule(uint8_t index) const {
        if (index >= _scheduleCount) {
            return nullptr;
        }
        return &_schedule[index];
    }

    // Очистить все расписания
    void clearSchedules() {
        _scheduleCount = 0;
        _wasActivated = false;
    }
}; 