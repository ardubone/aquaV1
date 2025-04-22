#pragma once
#include <WebServer.h>

void setupWebServer();
void handleWebRequests();
void handleAutoFeederActivate();
void handleAutoFeederStatus();

extern WebServer server;