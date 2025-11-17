#pragma once

#include <Arduino.h>
#include <RTClib.h>
#include "autofeeder_scheduler.h"
#include "feeder_logger.h"
#include "pcf8574_manager.h"

// Тип кормушки
enum FeederType {
    FEEDER_TANK10 = 0,
    FEEDER_TANK20 = 1
};

// Класс для управления одной кормушкой
class AutoFeeder {
private:
    FeederType _type;
    PCF8574Manager* _pcfManager;
    RTC_DS1307* _rtc;
    AutoFeederScheduler _scheduler;
    FeederLogger _logger;
    
    // Состояние кормушки
    bool _isRelayOn;
    bool _isLimitIgnored;
    unsigned long _limitIgnoreStartTime;
    unsigned long _lastScheduleCheck;
    bool _isBlockedAfterLimitTrigger;
    unsigned long _lastLimitTriggeredTime;
    DateTime _relayOnTime;
    DateTime _relayOffTime;
    bool _limitTriggered;
    DateTime _limitTriggerTime;
    FeedingType _currentFeedingType;  // Тип текущего кормления
    
    // Callback для активации (используется планировщиком)
    static bool activateCallbackTank10();
    static bool activateCallbackTank20();
    
    // Внутренние методы
    bool activateInternal(FeedingType type);
    void checkLimitSwitch();
    void checkSchedule();
    
public:
    AutoFeeder(FeederType type);
    
    // Инициализация
    void init(PCF8574Manager* pcfManager, RTC_DS1307* rtc);
    
    // Обновление состояния (вызывать в loop)
    void update();
    
    // Ручная активация кормушки
    bool activateManual();
    
    // Получить логи кормления
    const FeederLogger& getLogger() const { return _logger; }
    
    // Получить планировщик для управления календарем
    AutoFeederScheduler& getScheduler() { return _scheduler; }
    
    // Получить состояние реле
    bool isRelayOn() const { return _isRelayOn; }
    
    // Получить тип кормушки
    FeederType getType() const { return _type; }
    
    // Статические экземпляры для доступа из callback
    static AutoFeeder* instanceTank10;
    static AutoFeeder* instanceTank20;
};

// Глобальные функции для работы с кормушками
void initAutoFeeder(PCF8574Manager* pcfManager, RTC_DS1307* rtc);
void updateAutoFeeder();
AutoFeeder* getFeederTank10();
AutoFeeder* getFeederTank20();
void setupDefaultFeederSchedule();  // Загрузка дефолтного расписания в обе кормушки
