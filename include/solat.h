#pragma once
#include <Arduino.h>
#include <RTClib.h>
#include "config.h"

void fetchAndSaveSolat(const char* zone);
void loadSolatFromPrefs(const char* zone);
int  timeToMinutes(const char* t);
int  getCurrentSolatIndex(DateTime now);
int  getNextSolatIndex(DateTime now);
void getSolatLabel(int idx, char* out);
void getSolatTime(int idx, SolatTime& s, char* out);
void copySolatTime(const SolatTime& src, SolatTime& dst);