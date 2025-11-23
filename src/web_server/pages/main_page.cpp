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
    html += "<p class=\"mb-1\"><i class=\"bi bi-droplet\"></i> " + String(TANK_LRG_NAME) + ": <span class=\"value\">" + String(getLrgTemperature(), 1) + "</span><span class=\"unit\">°C</span></p>\n";
    html += "<p class=\"mb-0\"><i class=\"bi bi-droplet\"></i> " + String(TANK_SML_NAME) + ": <span class=\"value\">" + String(getSmlTemperature(), 1) + "</span><span class=\"unit\">°C</span></p>\n";
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

    // Управление светом аквариума S
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-power\"></i> Управление светом " + String(TANK_SML_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getLightTankSmlState() ? "bg-success" : "bg-danger") + "\">" + String(getLightTankSmlState() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isLightTankSmlManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isLightTankSmlManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggleTankSml = getLightTankSmlLastToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggleTankSml.day()) + "." + String(lastToggleTankSml.month()) + "." + String(lastToggleTankSml.year()) + " " + String(lastToggleTankSml.hour()) + ":" + String(lastToggleTankSml.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getLightTankSmlState() ? "<a class=\"btn btn-off\" href=\"/relay_tank10/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/relay_tank10/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isLightTankSmlManualMode() ? "<a class=\"btn btn-auto\" href=\"/relay_tank10/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/relay_tank10/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "</div></div></div></div>\n";

    // Управление UV лампой аквариума S
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Управление UV лампой " + String(TANK_SML_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getUvLampTankSmlState() ? "bg-success" : "bg-danger") + "\">" + String(getUvLampTankSmlState() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isUvLampTankSmlManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isUvLampTankSmlManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggleUvTankSml = getUvLampTankSmlLastToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggleUvTankSml.day()) + "." + String(lastToggleUvTankSml.month()) + "." + String(lastToggleUvTankSml.year()) + " " + String(lastToggleUvTankSml.hour()) + ":" + String(lastToggleUvTankSml.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getUvLampTankSmlState() ? "<a class=\"btn btn-off\" href=\"/uv_lamp_tank10/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/uv_lamp_tank10/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isUvLampTankSmlManualMode() ? "<a class=\"btn btn-auto\" href=\"/uv_lamp_tank10/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/uv_lamp_tank10/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "</div></div></div></div>\n";

    // Управление светом аквариума L
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-power\"></i> Управление светом " + String(TANK_LRG_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getLightTankLrgState() ? "bg-success" : "bg-danger") + "\">" + String(getLightTankLrgState() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isLightTankLrgManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isLightTankLrgManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggle = getLightTankLrgLastToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggle.day()) + "." + String(lastToggle.month()) + "." + String(lastToggle.year()) + " " + String(lastToggle.hour()) + ":" + String(lastToggle.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getLightTankLrgState() ? "<a class=\"btn btn-off\" href=\"/relay/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/relay/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isLightTankLrgManualMode() ? "<a class=\"btn btn-auto\" href=\"/relay/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/relay/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "</div></div></div></div>\n";

    // Управление UV лампой аквариума L
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Управление UV лампой " + String(TANK_LRG_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getUvLampTankLrgState() ? "bg-success" : "bg-danger") + "\">" + String(getUvLampTankLrgState() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isUvLampTankLrgManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isUvLampTankLrgManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggleUvTankLrg = getUvLampTankLrgLastToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggleUvTankLrg.day()) + "." + String(lastToggleUvTankLrg.month()) + "." + String(lastToggleUvTankLrg.year()) + " " + String(lastToggleUvTankLrg.hour()) + ":" + String(lastToggleUvTankLrg.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getUvLampTankLrgState() ? "<a class=\"btn btn-off\" href=\"/uv_lamp_tank20/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/uv_lamp_tank20/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isUvLampTankLrgManualMode() ? "<a class=\"btn btn-auto\" href=\"/uv_lamp_tank20/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/uv_lamp_tank20/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
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
    
    html += "<p class=\"mb-2\">" + String(TANK_SML_NAME) + ": <span class=\"badge " + String(tank10Active ? "bg-success" : "bg-secondary") + "\">" + String(tank10Active ? "Активна" : "Неактивна") + "</span></p>\n";
    html += "<p class=\"mb-2\">" + String(TANK_LRG_NAME) + ": <span class=\"badge " + String(tank20Active ? "bg-success" : "bg-secondary") + "\">" + String(tank20Active ? "Активна" : "Неактивна") + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += "<a class=\"btn btn-outline-primary\" href=\"/autofeeder\"><i class=\"bi bi-gear\"></i> Настройки кормушек</a>\n";
    html += "</div></div></div></div>\n";

    html += "</div>\n"; // закрываем row

    html += "<script>setInterval(() => location.reload(), 10000);</script>\n";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

