#pragma once
#include <Arduino.h>

// ─── Setup (panggil sekali selepas initDfPlayer() dalam setup()) ──────
void serviceInit();

// ─── Loop tick (panggil sekali dalam loop(), tiada delay) ─────────────
void serviceTick();

// ─── Tangguh sebutan masa sehingga SD sedia ───────────────────────────
// Gantikan blok `if (dfPlayerReady) { appFsmEnqSpeak(...) }` dalam setup()
void serviceRequestStartupSpeak();

// ─── Dipindah dari utils.cpp — boleh dipanggil dari setup() ──────────
void connectWiFi();
void syncRTCwithNTP();