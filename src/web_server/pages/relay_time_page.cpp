// relay_time_page.cpp
#include "relay_time_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/relay.h"
#include "../../../include/config.h"

extern WebServer server;

void handleSetRelayTimePage() {
    String html = htmlHeader("Настройка времени");
    html += generateNavMenu("setrelaytime");
    
    // Табы для переключения между блоками
    html += "<ul class=\"nav nav-tabs mb-3\" id=\"timeTabs\" role=\"tablist\">\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link active\" id=\"light-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#light-pane\" type=\"button\" role=\"tab\" aria-controls=\"light-pane\" aria-selected=\"true\">Свет</button>\n";
    html += "</li>\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link\" id=\"uv-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#uv-pane\" type=\"button\" role=\"tab\" aria-controls=\"uv-pane\" aria-selected=\"false\">УФ лампа</button>\n";
    html += "</li>\n";
    html += "</ul>\n";
    
    html += "<div class=\"tab-content\" id=\"timeTabsContent\">\n";
    
    // Блок Свет (по умолчанию открыт)
    html += "<div class=\"tab-pane fade show active\" id=\"light-pane\" role=\"tabpanel\" aria-labelledby=\"light-tab\">\n";
    
    // Свет аквариума S
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\">Настройка времени работы света " + String(TANK_SML_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<form action=\"/setrelaytime_tank10\" method=\"post\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getLightTankSmlOnHour()) + "\">\n";
    html += "</div>\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getLightTankSmlOffHour()) + "\">\n";
    html += "</div>\n";
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";
    
    // Свет аквариума L
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\">Настройка времени работы света " + String(TANK_LRG_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<form action=\"/setrelaytime\" method=\"post\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getLightTankLrgOnHour()) + "\">\n";
    html += "</div>\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getLightTankLrgOffHour()) + "\">\n";
    html += "</div>\n";
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";
    
    html += "</div>\n"; // закрываем tab-pane Свет
    
    // Блок УФ лампы
    html += "<div class=\"tab-pane fade\" id=\"uv-pane\" role=\"tabpanel\" aria-labelledby=\"uv-tab\">\n";
    
    // UV лампа аквариума S
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Настройка времени работы UV лампы " + String(TANK_SML_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<form action=\"/setrelaytime_uv_lamp_tank10\" method=\"post\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTankSmlOnHour()) + "\">\n";
    html += "</div>\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTankSmlOffHour()) + "\">\n";
    html += "</div>\n";
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";
    
    // UV лампа аквариума L
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Настройка времени работы UV лампы " + String(TANK_LRG_NAME) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<form action=\"/setrelaytime_uv_lamp_tank20\" method=\"post\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTankLrgOnHour()) + "\">\n";
    html += "</div>\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTankLrgOffHour()) + "\">\n";
    html += "</div>\n";
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";
    
    html += "</div>\n"; // закрываем tab-pane УФ лампы
    html += "</div>\n"; // закрываем tab-content
    
    
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleSetRelayTime() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        setLightTankLrgTimes(server.arg("onHour").toInt(), server.arg("offHour").toInt());
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}

void handleSetRelayTank10Time() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        setLightTankSmlTimes(server.arg("onHour").toInt(), server.arg("offHour").toInt());
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}

void handleSetUvLampTank10Time() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        uint8_t onHour = server.arg("onHour").toInt();
        uint8_t offHour = server.arg("offHour").toInt();
        setUvLampTankSmlTimes(onHour, offHour);
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}

void handleSetUvLampTank20Time() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        uint8_t onHour = server.arg("onHour").toInt();
        uint8_t offHour = server.arg("offHour").toInt();
        setUvLampTankLrgTimes(onHour, offHour);
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}
