#include "web_server.h"
#include "display.h"
#include "logger.h"
#include "config.h"
#include <RTClib.h>

WebServer server(80);

extern bool relayState;
extern DateTime lastRelayToggleTime;
extern RTC_DS1307 rtc;
extern bool relayManualOverride;

// Прототипы
String htmlHeader(const String &title);
String htmlFooter();

void handleMainPage();
void handleLogsPage();
void handleGraphPage(String type, const String &label, const String &color);
void handleGraphData();
void handleRelayOn();
void handleRelayOff();

String htmlHeader(const String &title)
{
    String html = "<!DOCTYPE html><html><head>\n";
    html += "<meta charset=\"UTF-8\">\n";
    html += "<title>" + title + "</title>\n";
    html += "<style>\n"
            "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: var(--bg); color: var(--fg); }\n"
            "nav a { margin: 0 10px; text-decoration: none; color: var(--link); }\n"
            ".btn { padding: 6px 12px; text-decoration: none; color: white; border: none; background: #2196F3; }\n"
            ".btn.off { background: #f44336; }\n"
            "canvas { background-color: #222; border: 1px solid #444; max-width: 100%; }\n"
            "</style>\n";
    html += "<script>\n"
            "const themes = {\n"
            "  dark: { '--bg': '#121212', '--fg': '#fff', '--link': '#90caf9' },\n"
            "  light: { '--bg': '#ffffff', '--fg': '#000', '--link': '#1e88e5' }\n"
            "};\n"
            "const setTheme = name => {\n"
            "  const root = document.documentElement;\n"
            "  const theme = themes[name];\n"
            "  Object.keys(theme).forEach(k => root.style.setProperty(k, theme[k]));\n"
            "  localStorage.setItem('theme', name);\n"
            "};\n"
            "window.onload = () => { setTheme(localStorage.getItem('theme') || 'dark'); };\n"
            "</script>\n";
    html += "</head><body>\n";
    html += "<nav>\n"
            "<a href=\"/\">Main</a>\n"
            "<a href=\"/logs\">Logs</a>\n"
            "<a href=\"/graph/inner\">Inner</a>\n"
            "<a href=\"/graph/outer\">Outer</a>\n"
            "<a href=\"/graph/room\">Room</a>\n"
            "<a href=\"/graph/humid\">Humidity</a>\n"
            "<a href=\"/graph/press\">Pressure</a>\n"
            "<button onclick=\"setTheme('light')\">☀️</button>\n"
            "<button onclick=\"setTheme('dark')\">🌙</button>\n"
            "</nav><hr>\n";
    return html;
}

String htmlFooter()
{
    return "</body></html>";
}

void handleAutoOn()
{
    relayManualOverride = false;
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAutoOff()
{
    relayManualOverride = true;
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleMainPage()
{
    String html = htmlHeader("Main");
    html += "<h1>Main</h1>";
    html += "<p><b>Room Temp:</b> " + String(getRoomTemp(), 1) + " °C</p>";
    html += "<p><b>Humidity:</b> " + String(getRoomHumidity(), 0) + " %</p>";
    html += "<p><b>Pressure:</b> " + String(getRoomPressure(), 1) + " mmHg</p>";
    html += "<p><b>Relay:</b> " + String(relayState ? "ON" : "OFF") + "</p>";
    html += relayState ? "<a class='btn off' href='/relay/off'>Turn OFF</a>" : "<a class='btn' href='/relay/on'>Turn ON</a>";
    html += "<p><b>Mode:</b> " + String(relayManualOverride ? "Manual" : "Auto") + "</p>";
    html += relayManualOverride
                ? "<a class='btn' href='/relay/auto/on'>Switch to AUTO</a>"
                : "<a class='btn off' href='/relay/auto/off'>Switch to MANUAL</a>";

    html += "<script>setInterval(() => location.reload(), 10000);</script>";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleLogsPage()
{
    String html = htmlHeader("Logs");
    html += "<h1>Logs</h1><table border='1'><tr><th>Time</th><th>Inner</th><th>Outer</th><th>Room</th><th>Humid</th><th>Press</th></tr>";
    for (int i = max(0, logCount - 60); i < logCount; i++)
    {
        const LogEntry &l = temperatureLogs[i];
        html += "<tr><td>" + l.timestamp.timestamp() + "</td><td>" + String(l.innerTemp, 1) + "</td><td>" + String(l.outerTemp, 1) + "</td><td>" + String(l.roomTemp, 1) + "</td><td>" + String(l.roomHumidity, 0) + "</td><td>" + String(l.roomPressure, 1) + "</td></tr>";
    }
    html += "</table>" + htmlFooter();
    server.send(200, "text/html", html);
}

void handleGraphPage(String type, const String &label, const String &color)
{
    String html = htmlHeader("Graph " + type);
    html += "<h1>" + label + " Graph</h1>";
    html += "<canvas id='chart'></canvas>";
    html += "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>";
    html += "<script>";
    html += "let chart;";
    html += "function loadData() {";
    html += "fetch('/data?type=" + type + "')";
    html += ".then(r => r.json())";
    html += ".then(data => {";
    html += "const ctx = document.getElementById('chart').getContext('2d');";
    html += "if (!chart) { chart = new Chart(ctx, { type: 'line', data: { labels: data.timestamps, datasets: [{ label: '" + label + "', data: data.values, borderColor: '" + color + "', fill: false }] },";
    html += "options: { responsive: true, scales: { x: { ticks:{color:'#fff'} }, y: { ticks:{color:'#fff'} } }, plugins: { legend: { labels: { color: '#fff' } } } } });";
    html += "} else { chart.data.labels = data.timestamps; chart.data.datasets[0].data = data.values; chart.update(); }";
    html += "});";
    html += "}";
    html += "loadData(); setInterval(loadData, 10000);";
    html += "</script>";
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
        if (type == "inner")
            v = temperatureLogs[i].innerTemp;
        else if (type == "outer")
            v = temperatureLogs[i].outerTemp;
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
    relayState = true;
    relayManualOverride = true;
    digitalWrite(RELAY_PIN, HIGH);
    lastRelayToggleTime = rtc.now();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRelayOff()
{
    relayState = false;
    relayManualOverride = true;
    digitalWrite(RELAY_PIN, LOW);
    lastRelayToggleTime = rtc.now();
    server.sendHeader("Location", "/");
    server.send(303);
}

void setupWebServer()
{
    server.on("/", handleMainPage);
    server.on("/logs", handleLogsPage);
    server.on("/graph/inner", []()
              { handleGraphPage("inner", "Inner Temp", "red"); });
    server.on("/graph/outer", []()
              { handleGraphPage("outer", "Outer Temp", "blue"); });
    server.on("/graph/room", []()
              { handleGraphPage("room", "Room Temp", "lime"); });
    server.on("/graph/humid", []()
              { handleGraphPage("humid", "Humidity", "aqua"); });
    server.on("/graph/press", []()
              { handleGraphPage("press", "Pressure", "orange"); });
    server.on("/data", handleGraphData);
    server.on("/relay/on", handleRelayOn);
    server.on("/relay/off", handleRelayOff);
    server.on("/relay/auto/on", handleAutoOn);
    server.on("/relay/auto/off", handleAutoOff);

    server.begin();
}

void handleWebRequests()
{
    server.handleClient();
}
