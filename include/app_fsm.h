#pragma once
#include <Arduino.h>

/**
 * Mesin keadaan ringkas untuk gelung utama (tanpa delay panjang):
 * - Blink penunjuk masa (colon) ikut millis
 * - Audio DFPlayer: baris gilir kerja (play trek, jurang ms, speakTime)
 *
 * Panggil appFsmUpdate() setiap loop(); guna enqueue untuk gantikan playSound/speakTime berblok.
 */
void appFsmInit();
void appFsmUpdate();

bool appFsmAudioBusy();
bool appFsmAudioEnqueuePlay(int track, int tailMs);
bool appFsmAudioEnqueueGap(unsigned ms);
bool appFsmAudioEnqueueSpeakTime(int hours, int minutes);
