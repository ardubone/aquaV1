// autofeeder_api.cpp
#include "autofeeder_api.h"
#include "../../../include/web_server.h"
#include "../../../include/autofeeder.h"
#include "../../../include/feeder_logger.h"

extern WebServer server;

// Вспомогательная функция для получения логов в JSON
String getFeederLogsJSON(const FeederLogger& logger) {
    String json = "[";
    uint8_t logCount = logger.getLogCount();
    
    for (uint8_t i = 0; i < logCount; i++) {
        const FeedingLog& log = logger.getLog(i);
        
        if (i > 0) json += ",";
        json += "{";
        json += "\"timestamp\":\"" + String(log.timestamp.year()) + "-" + 
                String(log.timestamp.month() < 10 ? "0" : "") + String(log.timestamp.month()) + "-" +
                String(log.timestamp.day() < 10 ? "0" : "") + String(log.timestamp.day()) + " " +
                String(log.timestamp.hour() < 10 ? "0" : "") + String(log.timestamp.hour()) + ":" +
                String(log.timestamp.minute() < 10 ? "0" : "") + String(log.timestamp.minute()) + "\",";
        json += "\"type\":\"" + String(log.type == FEEDING_AUTO ? "АВТО" : "РУЧНОЕ") + "\",";
        json += "\"relayOnTime\":\"" + String(log.relayOnTime.year()) + "-" + 
                String(log.relayOnTime.month() < 10 ? "0" : "") + String(log.relayOnTime.month()) + "-" +
                String(log.relayOnTime.day() < 10 ? "0" : "") + String(log.relayOnTime.day()) + " " +
                String(log.relayOnTime.hour() < 10 ? "0" : "") + String(log.relayOnTime.hour()) + ":" +
                String(log.relayOnTime.minute() < 10 ? "0" : "") + String(log.relayOnTime.minute()) + "\",";
        json += "\"relayOffTime\":\"" + String(log.relayOffTime.year()) + "-" + 
                String(log.relayOffTime.month() < 10 ? "0" : "") + String(log.relayOffTime.month()) + "-" +
                String(log.relayOffTime.day() < 10 ? "0" : "") + String(log.relayOffTime.day()) + " " +
                String(log.relayOffTime.hour() < 10 ? "0" : "") + String(log.relayOffTime.hour()) + ":" +
                String(log.relayOffTime.minute() < 10 ? "0" : "") + String(log.relayOffTime.minute()) + "\",";
        json += "\"limitTriggered\":" + String(log.limitTriggered ? "true" : "false") + ",";
        if (log.limitTriggered) {
            json += "\"limitTriggerTime\":\"" + String(log.limitTriggerTime.year()) + "-" + 
                    String(log.limitTriggerTime.month() < 10 ? "0" : "") + String(log.limitTriggerTime.month()) + "-" +
                    String(log.limitTriggerTime.day() < 10 ? "0" : "") + String(log.limitTriggerTime.day()) + " " +
                    String(log.limitTriggerTime.hour() < 10 ? "0" : "") + String(log.limitTriggerTime.hour()) + ":" +
                    String(log.limitTriggerTime.minute() < 10 ? "0" : "") + String(log.limitTriggerTime.minute()) + "\"";
        } else {
            json += "\"limitTriggerTime\":\"\"";
        }
        json += "}";
    }
    json += "]";
    return json;
}

// Вспомогательная функция для получения расписаний в JSON
String getScheduleJSON(AutoFeederScheduler& scheduler) {
    String json = "[";
    uint8_t scheduleCount = scheduler.getScheduleCount();
    
    for (uint8_t i = 0; i < scheduleCount; i++) {
        const ScheduleTime* sched = scheduler.getSchedule(i);
        if (!sched) continue;
        
        if (i > 0) json += ",";
        json += "{";
        json += "\"index\":" + String(i) + ",";
        json += "\"hour\":" + String(sched->hour) + ",";
        json += "\"minute\":" + String(sched->minute) + ",";
        json += "\"dayOfWeek\":" + String(sched->dayOfWeek) + ",";
        json += "\"repeatCount\":" + String(sched->repeatCount);
        json += "}";
    }
    json += "]";
    return json;
}

