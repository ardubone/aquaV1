// relay_time_page.cpp
#include "relay_time_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/relay.h"

extern WebServer server;

void handleSetRelayTimePage() {
    String html = htmlHeader("Настройка времени реле");
    html += generateNavMenu("setrelaytime");
    
    // Карточка для Tank20
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\">Настройка времени работы реле Tank20</div>\n";
    html += "<div class=\"card-body\">\n";
    
    html += "<form action=\"/setrelaytime\" method=\"post\">\n";
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

    // Карточка для Tank10
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\">Настройка времени работы реле Tank10</div>\n";
    html += "<div class=\"card-body\">\n";
    
    html += "<form action=\"/setrelaytime_tank10\" method=\"post\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getRelayTank10OnHour()) + "\">\n";
    html += "</div>\n";
    
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getRelayTank10OffHour()) + "\">\n";
    html += "</div>\n";
    
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";

    // Карточка для UV лампы Tank10
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Настройка времени работы UV лампы Tank10</div>\n";
    html += "<div class=\"card-body\">\n";
    
    html += "<form action=\"/setrelaytime_uv_lamp_tank10\" method=\"post\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTank10OnHour()) + "\">\n";
    html += "</div>\n";
    
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTank10OffHour()) + "\">\n";
    html += "</div>\n";
    
    html += "<button type=\"submit\" class=\"btn btn-primary\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "</form></div></div>\n";

    // Карточка для UV лампы Tank20
    html += "<div class=\"card mb-4\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-lightbulb\"></i> Настройка времени работы UV лампы Tank20</div>\n";
    html += "<div class=\"card-body\">\n";
    
    html += "<form action=\"/setrelaytime_uv_lamp_tank20\" method=\"post\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время включения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"onHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTank20OnHour()) + "\">\n";
    html += "</div>\n";
    
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время выключения</label>\n";
    html += "<input type=\"number\" class=\"form-control\" name=\"offHour\" placeholder=\"Час\" min=\"0\" max=\"23\" value=\"" + String(getUvLampTank20OffHour()) + "\">\n";
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

void handleSetRelayTank10Time() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        setRelayTank10Times(server.arg("onHour").toInt(), server.arg("offHour").toInt());
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}

void handleSetUvLampTank10Time() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        uint8_t onHour = server.arg("onHour").toInt();
        uint8_t offHour = server.arg("offHour").toInt();
        setUvLampTank10Times(onHour, offHour);
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}

void handleSetUvLampTank20Time() {
    if (server.hasArg("onHour") && server.hasArg("offHour")) {
        uint8_t onHour = server.arg("onHour").toInt();
        uint8_t offHour = server.arg("offHour").toInt();
        setUvLampTank20Times(onHour, offHour);
    }
    
    server.sendHeader("Location", "/setrelaytime");
    server.send(303);
}

