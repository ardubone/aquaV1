// logger_api.cpp
#include "logger_api.h"
#include "../../../include/web_server.h"
#include "../../../include/logger.h"

extern WebServer server;

void handleCriticalLogsApi()
{
    uint16_t count = 0;
    CriticalLogEntry* criticalLogs = getCriticalLogs(&count);
    
    String json = "{\"count\":" + String(count) + ",\"logs\":[";
    
    for (uint16_t i = 0; i < count; i++) {
        if (i > 0) json += ",";
        
        json += "{";
        json += "\"timestamp\":\"" + criticalLogs[i].timestamp.timestamp() + "\",";
        json += "\"parameterType\":" + String(criticalLogs[i].parameterType) + ",";
        
        String paramName = "";
        float currentValue = 0;
        if (criticalLogs[i].parameterType == 0) {
            paramName = "tank20";
            currentValue = criticalLogs[i].tank20Temp;
        } else if (criticalLogs[i].parameterType == 1) {
            paramName = "tank10";
            currentValue = criticalLogs[i].tank10Temp;
        } else if (criticalLogs[i].parameterType == 2) {
            paramName = "humidity";
            currentValue = criticalLogs[i].roomHumidity;
        }
        
        json += "\"parameter\":\"" + paramName + "\",";
        json += "\"currentValue\":" + String(currentValue, 2) + ",";
        json += "\"previousAvg\":" + String(criticalLogs[i].previousAvg, 2) + ",";
        json += "\"changePercent\":" + String(criticalLogs[i].changePercent, 2);
        json += "}";
    }
    
    json += "]}";
    
    server.send(200, "application/json", json);
}



