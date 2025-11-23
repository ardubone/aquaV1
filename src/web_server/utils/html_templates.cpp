// html_templates.cpp
#include "html_templates.h"
#include "../../../include/config.h"

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

String generateNavMenu(const String& activePage) {
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
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "stream" ? activeClass : emptyClass) + "\" href=\"/stream\"><i class=\"bi bi-camera-video\"></i> Трансляция</a></li>\n";
    html += "<li class=\"nav-item\"><a class=\"nav-link" + (activePage == "debug" ? activeClass : emptyClass) + "\" href=\"/debug\"><i class=\"bi bi-tools\"></i> Дебаг</a></li>\n";
    
    html += "</ul></div></div></nav>\n";
    return html;
}

