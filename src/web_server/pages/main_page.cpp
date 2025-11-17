// main_page.cpp
#include "main_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/relay.h"
#include "../../../include/sensors.h"
#include "../../../include/temperature.h"
#include "../../../include/autofeeder.h"
#include "../../../include/config.h"
#include <RTClib.h>
#include <WiFi.h>

extern WebServer server;
extern RTC_DS1307 rtc;

#ifdef DEBUG_MODE
#include "../../debug_mocks.h"
#endif

void handleMainPage()
{
    String html = htmlHeader("Главная страница");
    html += generateNavMenu();
    
    // Основная информация
    html += "<div class=\"row\">\n";
    
    // Дата и время
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-clock\"></i> Дата и время</div>\n";
    html += "<div class=\"card-body\">\n";
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    html += "<p class=\"mb-1\"><i class=\"bi bi-calendar\"></i> " + String(now.day()) + "." + String(now.month()) + "." + String(now.year()) + "</p>\n";
    html += "<p class=\"mb-0\"><i class=\"bi bi-clock\"></i> " + String(now.hour()) + ":" + String(now.minute()) + "</p>\n";
    html += "</div></div></div>\n";

    // Сеть
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-wifi\"></i> Сеть</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-0\"><i class=\"bi bi-hdd-network\"></i> " + WiFi.localIP().toString() + "</p>\n";
    html += "</div></div></div>\n";

    // Температуры аквариумов
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-thermometer-half\"></i> Аквариумы</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-1\"><i class=\"bi bi-droplet\"></i> Аквариум20: <span class=\"value\">" + String(getTank20Temperature(), 1) + "</span><span class=\"unit\">°C</span></p>\n";
    html += "<p class=\"mb-0\"><i class=\"bi bi-droplet\"></i> Аквариум10: <span class=\"value\">" + String(getTank10Temperature(), 1) + "</span><span class=\"unit\">°C</span></p>\n";
    html += "</div></div></div>\n";

    // Данные комнаты
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-house\"></i> Комната</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-1\"><i class=\"bi bi-thermometer-half\"></i> Температура: <span class=\"value\">" + String(getRoomTemp(), 1) + "</span><span class=\"unit\">°C</span></p>\n";
    html += "<p class=\"mb-1\"><i class=\"bi bi-moisture\"></i> Влажность: <span class=\"value\">" + String(getRoomHumidity(), 0) + "</span><span class=\"unit\">%</span></p>\n";
    html += "<p class=\"mb-0\"><i class=\"bi bi-speedometer2\"></i> Давление: <span class=\"value\">" + String(getRoomPressure(), 1) + "</span><span class=\"unit\">мм рт.ст.</span></p>\n";
    html += "</div></div></div>\n";

    // Управление реле Tank10
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-power\"></i> Управление реле Tank10</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getRelayTank10State() ? "bg-success" : "bg-danger") + "\">" + String(getRelayTank10State() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isRelayTank10ManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isRelayTank10ManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggleTank10 = getRelayTank10LastToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggleTank10.day()) + "." + String(lastToggleTank10.month()) + "." + String(lastToggleTank10.year()) + " " + String(lastToggleTank10.hour()) + ":" + String(lastToggleTank10.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getRelayTank10State() ? "<a class=\"btn btn-off\" href=\"/relay_tank10/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/relay_tank10/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isRelayTank10ManualMode() ? "<a class=\"btn btn-auto\" href=\"/relay_tank10/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/relay_tank10/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "</div></div></div></div>\n";

    // Управление UV лампой Tank10
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Управление UV лампой Tank10</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getUvLampTank10State() ? "bg-success" : "bg-danger") + "\">" + String(getUvLampTank10State() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isUvLampTank10ManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isUvLampTank10ManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggleUvTank10 = getUvLampTank10LastToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggleUvTank10.day()) + "." + String(lastToggleUvTank10.month()) + "." + String(lastToggleUvTank10.year()) + " " + String(lastToggleUvTank10.hour()) + ":" + String(lastToggleUvTank10.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getUvLampTank10State() ? "<a class=\"btn btn-off\" href=\"/uv_lamp_tank10/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/uv_lamp_tank10/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isUvLampTank10ManualMode() ? "<a class=\"btn btn-auto\" href=\"/uv_lamp_tank10/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/uv_lamp_tank10/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "<a class=\"btn btn-outline-primary\" href=\"/setrelaytime\"><i class=\"bi bi-clock\"></i> Настроить время</a>\n";
    html += "</div></div></div></div>\n";

    // Управление реле Tank20
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-power\"></i> Управление реле Tank20</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getRelayState() ? "bg-success" : "bg-danger") + "\">" + String(getRelayState() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isRelayManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isRelayManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggle = getLastRelayToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggle.day()) + "." + String(lastToggle.month()) + "." + String(lastToggle.year()) + " " + String(lastToggle.hour()) + ":" + String(lastToggle.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getRelayState() ? "<a class=\"btn btn-off\" href=\"/relay/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/relay/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isRelayManualMode() ? "<a class=\"btn btn-auto\" href=\"/relay/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/relay/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "</div></div></div></div>\n";

    // Управление UV лампой Tank20
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Управление UV лампой Tank20</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getUvLampTank20State() ? "bg-success" : "bg-danger") + "\">" + String(getUvLampTank20State() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isUvLampTank20ManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isUvLampTank20ManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggleUvTank20 = getUvLampTank20LastToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggleUvTank20.day()) + "." + String(lastToggleUvTank20.month()) + "." + String(lastToggleUvTank20.year()) + " " + String(lastToggleUvTank20.hour()) + ":" + String(lastToggleUvTank20.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getUvLampTank20State() ? "<a class=\"btn btn-off\" href=\"/uv_lamp_tank20/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/uv_lamp_tank20/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isUvLampTank20ManualMode() ? "<a class=\"btn btn-auto\" href=\"/uv_lamp_tank20/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/uv_lamp_tank20/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "<a class=\"btn btn-outline-primary\" href=\"/setrelaytime\"><i class=\"bi bi-clock\"></i> Настроить время</a>\n";
    html += "</div></div></div></div>\n";

    // Автокормушки
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-egg\"></i> Автокормушки</div>\n";
    html += "<div class=\"card-body\">\n";
    
    // Получаем состояние кормушек
    AutoFeeder* feeder10 = getFeederTank10();
    AutoFeeder* feeder20 = getFeederTank20();
    
    bool tank10Active = feeder10 ? feeder10->isRelayOn() : false;
    bool tank20Active = feeder20 ? feeder20->isRelayOn() : false;
    
    html += "<p class=\"mb-2\">Tank10: <span class=\"badge " + String(tank10Active ? "bg-success" : "bg-secondary") + "\">" + String(tank10Active ? "Активна" : "Неактивна") + "</span></p>\n";
    html += "<p class=\"mb-2\">Tank20: <span class=\"badge " + String(tank20Active ? "bg-success" : "bg-secondary") + "\">" + String(tank20Active ? "Активна" : "Неактивна") + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += "<a class=\"btn btn-outline-primary\" href=\"/autofeeder\"><i class=\"bi bi-gear\"></i> Настройки кормушек</a>\n";
    html += "</div></div></div></div>\n";

    html += "</div>\n"; // закрываем row

    html += "<script>setInterval(() => location.reload(), 10000);</script>\n";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

