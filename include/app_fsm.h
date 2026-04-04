#pragma once
#include <Arduino.h>

/**
 * Mesin keadaan ringkas untuk gelung utama (tanpa delay panjang):
 * - Blink penunjuk masa (colon) ikut millis
 * - Audio DFPlayer: baris gilir kerja (play trek, jurang ms, speakTime)
 *
 * Panggil appFsmUpdate() setiap loop(); guna appFsmEnq* untuk gantikan playSound/speakTime berblok.
 */
void appFsmInit();
void appFsmUpdate();

bool appFsmAudioBusy();
/** Main ikut DFPLAYER_PLAY_MODE pada config.h (serba guna lama) */
bool appFsmEnqPlay(int track, int tailMs);
/** Main 00n.mp3 di punca kad SD (dfPlayer.play) */
bool appFsmEnqRoot(int track, int tailMs);
/** Main dari folder mp3/ pada kad (playMp3Folder) */
bool appFsmEnqMp3(int track, int tailMs);
/** Main trek dalam folder bernombor 01–99 (playFolder) */
bool appFsmEnqFolder(int folderNumber, int trackInFolder, int tailMs);
bool appFsmEnqGap(unsigned ms);
/** Sebut jam/minit (folder 01/02) */
bool appFsmEnqSpeak(int hours, int minutes);
