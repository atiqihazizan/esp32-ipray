#pragma once
#include <Arduino.h>

void showInitializing(int dots);
void connectWiFi();
void syncRTCwithNTP();

void handleButtons();
void handleSolatRetry();
void handleBlink();
void handleBuzzer();

/** Ditakrif dalam main.cpp — dipanggil dari util/input */
void playSound(int trackNumber, int delayMs = 0);

int rightToLen(const char *strText, int charW = 6, int padRight = 0);
void drawSignalBars(int x, int y);
void calculateHijri(int gYear, int gMonth, int gDay, int &hDay, int &hMonth, int &hYear);
void to12h(const char* time24, char* out); 