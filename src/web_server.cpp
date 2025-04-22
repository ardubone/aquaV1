#include "web_server.h"
#include "logger.h"
#include "config.h"
#include "relay.h"
#include "sensors.h"
#include "temperature.h"
#include "autofeeder.h"
#include <RTClib.h>

WebServer server(80);

extern RTC_DS1307 rtc;

// Прототипы
String htmlHeader(const String &title);
String htmlFooter();

void handleMainPage();
void handleLogsPage();
void handleGraphPage(String type, const String &label, const String &color);
void handleGraphData();
void handleRelayOn();
void handleRelayOff();
void handleSetTimePage();
void handleSetTime();
void handleWiFiStatusPage();
void handleSetRelayTimePage();
void handleSetRelayTime();
void handleAutoFeederPage();
void handleAutoFeederActivate();
void handleAutoFeederStatus();

// Структура для времени реле
struct RelayTime {
    uint8_t onHour;
    uint8_t onMinute;
    uint8_t offHour;
    uint8_t offMinute;
};

RelayTime relayTime = {8, 0, 19, 0}; // Значения по умолчанию

String htmlHeader(const String &title)
{
    String html = "<!DOCTYPE html><html data-bs-theme=\"dark\"><head>\n";
    html += "<meta charset=\"UTF-8\">\n";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    html += "<title>" + title + "</title>\n";
    html += "<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n";
    html += "<link href=\"https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.0/font/bootstrap-icons.css\" rel=\"stylesheet\">\n";
    html += "<style>\n";
    html += "body { font-family: 'Segoe UI', system-ui, -apple-system, sans-serif; background-color: #212529; color: #f8f9fa; }\n";
    html += ".card { border-radius: 15px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); margin-bottom: 1rem; border: 1px solid #495057; background-color: #2c3034; }\n";
    html += ".card-header { border-radius: 15px 15px 0 0 !important; border-bottom: 1px solid #495057; background-color: #343a40; }\n";
    html += ".btn { border-radius: 10px; padding: 0.5rem 1rem; font-weight: 500; transition: all 0.2s ease-in-out; }\n";
    html += ".btn:hover { transform: translateY(-1px); box-shadow: 0 2px 4px rgba(0,0,0,0.2); }\n";
    html += ".btn-on { background-color: #28a745; color: white; border: 1px solid #28a745; }\n";
    html += ".btn-on:hover { background-color: #218838; border-color: #1e7e34; }\n";
    html += ".btn-off { background-color: #dc3545; color: white; border: 1px solid #dc3545; }\n";
    html += ".btn-off:hover { background-color: #c82333; border-color: #bd2130; }\n";
    html += ".btn-auto { background-color: #17a2b8; color: white; border: 1px solid #17a2b8; }\n";
    html += ".btn-auto:hover { background-color: #138496; border-color: #117a8b; }\n";
    html += ".btn-manual { background-color: #ffc107; color: black; border: 1px solid #ffc107; }\n";
    html += ".btn-manual:hover { background-color: #e0a800; border-color: #d39e00; }\n";
    html += ".btn-outline-primary { border: 1px solid #0d6efd; color: #0d6efd; }\n";
    html += ".btn-outline-primary:hover { background-color: #0d6efd; color: white; }\n";
    html += ".value { font-size: 1.5rem; font-weight: bold; color: #f8f9fa; }\n";
    html += ".unit { font-size: 0.9rem; color: #adb5bd; }\n";
    html += ".navbar { box-shadow: 0 2px 4px rgba(0,0,0,0.3); background-color: #343a40 !important; }\n";
    html += ".table { margin-bottom: 0; color: #f8f9fa; }\n";
    html += ".table th { border-top: none; background-color: #343a40; }\n";
    html += ".table td { border-color: #495057; }\n";
    html += "@media (max-width: 768px) { .card { margin-bottom: 0.5rem; } }\n";
    html += "</style>\n";
    html += "</head><body>\n";
    html += "<div class=\"container py-4\">\n";
    return html;
}

