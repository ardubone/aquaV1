#include "stream_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"

extern WebServer server;

void handleStreamPage() {
    String html = htmlHeader("Трансляция с камеры");
    html += generateNavMenu("stream");
    
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-camera-video\"></i> Онлайн трансляция</div>\n";
    html += "<div class=\"card-body\">\n";
    
    html += "<div class=\"alert alert-info\">\n";
    html += "<i class=\"bi bi-info-circle\"></i> Функция трансляции с камеры еще не реализована.\n";
    html += "</div>\n";
    
    html += "</div></div>\n";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleStreamFrame() {
    server.send(503, "text/plain", "Camera not available - feature not implemented");
}
