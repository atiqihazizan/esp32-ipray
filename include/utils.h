#pragma once
#include <Arduino.h>

void showInitializing(int dots);

/** UART2 + DFPlayer Mini — volume, EQ, SD check */
void initDfPlayer();

/** Arahan DFPlayer segera (tiada delay) — untuk app_fsm */
void utilsDfPlayerSendStop();
void utilsDfPlayerSendVolume(int v);
/** Main trek: 0=mp3 1=root 2=folder jam 3=folder minit (speakTime) */
void dfPlay(int trackNumber, int mode);

/** Main fail dalam folder bernombor (01–99), nombor trek 1–255 dalam folder */
void dfPlayFolder(int folderNumber, int trackInFolder);

/** true = DFPlayer tidak main (ikut DFPLAYER_BUSY_* pada config.h) */
bool utilsDfPlayerOutputIdle();

/**
 * Imbas folder bernombor pada SD (01, 02, …) melalui UART DFPlayer.
 * Isi outFiles[i] = bilangan fail dalam folder (i+1). Memanggil stop() — blok seketika.
 */
bool utilsDfPlayerScanNumberedFolders(int* outFolderCount, int* outFilesPerFolder, int maxFolders);

int rightToLen(const char *strText, int charW = 6, int padRight = 0);
void drawSignalBars(int x, int y);
void calculateHijri(int gYear, int gMonth, int gDay, int &hDay, int &hMonth, int &hYear);
void to12h(const char* time24, char* out); 