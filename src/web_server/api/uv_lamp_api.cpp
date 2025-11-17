// uv_lamp_api.cpp
#include "uv_lamp_api.h"
#include "../../../include/web_server.h"
#include "../../../include/relay.h"
#include "../../../include/config.h"
#include <RTClib.h>

extern WebServer server;
extern RTC_DS1307 rtc;

#ifdef DEBUG_MODE
#include "../../debug_mocks.h"
#endif

// Обработчики для UV лампы Tank10
void handleUvLampTank10On() {
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleUvLampTank10(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleUvLampTank10Off() {
    if (getUvLampTank10State()) {
#ifdef DEBUG_MODE
        DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
        DateTime now = rtc.now();
#endif
        toggleUvLampTank10(now);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleUvLampTank10AutoOn() {
    resetUvLampTank10Override();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleUvLampTank10AutoOff() {
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleUvLampTank10(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

// Обработчики для UV лампы Tank20
void handleUvLampTank20On() {
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleUvLampTank20(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleUvLampTank20Off() {
    if (getUvLampTank20State()) {
#ifdef DEBUG_MODE
        DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
        DateTime now = rtc.now();
#endif
        toggleUvLampTank20(now);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleUvLampTank20AutoOn() {
    resetUvLampTank20Override();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleUvLampTank20AutoOff() {
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleUvLampTank20(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