String htmlFooter()
{
    String html = "</div>\n";
    html += "<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js\"></script>\n";
    html += "</body></html>";
    return html;
}

void handleAutoOn()
{
    resetRelayOverride();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAutoOff()
{
    toggleRelay(rtc.now());
    server.sendHeader("Location", "/");
    server.send(303);
}

// Добавляю функцию генерации меню
String generateNavMenu(const String& activePage = "") {
    String html = "<nav class=\"navbar navbar-expand-lg navbar-dark rounded mb-4\">\n";
    html += "<div class=\"container-fluid\">\n";
    html += "<a class=\"navbar-brand\" href=\"/\"><i class=\"bi bi-house-door\"></i> Главная</a>\n";
    html += "<button class=\"navbar-toggler\" type=\"button\" data-bs-toggle=\"collapse\" data-bs-target=\"#navbarNav\">\n";
    html += "<span class=\"navbar-toggler-icon\"></span></button>\n";
    html += "<div class=\"collapse navbar-collapse\" id=\"navbarNav\">\n";
    html += "<ul class=\"navbar-nav\">\n";
    
    String activeClass = " active";
    String emptyClass = "";
    
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "logs" ? activeClass : emptyClass) + "\" href=\"/logs\"><i class=\"bi bi-journal-text\"></i> Логи</a></li>\n";
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "graph" ? activeClass : emptyClass) + "\" href=\"/graph\"><i class=\"bi bi-graph-up\"></i> Графики</a></li>\n";
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "settime" ? activeClass : emptyClass) + "\" href=\"/settime\"><i class=\"bi bi-clock\"></i> Время</a></li>\n";
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "setrelaytime" ? activeClass : emptyClass) + "\" href=\"/setrelaytime\"><i class=\"bi bi-power\"></i> Время реле</a></li>\n";
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "wifi" ? activeClass : emptyClass) + "\" href=\"/wifi\"><i class=\"bi bi-wifi\"></i> WiFi</a></li>\n";
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "autofeeder" ? activeClass : emptyClass) + "\" href=\"/autofeeder\"><i class=\"bi bi-egg\"></i> Кормушка</a></li>\n";
    
    html += "</ul></div></div></nav>\n";
    return html;
}

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
    html += "<p class=\"mb-1\"><i class=\"bi bi-calendar\"></i> " + String(rtc.now().day()) + "." + String(rtc.now().month()) + "." + String(rtc.now().year()) + "</p>\n";
    html += "<p class=\"mb-0\"><i class=\"bi bi-clock\"></i> " + String(rtc.now().hour()) + ":" + String(rtc.now().minute()) + "</p>\n";
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

    // Управление реле
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-power\"></i> Управление реле</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(getRelayState() ? "bg-success" : "bg-danger") + "\">" + String(getRelayState() ? "ВКЛ" : "ВЫКЛ") + "</span></p>\n";
    html += "<p class=\"mb-2\">Режим: <span class=\"badge " + String(isRelayManualMode() ? "bg-warning" : "bg-info") + "\">" + String(isRelayManualMode() ? "Ручной" : "Авто") + "</span></p>\n";
    DateTime lastToggle = getLastRelayToggleTime();
    html += "<p class=\"mb-2\">Последнее переключение: <span class=\"text-muted\">" + String(lastToggle.day()) + "." + String(lastToggle.month()) + "." + String(lastToggle.year()) + " " + String(lastToggle.hour()) + ":" + String(lastToggle.minute()) + "</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += getRelayState() ? "<a class=\"btn btn-off\" href=\"/relay/off\"><i class=\"bi bi-power\"></i> Выключить</a>" : "<a class=\"btn btn-on\" href=\"/relay/on\"><i class=\"bi bi-power\"></i> Включить</a>\n";
    html += isRelayManualMode() ? "<a class=\"btn btn-auto\" href=\"/relay/auto/on\"><i class=\"bi bi-arrow-repeat\"></i> Перейти в АВТО</a>" : "<a class=\"btn btn-manual\" href=\"/relay/auto/off\"><i class=\"bi bi-hand-index\"></i> Перейти в РУЧНОЙ</a>\n";
    html += "</div></div></div></div>\n";

    // Автокормушка 
    html += "<div class=\"col-md-6 col-lg-4 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-egg\"></i> Автокормушка</div>\n";
    html += "<div class=\"card-body\">\n";
    
    // Внешняя переменная из autofeeder.cpp
    extern Mosfet autoFeederMosfet;
    
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(autoFeederMosfet.isOn() ? "bg-success" : "bg-danger") + "\">" + String(autoFeederMosfet.isOn() ? "Активна" : "Неактивна") + "</span></p>\n";
    html += "<p class=\"mb-2\">Следующее кормление: <span class=\"text-muted\">9:00, 19:00</span></p>\n";
    html += "<div class=\"d-grid gap-2\">\n";
    html += "<a class=\"btn btn-primary\" href=\"/autofeeder/activate\"><i class=\"bi bi-play-circle\"></i> Покормить сейчас</a>\n";
    html += "<a class=\"btn btn-outline-primary\" href=\"/autofeeder\"><i class=\"bi bi-gear\"></i> Настройки</a>\n";
    html += "</div></div></div></div>\n";

    html += "</div>\n"; // закрываем row

    html += "<script>setInterval(() => location.reload(), 10000);</script>\n";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleLogsPage()
{
    String html = htmlHeader("Логи");
    html += generateNavMenu("logs");
    
    // Таблица логов
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header bg-primary text-white\"><i class=\"bi bi-table\"></i> История измерений</div>\n";
    html += "<div class=\"card-body p-0\">\n";
    html += "<div class=\"table-responsive\">\n";
    html += "<table class=\"table table-striped table-hover mb-0\">\n";
    html += "<thead class=\"table-light\">\n";
    html += "<tr>\n";
    html += "<th><i class=\"bi bi-clock\"></i> Время</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> Аквариум20</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> Аквариум10</th>\n";
    html += "<th><i class=\"bi bi-house\"></i> Комната</th>\n";
    html += "<th><i class=\"bi bi-moisture\"></i> Влажность</th>\n";
    html += "<th><i class=\"bi bi-speedometer2\"></i> Давление</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody>\n";

    for (int i = max(0, logCount - 60); i < logCount; i++)
    {
        const LogEntry &l = temperatureLogs[i];
        html += "<tr>\n";
        html += "<td>" + l.timestamp.timestamp() + "</td>\n";
        html += "<td>" + String(l.tank20Temp, 1) + " °C</td>\n";
        html += "<td>" + String(l.tank10Temp, 1) + " °C</td>\n";
        html += "<td>" + String(l.roomTemp, 1) + " °C</td>\n";
        html += "<td>" + String(l.roomHumidity, 0) + " %</td>\n";
        html += "<td>" + String(l.roomPressure, 1) + " мм рт.ст.</td>\n";
        html += "</tr>\n";
    }

    html += "</tbody>\n";
    html += "</table>\n";
    html += "</div>\n"; // table-responsive
    html += "</div>\n"; // card-body
    html += "</div>\n"; // card

    html += "<script>setInterval(() => location.reload(), 10000);</script>\n";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleGraphPage(String type, const String &label, const String &color)
{
    String html = htmlHeader("График " + label);
    html += generateNavMenu("graph");
    
    // Выбор датчика
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-graph-up\"></i> Выбор датчика</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<div class=\"btn-group\" role=\"group\">\n";
    html += "<a class=\"btn " + String(type == "tank20" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/tank20\">Аквариум20</a>\n";
    html += "<a class=\"btn " + String(type == "tank10" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/tank10\">Аквариум10</a>\n";
    html += "<a class=\"btn " + String(type == "room" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/room\">Комната</a>\n";
    html += "<a class=\"btn " + String(type == "humid" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/humid\">Влажность</a>\n";
    html += "<a class=\"btn " + String(type == "press" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/press\">Давление</a>\n";
    html += "</div></div></div>\n";

    // График
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-graph-up\"></i> " + label + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<canvas id='chart' style=\"width: 100%; height: 400px;\"></canvas>\n";
    html += "</div></div>\n";

    // Скрипты
    html += "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n";
    html += "<script>\n";
    html += "let chart;\n";
    html += "function loadData() {\n";
    html += "  fetch('/data?type=" + type + "')\n";
    html += "  .then(r => r.json())\n";
    html += "  .then(data => {\n";
    html += "    const ctx = document.getElementById('chart').getContext('2d');\n";
    html += "    if (!chart) {\n";
    html += "      chart = new Chart(ctx, {\n";
    html += "        type: 'line',\n";
    html += "        data: {\n";
    html += "          labels: data.timestamps,\n";
    html += "          datasets: [{\n";
    html += "            label: '" + label + "',\n";
    html += "            data: data.values,\n";
    html += "            borderColor: '" + color + "',\n";
    html += "            backgroundColor: '" + color + "20',\n";
    html += "            borderWidth: 2,\n";
    html += "            tension: 0.1,\n";
    html += "            fill: true\n";
    html += "          }]\n";
    html += "        },\n";
    html += "        options: {\n";
    html += "          responsive: true,\n";
    html += "          maintainAspectRatio: false,\n";
    html += "          plugins: {\n";
    html += "            legend: {\n";
    html += "              labels: {\n";
    html += "                color: '#f8f9fa',\n";
    html += "                font: {\n";
    html += "                  size: 14\n";
    html += "                }\n";
    html += "              }\n";
    html += "            }\n";
    html += "          },\n";
    html += "          scales: {\n";
    html += "            x: {\n";
    html += "              grid: {\n";
    html += "                color: 'rgba(255,255,255,0.1)'\n";
    html += "              },\n";
    html += "              ticks: {\n";
    html += "                color: '#adb5bd'\n";
    html += "              }\n";
    html += "            },\n";
    html += "            y: {\n";
    html += "              grid: {\n";
    html += "                color: 'rgba(255,255,255,0.1)'\n";
    html += "              },\n";
    html += "              ticks: {\n";
    html += "                color: '#adb5bd'\n";
    html += "              }\n";
    html += "            }\n";
    html += "          }\n";
    html += "        }\n";
    html += "      });\n";
    html += "    } else {\n";
    html += "      chart.data.labels = data.timestamps;\n";
    html += "      chart.data.datasets[0].data = data.values;\n";
    html += "      chart.update();\n";
    html += "    }\n";
    html += "  });\n";
    html += "}\n";
    html += "loadData();\n";
    html += "setInterval(loadData, 10000);\n";
    html += "</script>\n";

    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleGraphData()
{
    String type = server.arg("type");
    String json = "{ \"timestamps\": [";
    for (int i = max(0, logCount - 60); i < logCount; i++)
    {
        json += "\"" + temperatureLogs[i].timestamp.timestamp() + "\"";
        if (i < logCount - 1)
            json += ",";
    }
    json += "], \"values\": [";
    for (int i = max(0, logCount - 60); i < logCount; i++)
    {
        float v = 0;
        if (type == "tank20")
            v = temperatureLogs[i].tank20Temp;
        else if (type == "tank10")
            v = temperatureLogs[i].tank10Temp;
        else if (type == "room")
            v = temperatureLogs[i].roomTemp;
        else if (type == "humid")
            v = temperatureLogs[i].roomHumidity;
        else if (type == "press")
            v = temperatureLogs[i].roomPressure;
        json += String(v, 1);
        if (i < logCount - 1)
            json += ",";
    }
    json += "] }";
    server.send(200, "application/json", json);
}

void handleRelayOn()
{
    toggleRelay(rtc.now());
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRelayOff()
{
    if (getRelayState()) {
        toggleRelay(rtc.now());
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleSetTimePage()
{
    String html = htmlHeader("Установка времени");
    html += generateNavMenu("settime");
    
    // Форма установки времени
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-clock\"></i> Установка времени</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<form action=\"/settime\" method=\"POST\">\n";
    
    // Дата
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Дата</label>\n";
    html += "<div class=\"row g-2\">\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"day\" placeholder=\"День\" min=\"1\" max=\"31\" value=\"" + String(rtc.now().day()) + "\"></div>\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"month\" placeholder=\"Месяц\" min=\"1\" max=\"12\" value=\"" + String(rtc.now().month()) + "\"></div>\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"year\" placeholder=\"Год\" min=\"2000\" max=\"2099\" value=\"" + String(rtc.now().year()) + "\"></div>\n";
    html += "</div></div>\n";
    
    // Время
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время</label>\n";
    html += "<div class=\"row g-2\">\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"hour\" placeholder=\"Часы\" min=\"0\" max=\"23\" value=\"" + String(rtc.now().hour()) + "\"></div>\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"minute\" placeholder=\"Минуты\" min=\"0\" max=\"59\" value=\"" + String(rtc.now().minute()) + "\"></div>\n";
    html += "</div></div>\n";
    
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";

    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleSetTime()
{
    if (server.hasArg("day") && server.hasArg("month") && server.hasArg("year") && 
        server.hasArg("hour") && server.hasArg("minute"))
    {
        int day = server.arg("day").toInt();
        int month = server.arg("month").toInt();
        int year = server.arg("year").toInt();
        int hour = server.arg("hour").toInt();
        int minute = server.arg("minute").toInt();
        
        DateTime newTime(year, month, day, hour, minute, 0);
        rtc.adjust(newTime);
    }
    
    server.sendHeader("Location", "/settime");
    server.send(303);
}

void handleWiFiStatusPage()
{
    String html = htmlHeader("Статус WiFi");
    html += generateNavMenu("wifi");
    
    // Статус WiFi
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-wifi\"></i> Статус WiFi</div>\n";
    html += "<div class=\"card-body\">\n";
    
    if (WiFi.status() == WL_CONNECTED)
    {
        html += "<p class=\"mb-2\"><i class=\"bi bi-check-circle-fill text-success\"></i> Подключено</p>\n";
        html += "<p class=\"mb-2\"><i class=\"bi bi-router\"></i> SSID: " + WiFi.SSID() + "</p>\n";
        html += "<p class=\"mb-2\"><i class=\"bi bi-hdd-network\"></i> IP: " + WiFi.localIP().toString() + "</p>\n";
        html += "<p class=\"mb-2\"><i class=\"bi bi-signal\"></i> Сила сигнала: " + String(WiFi.RSSI()) + " dBm</p>\n";
    }
    else
    {
        html += "<p class=\"mb-2\"><i class=\"bi bi-x-circle-fill text-danger\"></i> Не подключено</p>\n";
    }
    
    html += "</div></div>\n";

    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleSetRelayTimePage() {
    String html = htmlHeader("Установка времени реле");
    html += generateNavMenu("setrelaytime");
    
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-clock\"></i> Установка времени реле</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<form action=\"/setrelaytime\" method=\"POST\">\n";
    
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getRelayOnHour()) + "\">\n";
    html += "</div>\n";
    
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getRelayOffHour()) + "\">\n";
    html += "</div>\n";
    
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";

    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleSetRelayTime() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        setRelayTimes(server.arg("onHour").toInt(), server.arg("offHour").toInt());
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}

void handleAutoFeederPage() {
    String html = htmlHeader("Автокормушка");
    html += generateNavMenu("autofeeder");
    
    // Карточка управления
    html += "<div class=\"row\">\n";
    html += "<div class=\"col-md-6 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-egg\"></i> Управление кормушкой</div>\n";
    html += "<div class=\"card-body\">\n";
    
    // Внешняя переменная из autofeeder.cpp
    extern Mosfet autoFeederMosfet;
    
    html += "<p class=\"mb-2\">Состояние: <span class=\"badge " + String(autoFeederMosfet.isOn() ? "bg-success" : "bg-danger") + "\">" + String(autoFeederMosfet.isOn() ? "Активна" : "Неактивна") + "</span></p>\n";
    
    html += "<div class=\"d-grid gap-2\">\n";
    html += "<a class=\"btn btn-primary\" href=\"/autofeeder/activate\"><i class=\"bi bi-play-circle\"></i> Активировать подачу корма</a>\n";
    html += "</div></div></div></div>\n";
    
    // Карточка расписания
    html += "<div class=\"col-md-6 mb-3\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-calendar-check\"></i> Расписание</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<ul class=\"list-group\">\n";
    html += "<li class=\"list-group-item d-flex justify-content-between align-items-center\">Понедельник <span>9:00, 19:00</span></li>\n";
    html += "<li class=\"list-group-item d-flex justify-content-between align-items-center\">Вторник <span>9:00, 19:00</span></li>\n";
    html += "<li class=\"list-group-item d-flex justify-content-between align-items-center\">Среда <span>9:00, 19:00</span></li>\n";
    html += "<li class=\"list-group-item d-flex justify-content-between align-items-center\">Четверг <span>9:00, 19:00</span></li>\n";
    html += "<li class=\"list-group-item d-flex justify-content-between align-items-center\">Пятница <span>9:00, 19:00</span></li>\n";
    html += "<li class=\"list-group-item d-flex justify-content-between align-items-center\">Воскресенье <span>9:00, 19:00</span></li>\n";
    html += "</ul>\n";
    html += "</div></div></div>\n";
    
    html += "</div>\n"; // закрываем row
    
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleAutoFeederActivate() {
    activateFeeder();
    server.sendHeader("Location", "/autofeeder");
    server.send(303);
}

void handleAutoFeederStatus() {
    extern Mosfet autoFeederMosfet;
    String json = "{\"active\": " + String(autoFeederMosfet.isOn() ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

void setupWebServer()
{
    server.on("/", handleMainPage);
    server.on("/logs", handleLogsPage);
    server.on("/graph", []() { handleGraphPage("tank20", "Аквариум20 Температура", "red"); });
    server.on("/graph/tank20", []() { handleGraphPage("tank20", "Аквариум20 Температура", "red"); });
    server.on("/graph/tank10", []() { handleGraphPage("tank10", "Аквариум10 Температура", "blue"); });
    server.on("/graph/room", []() { handleGraphPage("room", "Комната Температура", "lime"); });
    server.on("/graph/humid", []() { handleGraphPage("humid", "Влажность", "aqua"); });
    server.on("/graph/press", []() { handleGraphPage("press", "Давление", "orange"); });
    server.on("/data", handleGraphData);
    server.on("/relay/on", handleRelayOn);
    server.on("/relay/off", handleRelayOff);
    server.on("/relay/auto/on", handleAutoOn);
    server.on("/relay/auto/off", handleAutoOff);
    server.on("/settime", HTTP_GET, handleSetTimePage);
    server.on("/settime", HTTP_POST, handleSetTime);
    server.on("/setrelaytime", HTTP_GET, handleSetRelayTimePage);
    server.on("/setrelaytime", HTTP_POST, handleSetRelayTime);
    server.on("/wifi", handleWiFiStatusPage);
    server.on("/autofeeder", handleAutoFeederPage);
    server.on("/autofeeder/activate", handleAutoFeederActivate);
    server.on("/autofeeder/status", handleAutoFeederStatus);

    server.begin();
}

void handleWebRequests()
{
    server.handleClient();
}
