#pragma once
#include <Arduino.h>

void showInitializing(int dots);
void connectWiFi();
void syncRTCwithNTP();

/** UART2 + DFPlayer Mini — volume, EQ, SD check */
void initDfPlayer();

void handleSolatRetry();

/** Arahan DFPlayer segera (tiada delay) — untuk app_fsm */
void utilsDfPlayerSendStop();
void utilsDfPlayerSendVolume(int v);
void utilsDfPlayerSendPlayIndexed(int track);
void utilsDfPlayerSendPlayFolder(uint8_t folder, uint8_t file);

/** true = DFPlayer tidak main (ikut DFPLAYER_BUSY_* pada config.h) */
bool utilsDfPlayerOutputIdle();

/** Hantar ke baris gilir audio app_fsm (tidak berblok) */
void playSound(int trackNumber, int delayMs = 0);
void speakTime(int hours, int minutes);

int rightToLen(const char *strText, int charW = 6, int padRight = 0);
void drawSignalBars(int x, int y);
void calculateHijri(int gYear, int gMonth, int gDay, int &hDay, int &hMonth, int &hYear);
void to12h(const char* time24, char* out); 