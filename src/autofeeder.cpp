#include "autofeeder.h"
#include "config.h"

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

// Определение статической константы
const unsigned long ScheduleTime::REPEAT_INTERVAL = 60000;

// Статические экземпляры для доступа из callback
AutoFeeder* AutoFeeder::instanceTank10 = nullptr;
AutoFeeder* AutoFeeder::instanceTank20 = nullptr;

// Глобальные экземпляры кормушек
AutoFeeder feederTank10(FEEDER_TANK10);
AutoFeeder feederTank20(FEEDER_TANK20);

// Константа для времени блокировки после срабатывания концевика (в мс)
const unsigned long LIMIT_BLOCK_TIME = 2000;

AutoFeeder::AutoFeeder(FeederType type) 
    : _type(type), _pcfManager(nullptr), _rtc(nullptr),
      _isRelayOn(false), _isLimitIgnored(false), _limitIgnoreStartTime(0),
      _lastScheduleCheck(0), _isBlockedAfterLimitTrigger(false),
      _lastLimitTriggeredTime(0), _limitTriggered(false),
      _currentFeedingType(FEEDING_AUTO) {
}

void AutoFeeder::init(PCF8574Manager* pcfManager, RTC_DS1307* rtc) {
    _pcfManager = pcfManager;
    _rtc = rtc;
    
    // Инициализация планировщика с callback
    FeederActivateCallback callback = (_type == FEEDER_TANK10) ? 
        activateCallbackTank10 : activateCallbackTank20;
    _scheduler.init(_rtc, callback);
    
    // Сохраняем указатель на экземпляр для callback
    if (_type == FEEDER_TANK10) {
        instanceTank10 = this;
    } else {
        instanceTank20 = this;
    }
    
    Serial.print(F("[AUTOFEEDER] Инициализация кормушки "));
    Serial.println(_type == FEEDER_TANK10 ? F("Tank10") : F("Tank20"));
    
    // Проверяем начальное состояние концевика
    bool limitState = (_type == FEEDER_TANK10) ? 
        _pcfManager->getFeederLimitTank10() : _pcfManager->getFeederLimitTank20();
    Serial.print(F("[AUTOFEEDER] Начальное состояние концевика: "));
    Serial.println(limitState ? F("НАЖАТ") : F("НЕ НАЖАТ"));
}

bool AutoFeeder::activateCallbackTank10() {
    if (instanceTank10) {
        return instanceTank10->activateInternal(FEEDING_AUTO);
    }
    return false;
}

bool AutoFeeder::activateCallbackTank20() {
    if (instanceTank20) {
        return instanceTank20->activateInternal(FEEDING_AUTO);
    }
    return false;
}

bool AutoFeeder::activateInternal(FeedingType type) {
    if (!_pcfManager) return false;
#ifdef DEBUG_MODE
    if (!_rtc && !isRtcInitialized) {
        // В DEBUG_MODE можем работать без RTC
    } else if (!_rtc) {
        return false;
    }
#else
    if (!_rtc) return false;
#endif
    
    unsigned long currentTime = millis();
    
    // Проверяем, не заблокирована ли кормушка после срабатывания концевика
    if (_isBlockedAfterLimitTrigger && 
        (currentTime - _lastLimitTriggeredTime < LIMIT_BLOCK_TIME)) {
        Serial.println(F("[AUTOFEEDER] Активация отменена - блокировка после срабатывания концевика"));
        return false;
    }
    
    if (_isRelayOn) {
        return false; // Кормушка уже активирована
    }
    
    // Проверяем, не нажат ли уже концевик
    bool limitPressed = (_type == FEEDER_TANK10) ? 
        _pcfManager->getFeederLimitTank10() : _pcfManager->getFeederLimitTank20();
    
    if (limitPressed) {
        Serial.println(F("[AUTOFEEDER] Активация отменена - концевик уже нажат"));
        return false;
    }
    
    // Сбрасываем блокировку после срабатывания концевика
    _isBlockedAfterLimitTrigger = false;
    
    Serial.print(F("[AUTOFEEDER] Активация кормушки "));
    Serial.print(_type == FEEDER_TANK10 ? F("Tank10") : F("Tank20"));
    Serial.print(F(" ("));
    Serial.print(type == FEEDING_AUTO ? F("АВТО") : F("РУЧНОЕ"));
    Serial.println(F(")"));
    
    // Включаем реле
    bool relayResult = (_type == FEEDER_TANK10) ? 
        _pcfManager->setFeederRelayTank10(true) : 
        _pcfManager->setFeederRelayTank20(true);
    
    if (!relayResult) {
        Serial.println(F("[AUTOFEEDER] Ошибка включения реле"));
        return false;
    }
    
    _isRelayOn = true;
#ifdef DEBUG_MODE
    _relayOnTime = (_rtc && isRtcInitialized) ? _rtc->now() : getMockTime();
#else
    _relayOnTime = _rtc->now();
#endif
    _limitTriggered = false;
    _currentFeedingType = type;  // Сохраняем тип кормления
    
    delay(AUTOFEEDER_RELAY_DELAY);
    
    _limitIgnoreStartTime = millis();
    _isLimitIgnored = true;
    Serial.println(F("[AUTOFEEDER] Начало игнорирования концевика"));
    
    return true;
}

