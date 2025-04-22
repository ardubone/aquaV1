#pragma once

#include <Arduino.h>
#include <RTClib.h>
#include "button.h"
#include "limit_switch.h"
#include "mosfet.h"
#include "autofeeder_scheduler.h"

// Объявления функций
void initAutoFeeder();
void setupAutoFeederSchedule();
void updateAutoFeeder();
bool activateFeeder(); 