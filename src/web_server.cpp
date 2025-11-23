// web_server.cpp
#include "web_server.h"
#include <RTClib.h>
#include "pcf8574_manager.h"

// Страницы
#include "web_server/pages/main_page.h"
#include "web_server/pages/logs_page.h"
#include "web_server/pages/graph_page.h"
#include "web_server/pages/time_page.h"
#include "web_server/pages/wifi_page.h"
#include "web_server/pages/relay_time_page.h"
#include "web_server/pages/autofeeder_page.h"
#include "web_server/pages/stream_page.h"

// API
#include "web_server/api/relay_api.h"
#include "web_server/api/autofeeder_api.h"
#include "web_server/api/uv_lamp_api.h"
#include "web_server/api/logger_api.h"

// Дебаг страница (всегда доступна)
#include "web_server/pages/debug_page.h"
#include "web_server/api/debug_api.h"

// Глобальные объекты
WebServer server(80);
extern RTC_DS1307 rtc;
extern PCF8574Manager pcfManager;

void setupWebServer()
{
    server.on("/", handleMainPage);
    server.on("/logs", handleLogsPage);
    server.on("/graph", []() {
        handleGraphPage("temp", "Температура, °C", "#FF9900");
    });
    server.on("/graph/temp", []() {
        handleGraphPage("temp", "Температура, °C", "#FF9900");
    });
    server.on("/graph/tank20", []() {
        handleGraphPage("tank20", "Аквариум20, °C", "#FF9900");
    });
    server.on("/graph/tank10", []() {
        handleGraphPage("tank10", "Аквариум10, °C", "#FF9900");
    });
    server.on("/graph/room", []() {
        handleGraphPage("room", "Комната, °C", "#3399FF");
    });
    server.on("/graph/humid", []() {
        handleGraphPage("humid", "Влажность, %", "#3399FF");
    });
    server.on("/graph/press", []() {
        handleGraphPage("press", "Давление, мм рт.ст.", "#33CC33");
    });
    server.on("/data", handleGraphData);
    
    // API для критических логов
    server.on("/api/critical", handleCriticalLogsApi);
    
    // Обработчики управления реле Tank20
    server.on("/relay/on", handleRelayOn);
    server.on("/relay/off", handleRelayOff);
    server.on("/relay/auto/on", handleAutoOn);
    server.on("/relay/auto/off", handleAutoOff);
    
    // Обработчики управления реле Tank10
    server.on("/relay_tank10/on", handleRelayTank10On);
    server.on("/relay_tank10/off", handleRelayTank10Off);
    server.on("/relay_tank10/auto/on", handleAutoTank10On);
    server.on("/relay_tank10/auto/off", handleAutoTank10Off);
    
    server.on("/settime", HTTP_GET, handleSetTimePage);
    server.on("/settime", HTTP_POST, handleSetTime);
    server.on("/wifi", handleWiFiStatusPage);
    
    // Настройка времени реле
    server.on("/setrelaytime", HTTP_GET, handleSetRelayTimePage);
    server.on("/setrelaytime", HTTP_POST, handleSetRelayTime);
    server.on("/setrelaytime_tank10", HTTP_POST, handleSetRelayTank10Time);
    server.on("/setrelaytime_uv_lamp_tank10", HTTP_POST, handleSetUvLampTank10Time);
    server.on("/setrelaytime_uv_lamp_tank20", HTTP_POST, handleSetUvLampTank20Time);
    
    // Управление автокормушками
    server.on("/autofeeder", handleAutoFeederPage);
    
    // API для кормушки Tank10
    server.on("/autofeeder/tank10/activate", handleAutoFeederTank10Activate);
    server.on("/autofeeder/tank10/status", handleAutoFeederTank10Status);
    server.on("/autofeeder/tank10/logs", handleAutoFeederTank10Logs);
    server.on("/autofeeder/tank10/schedule/add", HTTP_POST, handleAutoFeederTank10ScheduleAdd);
    server.on("/autofeeder/tank10/schedule/remove", HTTP_POST, handleAutoFeederTank10ScheduleRemove);
    server.on("/autofeeder/tank10/schedule/get", handleAutoFeederTank10ScheduleGet);
    
    // API для кормушки Tank20
    server.on("/autofeeder/tank20/activate", handleAutoFeederTank20Activate);
    server.on("/autofeeder/tank20/status", handleAutoFeederTank20Status);
    server.on("/autofeeder/tank20/logs", handleAutoFeederTank20Logs);
    server.on("/autofeeder/tank20/schedule/add", HTTP_POST, handleAutoFeederTank20ScheduleAdd);
    server.on("/autofeeder/tank20/schedule/remove", HTTP_POST, handleAutoFeederTank20ScheduleRemove);
    server.on("/autofeeder/tank20/schedule/get", handleAutoFeederTank20ScheduleGet);
    
    
    // Обработчики управления UV лампой Tank10
    server.on("/uv_lamp_tank10/on", handleUvLampTank10On);
    server.on("/uv_lamp_tank10/off", handleUvLampTank10Off);
    server.on("/uv_lamp_tank10/auto/on", handleUvLampTank10AutoOn);
    server.on("/uv_lamp_tank10/auto/off", handleUvLampTank10AutoOff);
    
    // Обработчики управления UV лампой Tank20
    server.on("/uv_lamp_tank20/on", handleUvLampTank20On);
    server.on("/uv_lamp_tank20/off", handleUvLampTank20Off);
    server.on("/uv_lamp_tank20/auto/on", handleUvLampTank20AutoOn);
    server.on("/uv_lamp_tank20/auto/off", handleUvLampTank20AutoOff);
    
    // Трансляция с камеры
    server.on("/stream", handleStreamPage);
    server.on("/stream/frame", handleStreamFrame);
    
    // Дебаг страница (всегда доступна)
    server.on("/debug", handleDebugPage);
    server.on("/debug/pcf8574/status", handleDebugPcf8574Status);
    server.on("/debug/pcf8574/setpin", HTTP_POST, handleDebugPcf8574SetPin);
    server.on("/debug/temperature/status", handleDebugTemperatureStatus);
    server.on("/debug/temperature/setaddress", HTTP_POST, handleDebugTemperatureSetAddress);
    
    server.begin();
}

void handleWebRequests()
{
    server.handleClient();
}