bool AutoFeeder::activateManual() {
    return activateInternal(FEEDING_MANUAL);
}

void AutoFeeder::checkLimitSwitch() {
    if (!_pcfManager || !_isRelayOn || _isLimitIgnored) return;
    
    bool limitPressed = (_type == FEEDER_TANK10) ? 
        _pcfManager->getFeederLimitTank10() : 
        _pcfManager->getFeederLimitTank20();
    
    if (limitPressed) {
        Serial.print(F("[AUTOFEEDER] Концевик нажат ("));
        Serial.print(_type == FEEDER_TANK10 ? F("Tank10") : F("Tank20"));
        Serial.println(F(")"));
        
        // Выключаем реле
        if (_type == FEEDER_TANK10) {
            _pcfManager->setFeederRelayTank10(false);
        } else {
            _pcfManager->setFeederRelayTank20(false);
        }
        
        _isRelayOn = false;
#ifdef DEBUG_MODE
        _relayOffTime = (_rtc && isRtcInitialized) ? _rtc->now() : getMockTime();
        _limitTriggerTime = (_rtc && isRtcInitialized) ? _rtc->now() : getMockTime();
        DateTime now = (_rtc && isRtcInitialized) ? _rtc->now() : getMockTime();
#else
        _relayOffTime = _rtc->now();
        _limitTriggerTime = _rtc->now();
        DateTime now = _rtc->now();
#endif
        
        // Логируем кормление
        // Используем время начала кормления как timestamp
        DateTime logTimestamp = (_relayOnTime.unixtime() > 0) ? _relayOnTime : now;
        _logger.addLog(logTimestamp, _currentFeedingType, _relayOnTime, _relayOffTime, 
                      _limitTriggered, _limitTriggerTime);
        
        // Сбрасываем времена для следующего кормления
        _relayOnTime = DateTime(2000, 1, 1, 0, 0, 0);
        _relayOffTime = DateTime(2000, 1, 1, 0, 0, 0);
        _limitTriggered = false;
        
        // Устанавливаем флаг блокировки и время срабатывания
        unsigned long currentTime = millis();
        _lastLimitTriggeredTime = currentTime;
        _isBlockedAfterLimitTrigger = true;
        Serial.println(F("[AUTOFEEDER] Установлена блокировка повторной активации"));
    }
}

void AutoFeeder::checkSchedule() {
#ifdef DEBUG_MODE
    // В DEBUG_MODE можем работать без RTC
    if (!_rtc && !isRtcInitialized) {
        // Планировщик сам использует мок-время
    } else if (!_rtc) {
        return;
    }
#else
    if (!_rtc) return;
#endif
    
    unsigned long currentTime = millis();
    
    if ((currentTime - _lastScheduleCheck) >= AUTOFEEDER_SCHEDULE_CHECK_INT) {
        _lastScheduleCheck = currentTime;
        if (_scheduler.shouldActivate()) {
            // Активация происходит через callback
        }
    }
}

