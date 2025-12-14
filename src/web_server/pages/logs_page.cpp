// logs_page.cpp
#include "logs_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/logger.h"
#include "../../../include/config.h"
#include <RTClib.h>

extern WebServer server;

String formatLogEntry(const LogEntry& log) {
    DateTime dt = uint32ToDateTime(log.timestamp);
    String html = "<tr>\n";
    html += "<td>" + dt.timestamp() + "</td>\n";
    html += "<td>" + String(int16ToFloat(log.tankLrgTemp), 1) + " °C</td>\n";
    html += "<td>" + String(int16ToFloat(log.tankSmlTemp), 1) + " °C</td>\n";
    html += "<td>" + String(int16ToFloat(log.roomTemp), 1) + " °C</td>\n";
    html += "<td>" + String(uint16ToFloat(log.roomHumidity), 0) + " %</td>\n";
    html += "<td>" + String(uint16ToFloat(log.roomPressure), 1) + " мм рт.ст.</td>\n";
    html += "<td><i class=\"bi bi-check-circle text-success\"></i> Норма</td>\n";
    html += "</tr>\n";
    return html;
}

String formatMinuteLogEntry(const MinuteLogEntry& log) {
    DateTime dt = uint32ToDateTime(log.timestamp);
    String html = "<tr>\n";
    html += "<td>" + dt.timestamp() + "</td>\n";
    html += "<td>" + String(int16ToFloat(log.tankLrgTemp), 1) + " °C</td>\n";
    html += "<td>" + String(int16ToFloat(log.tankSmlTemp), 1) + " °C</td>\n";
    html += "<td>" + String(int16ToFloat(log.roomTemp), 1) + " °C</td>\n";
    html += "<td>" + String(uint16ToFloat(log.roomHumidity), 0) + " %</td>\n";
    html += "<td>" + String(uint16ToFloat(log.roomPressure), 1) + " мм рт.ст.</td>\n";
    html += "<td><i class=\"bi bi-check-circle text-success\"></i> Норма</td>\n";
    html += "</tr>\n";
    return html;
}

String formatCriticalLogEntry(const CriticalLogEntry& log) {
    DateTime dt = uint32ToDateTime(log.timestamp);
    String paramName = "";
    float currentValue = 0;
    if (log.parameterType == 0) {
        paramName = String(TANK_LRG_NAME);
        currentValue = int16ToFloat(log.tankLrgTemp);
    } else if (log.parameterType == 1) {
        paramName = String(TANK_SML_NAME);
        currentValue = int16ToFloat(log.tankSmlTemp);
    } else if (log.parameterType == 2) {
        paramName = "Влажность";
        currentValue = uint16ToFloat(log.roomHumidity);
    }
    
    String html = "<tr class=\"table-danger\">\n";
    html += "<td>" + dt.timestamp() + "</td>\n";
    html += "<td>" + paramName + "</td>\n";
    html += "<td>" + String(currentValue, 2) + "</td>\n";
    html += "<td>" + String(int16ToFloat(log.previousAvg), 2) + "</td>\n";
    html += "<td>" + String(uint16ToFloat(log.changePercent), 2) + " %</td>\n";
    html += "<td><i class=\"bi bi-exclamation-triangle-fill text-danger\"></i> Критично</td>\n";
    html += "</tr>\n";
    return html;
}

