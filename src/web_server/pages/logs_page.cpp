// logs_page.cpp
#include "logs_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/logger.h"

extern WebServer server;

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
    html += "<th><i class=\"bi bi-info-circle\"></i> Статус</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody>\n";

    uint16_t totalLogs = 0;
    LogEntry* logs = getLogs(&totalLogs);
    
    int startIdx = max(0, (int)totalLogs - 60);
    for (int i = startIdx; i < (int)totalLogs; i++)
    {
        const LogEntry &l = logs[i];
        String rowClass = l.isCritical ? "table-danger" : "";
        String criticalIcon = l.isCritical ? "<i class=\"bi bi-exclamation-triangle-fill text-danger\"></i> Критично" : "<i class=\"bi bi-check-circle text-success\"></i> Норма";
        
        html += "<tr class=\"" + rowClass + "\">\n";
        html += "<td>" + l.timestamp.timestamp() + "</td>\n";
        html += "<td>" + String(l.tank20Temp, 1) + " °C</td>\n";
        html += "<td>" + String(l.tank10Temp, 1) + " °C</td>\n";
        html += "<td>" + String(l.roomTemp, 1) + " °C</td>\n";
        html += "<td>" + String(l.roomHumidity, 0) + " %</td>\n";
        html += "<td>" + String(l.roomPressure, 1) + " мм рт.ст.</td>\n";
        html += "<td>" + criticalIcon + "</td>\n";
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

