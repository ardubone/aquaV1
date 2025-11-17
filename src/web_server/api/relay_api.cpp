// relay_api.cpp
#include "relay_api.h"
#include "../../../include/web_server.h"
#include "../../../include/relay.h"
#include "../../../include/config.h"
#include <RTClib.h>

extern WebServer server;
extern RTC_DS1307 rtc;

#ifdef DEBUG_MODE
#include "../../debug_mocks.h"
#endif

void handleRelayOn()
{
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleRelay(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRelayOff()
{
    if (getRelayState()) {
#ifdef DEBUG_MODE
        DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
        DateTime now = rtc.now();
#endif
        toggleRelay(now);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRelayTank10On() {
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleRelayTank10(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRelayTank10Off() {
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleRelayTank10(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAutoOn()
{
    resetRelayOverride();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAutoOff()
{
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleRelay(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAutoTank10On() {
    resetRelayTank10Override();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAutoTank10Off() {
#ifdef DEBUG_MODE
    DateTime now = isRtcInitialized ? rtc.now() : getMockTime();
#else
    DateTime now = rtc.now();
#endif
    toggleRelayTank10(now);
    server.sendHeader("Location", "/");
    server.send(303);
}