void handleLogsPage()
{
    String html = htmlHeader("Логи");
    html += generateNavMenu("logs");
    
    // Вкладки
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\">\n";
    html += "<ul class=\"nav nav-tabs card-header-tabs\" role=\"tablist\">\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link active\" id=\"main-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#main\" type=\"button\" role=\"tab\">Основная (за сутки)</button>\n";
    html += "</li>\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link\" id=\"daily-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#daily\" type=\"button\" role=\"tab\">За сутки</button>\n";
    html += "</li>\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link\" id=\"three-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#three\" type=\"button\" role=\"tab\">За трое</button>\n";
    html += "</li>\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link\" id=\"critical-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#critical\" type=\"button\" role=\"tab\">Крит</button>\n";
    html += "</li>\n";
    html += "</ul>\n";
    html += "</div>\n"; // card-header
    
    html += "<div class=\"tab-content\" id=\"myTabContent\">\n";
    
    // Вкладка "Основная (за сутки)"
    html += "<div class=\"tab-pane fade show active\" id=\"main\" role=\"tabpanel\">\n";
    html += "<div class=\"card-body p-0\">\n";
    html += "<div class=\"table-responsive\">\n";
    html += "<table class=\"table table-striped table-hover mb-0\">\n";
    html += "<thead class=\"table-light\">\n";
    html += "<tr>\n";
    html += "<th><i class=\"bi bi-clock\"></i> Время</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> " + String(TANK_LRG_NAME) + "</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> " + String(TANK_SML_NAME) + "</th>\n";
    html += "<th><i class=\"bi bi-house\"></i> Комната</th>\n";
    html += "<th><i class=\"bi bi-moisture\"></i> Влажность</th>\n";
    html += "<th><i class=\"bi bi-speedometer2\"></i> Давление</th>\n";
    html += "<th><i class=\"bi bi-info-circle\"></i> Статус</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody id=\"main-tbody\">\n";
    html += "</tbody>\n";
    html += "</table>\n";
    html += "</div>\n"; // table-responsive
    html += "</div>\n"; // card-body
    html += "</div>\n"; // tab-pane
    
    // Вкладка "За сутки"
    html += "<div class=\"tab-pane fade\" id=\"daily\" role=\"tabpanel\">\n";
    html += "<div class=\"card-body p-0\">\n";
    html += "<div class=\"table-responsive\">\n";
    html += "<table class=\"table table-striped table-hover mb-0\">\n";
    html += "<thead class=\"table-light\">\n";
    html += "<tr>\n";
    html += "<th><i class=\"bi bi-clock\"></i> Время</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> " + String(TANK_LRG_NAME) + "</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> " + String(TANK_SML_NAME) + "</th>\n";
    html += "<th><i class=\"bi bi-house\"></i> Комната</th>\n";
    html += "<th><i class=\"bi bi-moisture\"></i> Влажность</th>\n";
    html += "<th><i class=\"bi bi-speedometer2\"></i> Давление</th>\n";
    html += "<th><i class=\"bi bi-info-circle\"></i> Статус</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody id=\"daily-tbody\">\n";
    html += "</tbody>\n";
    html += "</table>\n";
    html += "</div>\n"; // table-responsive
    html += "</div>\n"; // card-body
    html += "</div>\n"; // tab-pane
    
    // Вкладка "За трое"
    html += "<div class=\"tab-pane fade\" id=\"three\" role=\"tabpanel\">\n";
    html += "<div class=\"card-body p-0\">\n";
    html += "<div class=\"table-responsive\">\n";
    html += "<table class=\"table table-striped table-hover mb-0\">\n";
    html += "<thead class=\"table-light\">\n";
    html += "<tr>\n";
    html += "<th><i class=\"bi bi-clock\"></i> Время</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> " + String(TANK_LRG_NAME) + "</th>\n";
    html += "<th><i class=\"bi bi-droplet\"></i> " + String(TANK_SML_NAME) + "</th>\n";
    html += "<th><i class=\"bi bi-house\"></i> Комната</th>\n";
    html += "<th><i class=\"bi bi-moisture\"></i> Влажность</th>\n";
    html += "<th><i class=\"bi bi-speedometer2\"></i> Давление</th>\n";
    html += "<th><i class=\"bi bi-info-circle\"></i> Статус</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody id=\"three-tbody\">\n";
    html += "</tbody>\n";
    html += "</table>\n";
    html += "</div>\n"; // table-responsive
    html += "</div>\n"; // card-body
    html += "</div>\n"; // tab-pane
    
    // Вкладка "Крит"
    html += "<div class=\"tab-pane fade\" id=\"critical\" role=\"tabpanel\">\n";
    html += "<div class=\"card-body p-0\">\n";
    html += "<div class=\"table-responsive\">\n";
    html += "<table class=\"table table-striped table-hover mb-0\">\n";
    html += "<thead class=\"table-light\">\n";
    html += "<tr>\n";
    html += "<th><i class=\"bi bi-clock\"></i> Время</th>\n";
    html += "<th><i class=\"bi bi-tag\"></i> Параметр</th>\n";
    html += "<th><i class=\"bi bi-arrow-up\"></i> Текущее</th>\n";
    html += "<th><i class=\"bi bi-arrow-down\"></i> Предыдущее</th>\n";
    html += "<th><i class=\"bi bi-percent\"></i> Изменение</th>\n";
    html += "<th><i class=\"bi bi-info-circle\"></i> Статус</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody id=\"critical-tbody\">\n";
    html += "</tbody>\n";
    html += "</table>\n";
    html += "</div>\n"; // table-responsive
    html += "</div>\n"; // card-body
    html += "</div>\n"; // tab-pane
    
    html += "</div>\n"; // tab-content
    html += "</div>\n"; // card
    
    // JavaScript для загрузки данных
    html += "<script>\n";
    html += "function fmt(v, digits, suffix) {\n";
    html += "  if (v === null || v === undefined || Number.isNaN(v)) return '-';\n";
    html += "  return v.toFixed(digits) + (suffix || '');\n";
    html += "}\n";
    html += "function loadMainTab() {\n";
    html += "  // Загружаем часовые данные за 24 часа\n";
    html += "  fetch('/api/logs/hourly')\n";
    html += "    .then(r => r.json())\n";
    html += "    .then(data => {\n";
    html += "      let tbody = document.getElementById('main-tbody');\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      // Добавляем часовые данные\n";
    html += "      for (let i = 0; i < data.logs.length; i++) {\n";
    html += "        let log = data.logs[i];\n";
    html += "        let row = '<tr><td>' + log.timestamp + '</td>';\n";
    html += "        row += '<td>' + fmt(log.tankLrgTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.tankSmlTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomHumidity, 0, ' %') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomPressure, 1, ' мм рт.ст.') + '</td>';\n";
    html += "        row += '<td><i class=\"bi bi-check-circle text-success\"></i> Норма</td></tr>';\n";
    html += "        tbody.innerHTML += row;\n";
    html += "      }\n";
    html += "      // Загружаем 10-минутные интервалы текущего часа\n";
    html += "      fetch('/api/logs/minutes')\n";
    html += "        .then(r => r.json())\n";
    html += "        .then(minData => {\n";
    html += "          for (let i = 0; i < minData.logs.length; i++) {\n";
    html += "            let log = minData.logs[i];\n";
    html += "            let row = '<tr class=\"table-info\"><td>' + log.timestamp + ' (10 мин)</td>';\n";
    html += "            row += '<td>' + fmt(log.tankLrgTemp, 1, ' °C') + '</td>';\n";
    html += "            row += '<td>' + fmt(log.tankSmlTemp, 1, ' °C') + '</td>';\n";
    html += "            row += '<td>' + fmt(log.roomTemp, 1, ' °C') + '</td>';\n";
    html += "            row += '<td>' + fmt(log.roomHumidity, 0, ' %') + '</td>';\n";
    html += "            row += '<td>' + fmt(log.roomPressure, 1, ' мм рт.ст.') + '</td>';\n";
    html += "            row += '<td><i class=\"bi bi-check-circle text-success\"></i> Норма</td></tr>';\n";
    html += "            tbody.innerHTML += row;\n";
    html += "          }\n";
    html += "        });\n";
    html += "    });\n";
    html += "}\n";
    html += "function loadDailyTab() {\n";
    html += "  fetch('/api/logs/hourly')\n";
    html += "    .then(r => r.json())\n";
    html += "    .then(data => {\n";
    html += "      let tbody = document.getElementById('daily-tbody');\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      for (let i = 0; i < data.logs.length; i++) {\n";
    html += "        let log = data.logs[i];\n";
    html += "        let row = '<tr><td>' + log.timestamp + '</td>';\n";
    html += "        row += '<td>' + fmt(log.tankLrgTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.tankSmlTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomHumidity, 0, ' %') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomPressure, 1, ' мм рт.ст.') + '</td>';\n";
    html += "        row += '<td><i class=\"bi bi-check-circle text-success\"></i> Норма</td></tr>';\n";
    html += "        tbody.innerHTML += row;\n";
    html += "      }\n";
    html += "    });\n";
    html += "}\n";
    html += "function loadThreeTab() {\n";
    html += "  fetch('/api/logs/hourly/72')\n";
    html += "    .then(r => r.json())\n";
    html += "    .then(data => {\n";
    html += "      let tbody = document.getElementById('three-tbody');\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      for (let i = 0; i < data.logs.length; i++) {\n";
    html += "        let log = data.logs[i];\n";
    html += "        let row = '<tr><td>' + log.timestamp + '</td>';\n";
    html += "        row += '<td>' + fmt(log.tankLrgTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.tankSmlTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomTemp, 1, ' °C') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomHumidity, 0, ' %') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.roomPressure, 1, ' мм рт.ст.') + '</td>';\n";
    html += "        row += '<td><i class=\"bi bi-check-circle text-success\"></i> Норма</td></tr>';\n";
    html += "        tbody.innerHTML += row;\n";
    html += "      }\n";
    html += "    });\n";
    html += "}\n";
    html += "function loadCriticalTab() {\n";
    html += "  fetch('/api/critical')\n";
    html += "    .then(r => r.json())\n";
    html += "    .then(data => {\n";
    html += "      let tbody = document.getElementById('critical-tbody');\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      for (let i = 0; i < data.logs.length; i++) {\n";
    html += "        let log = data.logs[i];\n";
    html += "        let paramName = log.parameter === 'tankLrg' ? '" + String(TANK_LRG_NAME) + "' : (log.parameter === 'tankSml' ? '" + String(TANK_SML_NAME) + "' : 'Влажность');\n";
    html += "        let row = '<tr class=\"table-danger\"><td>' + log.timestamp + '</td>';\n";
    html += "        row += '<td>' + paramName + '</td>';\n";
    html += "        row += '<td>' + fmt(log.currentValue, 2, '') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.previousAvg, 2, '') + '</td>';\n";
    html += "        row += '<td>' + fmt(log.changePercent, 2, ' %') + '</td>';\n";
    html += "        row += '<td><i class=\"bi bi-exclamation-triangle-fill text-danger\"></i> Критично</td></tr>';\n";
    html += "        tbody.innerHTML += row;\n";
    html += "      }\n";
    html += "    });\n";
    html += "}\n";
    html += "// Загружаем данные при загрузке страницы\n";
    html += "loadMainTab();\n";
    html += "// Обновляем данные каждые 10 секунд\n";
    html += "setInterval(() => {\n";
    html += "  let activeTab = document.querySelector('.nav-link.active');\n";
    html += "  if (activeTab.id === 'main-tab') loadMainTab();\n";
    html += "  else if (activeTab.id === 'daily-tab') loadDailyTab();\n";
    html += "  else if (activeTab.id === 'three-tab') loadThreeTab();\n";
    html += "  else if (activeTab.id === 'critical-tab') loadCriticalTab();\n";
    html += "}, 10000);\n";
    html += "// Загружаем данные при переключении вкладок\n";
    html += "document.getElementById('main-tab').addEventListener('shown.bs.tab', loadMainTab);\n";
    html += "document.getElementById('daily-tab').addEventListener('shown.bs.tab', loadDailyTab);\n";
    html += "document.getElementById('three-tab').addEventListener('shown.bs.tab', loadThreeTab);\n";
    html += "document.getElementById('critical-tab').addEventListener('shown.bs.tab', loadCriticalTab);\n";
    html += "</script>\n";
    
    html += htmlFooter();
    server.send(200, "text/html", html);
}
