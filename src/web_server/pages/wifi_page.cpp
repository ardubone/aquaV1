// wifi_page.cpp
#include "wifi_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include <WiFi.h>

extern WebServer server;

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

