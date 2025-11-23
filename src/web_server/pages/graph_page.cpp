// graph_page.cpp
#include "graph_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/logger.h"
#include "../../../include/config.h"

extern WebServer server;

void handleGraphPage(String type, const String &label, const String &color)
{
    String html = htmlHeader("График " + label);
    html += generateNavMenu("graph");
    
    // Выбор датчика
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-graph-up\"></i> Выбор датчика</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<div class=\"btn-group\" role=\"group\">\n";
    html += "<a class=\"btn " + String(type == "tank20" || type == "tankLrg" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/tank20\">" + String(TANK_LRG_NAME) + "</a>\n";
    html += "<a class=\"btn " + String(type == "tank10" || type == "tankSml" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/tank10\">" + String(TANK_SML_NAME) + "</a>\n";
    html += "<a class=\"btn " + String(type == "room" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/room\">Комната</a>\n";
    html += "<a class=\"btn " + String(type == "humid" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/humid\">Влажность</a>\n";
    html += "<a class=\"btn " + String(type == "press" ? "btn-primary" : "btn-outline-primary") + "\" href=\"/graph/press\">Давление</a>\n";
    html += "</div></div></div>\n";

    // График
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-graph-up\"></i> " + label + "</div>\n";
    html += "<div class=\"card-body\">\n";
    
    // Проверяем, валиден ли тип графика
    bool isValidType = (type == "tank20" || type == "tank10" || type == "tankLrg" || type == "tankSml" || type == "room" || type == "humid" || type == "press");
    
    if (!isValidType) {
        // Показываем заглушку
        html += "<div class=\"text-center py-5\">\n";
        html += "<i class=\"bi bi-graph-up\" style=\"font-size: 4rem; color: #6c757d;\"></i>\n";
        html += "<p class=\"mt-3 text-muted\" style=\"font-size: 1.2rem;\">Выберите график для отображения</p>\n";
        html += "</div>\n";
    } else {
        html += "<canvas id='chart' style=\"width: 100%; height: 400px;\"></canvas>\n";
    }
    html += "</div></div>\n";

    // Скрипты (только если тип валиден)
    if (isValidType) {
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
    }

    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleGraphData()
{
    String type = server.arg("type");
    
    uint16_t totalLogs = 0;
    LogEntry* logs = getLogs(&totalLogs);
    
    int startIdx = max(0, (int)totalLogs - 60);
    int endIdx = (int)totalLogs;
    
    String json = "{ \"timestamps\": [";
    for (int i = startIdx; i < endIdx; i++)
    {
        DateTime dt = uint32ToDateTime(logs[i].timestamp);
        json += "\"" + dt.timestamp() + "\"";
        if (i < endIdx - 1)
            json += ",";
    }
    json += "], \"values\": [";
    for (int i = startIdx; i < endIdx; i++)
    {
        float v = 0;
        if (type == "tank20" || type == "tankLrg")
            v = int16ToFloat(logs[i].tankLrgTemp);
        else if (type == "tank10" || type == "tankSml")
            v = int16ToFloat(logs[i].tankSmlTemp);
        else if (type == "room")
            v = int16ToFloat(logs[i].roomTemp);
        else if (type == "humid")
            v = uint16ToFloat(logs[i].roomHumidity);
        else if (type == "press")
            v = uint16ToFloat(logs[i].roomPressure);
        json += String(v, 1);
        if (i < endIdx - 1)
            json += ",";
    }
    json += "] }";
    server.send(200, "application/json", json);
}

