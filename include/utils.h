#pragma once
#include <Arduino.h>

void showInitializing(int dots);
void connectWiFi();
void syncRTCwithNTP();

/** UART2 + DFPlayer Mini — volume, EQ, SD check */
void initDfPlayer();

void handleSolatRetry();
void handleBlink();

/** Ditakrif dalam utils.cpp — dipanggil dari sound.cpp / buttons.cpp */
void playSound(int trackNumber, int delayMs = 0);

/** DFPlayer: folder jam lalu folder minit — rujuk DFPLAYER_SPEAK_FOLDER_* dalam config.h */
void speakTime(int hours, int minutes);

int rightToLen(const char *strText, int charW = 6, int padRight = 0);
void drawSignalBars(int x, int y);
void calculateHijri(int gYear, int gMonth, int gDay, int &hDay, int &hMonth, int &hYear);
void to12h(const char* time24, char* out); 