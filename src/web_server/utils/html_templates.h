// html_templates.h
#pragma once

#include <Arduino.h>

String htmlHeader(const String &title);
String htmlFooter();
String generateNavMenu(const String& activePage = "");

