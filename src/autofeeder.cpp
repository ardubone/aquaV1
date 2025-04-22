#include <Arduino.h>
#include "autofeeder.h"
#include "config.h"

// Компоненты автокормушки
Button autoFeederButton(AUTOFEEDER_BUTTON_PIN);
LimitSwitch autoFeederLimit(AUTOFEEDER_LIMIT_PIN);
Mosfet autoFeederMosfet(AUTOFEEDER_MOSFET_PIN);
AutoFeederScheduler autoFeederScheduler;

// Флаги и таймеры
unsigned long limitIgnoreStartTime = 0;
bool isLimitIgnored = false;
unsigned long lastScheduleCheck = 0;

// Внешняя переменная RTC
extern RTC_DS1307 rtc;

void initAutoFeeder() {
    autoFeederButton.init();
    autoFeederLimit.init();
    autoFeederMosfet.init();
    autoFeederScheduler.init(&rtc, &autoFeederMosfet);
    
    Serial.println(F("[AUTOFEEDER] Инициализация завершена"));
}

void setupAutoFeederSchedule() {
    // Добавляем расписания кормления
    // Время в формате (час, минута, день недели, количество повторений)
    // День недели: 0 = Воскресенье, 1 = Понедельник, ..., 6 = Суббота
    
    // Понедельник
    autoFeederScheduler.addSchedule(9, 0, 1, 1);
    autoFeederScheduler.addSchedule(19, 0, 1, 1);
    
    // Вторник
    autoFeederScheduler.addSchedule(9, 0, 2, 1);
    autoFeederScheduler.addSchedule(19, 0, 2, 1);
    
    // Среда
    autoFeederScheduler.addSchedule(9, 0, 3, 1);
    autoFeederScheduler.addSchedule(19, 0, 3, 1);
    
    // Четверг
    autoFeederScheduler.addSchedule(9, 0, 4, 1);
    autoFeederScheduler.addSchedule(19, 0, 4, 1);
    
    // Пятница
    autoFeederScheduler.addSchedule(9, 0, 5, 1);
    autoFeederScheduler.addSchedule(19, 0, 5, 1);
    
    // Воскресенье
    autoFeederScheduler.addSchedule(9, 0, 0, 1);
    autoFeederScheduler.addSchedule(19, 0, 0, 1);
}

bool activateFeeder() {
    if (autoFeederMosfet.isOn()) {
        return false; // Кормушка уже активирована
    }
    
    Serial.println(F("[AUTOFEEDER] Активация кормушки"));
    autoFeederMosfet.turnOn();
    delay(AUTOFEEDER_MOSFET_DELAY);
    autoFeederLimit.reset();
    limitIgnoreStartTime = millis();
    isLimitIgnored = true;
    Serial.println(F("[AUTOFEEDER] Начало игнорирования концевика"));
    
    return true;
}

void updateAutoFeeder() {
    unsigned long currentTime = millis();
    
    // Проверка таймера игнорирования концевика
    if (isLimitIgnored && (currentTime - limitIgnoreStartTime) > AUTOFEEDER_LIMIT_IGNORE_TIME) {
        isLimitIgnored = false;
        Serial.println(F("[AUTOFEEDER] Конец игнорирования концевика"));
    }

    // Проверка кнопки
    if (autoFeederButton.isPressed()) {
        activateFeeder();
    }

    // Проверка концевика
    if (!isLimitIgnored && autoFeederMosfet.isOn() && autoFeederLimit.isTriggered()) {
        Serial.println(F("[AUTOFEEDER] Сработал концевик"));
        autoFeederMosfet.turnOff();
    }

    // Проверка расписания
    if ((currentTime - lastScheduleCheck) >= AUTOFEEDER_SCHEDULE_CHECK_INT) {
        lastScheduleCheck = currentTime;
        if (autoFeederScheduler.shouldActivate()) {
            activateFeeder();
        }
    }
} 