void AutoFeeder::update() {
    if (!_pcfManager) return;
#ifdef DEBUG_MODE
    // В DEBUG_MODE можем работать без RTC
    if (!_rtc && !isRtcInitialized) {
        // Продолжаем работу с мок-временем
    } else if (!_rtc) {
        return;
    }
#else
    if (!_rtc) return;
#endif
    
    unsigned long currentTime = millis();
    
    // Проверка таймера игнорирования концевика
    if (_isLimitIgnored && 
        (currentTime - _limitIgnoreStartTime) > AUTOFEEDER_LIMIT_IGNORE_TIME) {
        _isLimitIgnored = false;
        Serial.print(F("[AUTOFEEDER] Конец игнорирования концевика ("));
        Serial.print(_type == FEEDER_TANK10 ? F("Tank10") : F("Tank20"));
        Serial.println(F(")"));
    }
    
    // Проверка концевика
    checkLimitSwitch();
    
    // Снимаем блокировку после истечения времени
    if (_isBlockedAfterLimitTrigger && 
        (currentTime - _lastLimitTriggeredTime > LIMIT_BLOCK_TIME)) {
        _isBlockedAfterLimitTrigger = false;
        Serial.print(F("[AUTOFEEDER] Блокировка снята, можно запускать кормушку снова ("));
        Serial.print(_type == FEEDER_TANK10 ? F("Tank10") : F("Tank20"));
        Serial.println(F(")"));
    }
    
    // Проверка расписания
    checkSchedule();
    
    // Ежедневная очистка логов
#ifdef DEBUG_MODE
    DateTime now = (_rtc && isRtcInitialized) ? _rtc->now() : getMockTime();
#else
    DateTime now = _rtc->now();
#endif
    _logger.dailyCleanup(now);
}

// Глобальные функции для инициализации и обновления
void initAutoFeeder(PCF8574Manager* pcfManager, RTC_DS1307* rtc) {
    feederTank10.init(pcfManager, rtc);
    feederTank20.init(pcfManager, rtc);
    Serial.println(F("[AUTOFEEDER] Обе кормушки инициализированы"));
}

void updateAutoFeeder() {
    feederTank10.update();
    feederTank20.update();
}

AutoFeeder* getFeederTank10() {
    return &feederTank10;
}

AutoFeeder* getFeederTank20() {
    return &feederTank20;
}

void setupDefaultFeederSchedule() {
    Serial.println(F("[AUTOFEEDER] Загрузка дефолтного расписания"));
    
    // Дни кормления: Пн(1), Вт(2), Ср(3), Чт(4), Пт(5), Вс(0)
    const uint8_t feedingDays[] = DEFAULT_FEEDING_DAYS;
    
    AutoFeeder* feeder10 = getFeederTank10();
    AutoFeeder* feeder20 = getFeederTank20();
    
    if (!feeder10 || !feeder20) {
        Serial.println(F("[AUTOFEEDER] Ошибка: кормушки не инициализированы"));
        return;
    }
    
    AutoFeederScheduler& scheduler10 = feeder10->getScheduler();
    AutoFeederScheduler& scheduler20 = feeder20->getScheduler();
    
    // Очищаем существующие расписания
    scheduler10.clearSchedules();
    scheduler20.clearSchedules();
    
    // Добавляем расписания для каждого дня
    for (uint8_t i = 0; i < DEFAULT_FEEDING_DAYS_COUNT; i++) {
        uint8_t dayOfWeek = feedingDays[i];
        
        // Первое кормление в 9:30
        scheduler10.addSchedule(DEFAULT_FEEDING_FIRST_HOUR, DEFAULT_FEEDING_FIRST_MINUTE, dayOfWeek, 1);
        scheduler20.addSchedule(DEFAULT_FEEDING_FIRST_HOUR, DEFAULT_FEEDING_FIRST_MINUTE, dayOfWeek, 1);
        
        // Второе кормление в 18:00
        scheduler10.addSchedule(DEFAULT_FEEDING_SECOND_HOUR, DEFAULT_FEEDING_SECOND_MINUTE, dayOfWeek, 1);
        scheduler20.addSchedule(DEFAULT_FEEDING_SECOND_HOUR, DEFAULT_FEEDING_SECOND_MINUTE, dayOfWeek, 1);
    }
    
    Serial.print(F("[AUTOFEEDER] Дефолтное расписание загружено: "));
    Serial.print(DEFAULT_FEEDING_DAYS_COUNT);
    Serial.print(F(" дней, по 2 кормления в день ("));
    Serial.print(DEFAULT_FEEDING_FIRST_HOUR);
    Serial.print(F(":"));
    Serial.print(DEFAULT_FEEDING_FIRST_MINUTE < 10 ? "0" : "");
    Serial.print(DEFAULT_FEEDING_FIRST_MINUTE);
    Serial.print(F(" и "));
    Serial.print(DEFAULT_FEEDING_SECOND_HOUR);
    Serial.print(F(":"));
    Serial.print(DEFAULT_FEEDING_SECOND_MINUTE < 10 ? "0" : "");
    Serial.print(DEFAULT_FEEDING_SECOND_MINUTE);
    Serial.println(F(")"));
}
