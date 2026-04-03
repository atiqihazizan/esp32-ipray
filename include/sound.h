#pragma once
#include <RTClib.h>

/** true jika minit RTC sepadan dengan mana-mana waktu solat (Subuh–Isyak). */
bool currentMinuteMatchesAnySolat(const DateTime& now);

/** Azan, amaran, loceng suku jam / jam penuh — DFPlayer & logik masa. */
void handleSound();
