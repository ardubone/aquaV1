// logger_api.cpp
#include "logger_api.h"
#include "../../../include/web_server.h"
#include "../../../include/logger.h"
#include <RTClib.h>

extern WebServer server;

static inline void appendJsonNumberOrNull(String& json, float value, uint8_t decimals) {
    if (isnan(value) || isinf(value)) {
        json += "null";
        return;
    }
    json += String(value, (unsigned int)decimals);
}

void handleCriticalLogsApi()
{
    uint16_t count = 0;
    CriticalLogEntry* criticalLogs = getCriticalLogs(&count);
    
    String json = "{\"count\":" + String(count) + ",\"logs\":[";
    
    for (uint16_t i = 0; i < count; i++) {
        if (i > 0) json += ",";
        
        DateTime dt = uint32ToDateTime(criticalLogs[i].timestamp);
        
        json += "{";
        json += "\"timestamp\":\"" + dt.timestamp() + "\",";
        json += "\"parameterType\":" + String(criticalLogs[i].parameterType) + ",";
        
        String paramName = "";
        float currentValue = 0;
        if (criticalLogs[i].parameterType == 0) {
            paramName = "tankLrg";
            currentValue = int16ToFloat(criticalLogs[i].tankLrgTemp);
        } else if (criticalLogs[i].parameterType == 1) {
            paramName = "tankSml";
            currentValue = int16ToFloat(criticalLogs[i].tankSmlTemp);
        } else if (criticalLogs[i].parameterType == 2) {
            paramName = "humidity";
            currentValue = uint16ToFloat(criticalLogs[i].roomHumidity);
        }
        
        json += "\"parameter\":\"" + paramName + "\",";
        json += "\"currentValue\":";
        appendJsonNumberOrNull(json, currentValue, 2);
        json += ",\"previousAvg\":";
        appendJsonNumberOrNull(json, int16ToFloat(criticalLogs[i].previousAvg), 2);
        json += ",\"changePercent\":";
        appendJsonNumberOrNull(json, uint16ToFloat(criticalLogs[i].changePercent), 2);
        json += "}";
    }
    
    json += "]}";
    
    server.send(200, "application/json", json);
}

void handleHourlyLogsApi()
{
    uint16_t count = 0;
    LogEntry* logs = getLogs(&count);
    
    String json = "{\"count\":" + String(count) + ",\"logs\":[";
    
    for (uint16_t i = 0; i < count; i++) {
        if (i > 0) json += ",";
        
        DateTime dt = uint32ToDateTime(logs[i].timestamp);
        
        json += "{";
        json += "\"timestamp\":\"" + dt.timestamp() + "\",";
        json += "\"tankLrgTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].tankLrgTemp), 1);
        json += ",\"tankSmlTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].tankSmlTemp), 1);
        json += ",\"roomTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].roomTemp), 1);
        json += ",\"roomHumidity\":";
        appendJsonNumberOrNull(json, uint16ToFloat(logs[i].roomHumidity), 0);
        json += ",\"roomPressure\":";
        appendJsonNumberOrNull(json, uint16ToFloat(logs[i].roomPressure), 1);
        json += ",";
        json += "\"samplesCount\":" + String(logs[i].samplesCount);
        json += "}";
    }
    
    json += "]}";
    
    server.send(200, "application/json", json);
}

void handleHourlyLogs72Api()
{
    uint16_t count = 0;
    LogEntry* logs = getLogsFromSPIFFS(&count);
    
    String json = "{\"count\":" + String(count) + ",\"logs\":[";
    
    for (uint16_t i = 0; i < count; i++) {
        if (i > 0) json += ",";
        
        DateTime dt = uint32ToDateTime(logs[i].timestamp);
        
        json += "{";
        json += "\"timestamp\":\"" + dt.timestamp() + "\",";
        json += "\"tankLrgTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].tankLrgTemp), 1);
        json += ",\"tankSmlTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].tankSmlTemp), 1);
        json += ",\"roomTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].roomTemp), 1);
        json += ",\"roomHumidity\":";
        appendJsonNumberOrNull(json, uint16ToFloat(logs[i].roomHumidity), 0);
        json += ",\"roomPressure\":";
        appendJsonNumberOrNull(json, uint16ToFloat(logs[i].roomPressure), 1);
        json += ",";
        json += "\"samplesCount\":" + String(logs[i].samplesCount);
        json += "}";
    }
    
    json += "]}";
    
    server.send(200, "application/json", json);
}

void handleMinutesLogsApi()
{
    uint8_t count = 0;
    MinuteLogEntry* logs = getCurrentHourMinuteLogs(&count);
    
    String json = "{\"count\":" + String(count) + ",\"logs\":[";
    
    for (uint8_t i = 0; i < count; i++) {
        if (i > 0) json += ",";
        
        DateTime dt = uint32ToDateTime(logs[i].timestamp);
        
        json += "{";
        json += "\"timestamp\":\"" + dt.timestamp() + "\",";
        json += "\"tankLrgTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].tankLrgTemp), 1);
        json += ",\"tankSmlTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].tankSmlTemp), 1);
        json += ",\"roomTemp\":";
        appendJsonNumberOrNull(json, int16ToFloat(logs[i].roomTemp), 1);
        json += ",\"roomHumidity\":";
        appendJsonNumberOrNull(json, uint16ToFloat(logs[i].roomHumidity), 0);
        json += ",\"roomPressure\":";
        appendJsonNumberOrNull(json, uint16ToFloat(logs[i].roomPressure), 1);
        json += ",";
        json += "\"samplesCount\":" + String(logs[i].samplesCount);
        json += "}";
    }
    
    json += "]}";
    
    server.send(200, "application/json", json);
}



