// time_page.cpp
#include "time_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/config.h"
#include <RTClib.h>

extern WebServer server;
extern RTC_DS1307 rtc;

#ifdef DEBUG_MODE
#include "../../debug_mocks.h"
#endif

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
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"day\" placeholder=\"День\" min=\"1\" max=\"31\" value=\"" + String(now.day()) + "\"></div>\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"month\" placeholder=\"Месяц\" min=\"1\" max=\"12\" value=\"" + String(now.month()) + "\"></div>\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"year\" placeholder=\"Год\" min=\"2000\" max=\"2099\" value=\"" + String(now.year()) + "\"></div>\n";
    html += "</div></div>\n";
    
    // Время
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Время</label>\n";
    html += "<div class=\"row g-2\">\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"hour\" placeholder=\"Часы\" min=\"0\" max=\"23\" value=\"" + String(now.hour()) + "\"></div>\n";
    html += "<div class=\"col\"><input type=\"number\" class=\"form-control\" name=\"minute\" placeholder=\"Минуты\" min=\"0\" max=\"59\" value=\"" + String(now.minute()) + "\"></div>\n";
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
#ifdef DEBUG_MODE
        if (isRtcInitialized) {
            rtc.adjust(newTime);
        }
#else
        rtc.adjust(newTime);
#endif
    }
    
    server.sendHeader("Location", "/settime");
    server.send(303);
}