// API для кормушки Tank10
void handleAutoFeederTank10Activate() {
    AutoFeeder* feeder = getFeederTank10();
    if (feeder) {
        bool result = feeder->activateManual();
        String json = "{\"success\":" + String(result ? "true" : "false") + "}";
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}

void handleAutoFeederTank10Status() {
    AutoFeeder* feeder = getFeederTank10();
    if (feeder) {
        String json = "{";
        json += "\"relayOn\":" + String(feeder->isRelayOn() ? "true" : "false") + ",";
        json += "\"type\":\"Tank10\"";
        json += "}";
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}

void handleAutoFeederTank10Logs() {
    AutoFeeder* feeder = getFeederTank10();
    if (feeder) {
        String json = getFeederLogsJSON(feeder->getLogger());
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}

void handleAutoFeederTank10ScheduleAdd() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    AutoFeeder* feeder = getFeederTank10();
    if (!feeder) {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
        return;
    }
    
    String body = server.arg("plain");
    
    // Парсим JSON (упрощенный вариант)
    uint8_t hour = server.arg("hour").toInt();
    uint8_t minute = server.arg("minute").toInt();
    uint8_t dayOfWeek = server.arg("dayOfWeek").toInt();
    uint8_t repeatCount = server.arg("repeatCount").toInt();
    if (repeatCount == 0) repeatCount = 1;
    
    AutoFeederScheduler& scheduler = feeder->getScheduler();
    bool result = scheduler.addSchedule(hour, minute, dayOfWeek, repeatCount);
    
    String json = "{\"success\":" + String(result ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

void handleAutoFeederTank10ScheduleRemove() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    AutoFeeder* feeder = getFeederTank10();
    if (!feeder) {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
        return;
    }
    
    uint8_t index = server.arg("index").toInt();
    AutoFeederScheduler& scheduler = feeder->getScheduler();
    bool result = scheduler.removeSchedule(index);
    
    String json = "{\"success\":" + String(result ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

void handleAutoFeederTank10ScheduleGet() {
    AutoFeeder* feeder = getFeederTank10();
    if (feeder) {
        String json = getScheduleJSON(feeder->getScheduler());
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}

// API для кормушки Tank20
void handleAutoFeederTank20Activate() {
    AutoFeeder* feeder = getFeederTank20();
    if (feeder) {
        bool result = feeder->activateManual();
        String json = "{\"success\":" + String(result ? "true" : "false") + "}";
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}

void handleAutoFeederTank20Status() {
    AutoFeeder* feeder = getFeederTank20();
    if (feeder) {
        String json = "{";
        json += "\"relayOn\":" + String(feeder->isRelayOn() ? "true" : "false") + ",";
        json += "\"type\":\"Tank20\"";
        json += "}";
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}

void handleAutoFeederTank20Logs() {
    AutoFeeder* feeder = getFeederTank20();
    if (feeder) {
        String json = getFeederLogsJSON(feeder->getLogger());
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}

void handleAutoFeederTank20ScheduleAdd() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    AutoFeeder* feeder = getFeederTank20();
    if (!feeder) {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
        return;
    }
    
    uint8_t hour = server.arg("hour").toInt();
    uint8_t minute = server.arg("minute").toInt();
    uint8_t dayOfWeek = server.arg("dayOfWeek").toInt();
    uint8_t repeatCount = server.arg("repeatCount").toInt();
    if (repeatCount == 0) repeatCount = 1;
    
    AutoFeederScheduler& scheduler = feeder->getScheduler();
    bool result = scheduler.addSchedule(hour, minute, dayOfWeek, repeatCount);
    
    String json = "{\"success\":" + String(result ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

void handleAutoFeederTank20ScheduleRemove() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }
    
    AutoFeeder* feeder = getFeederTank20();
    if (!feeder) {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
        return;
    }
    
    uint8_t index = server.arg("index").toInt();
    AutoFeederScheduler& scheduler = feeder->getScheduler();
    bool result = scheduler.removeSchedule(index);
    
    String json = "{\"success\":" + String(result ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

void handleAutoFeederTank20ScheduleGet() {
    AutoFeeder* feeder = getFeederTank20();
    if (feeder) {
        String json = getScheduleJSON(feeder->getScheduler());
        server.send(200, "application/json", json);
    } else {
        server.send(500, "application/json", "{\"error\":\"Feeder not initialized\"}");
    }
}
