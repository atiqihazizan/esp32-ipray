#include "sound.h"
#include "config.h"
#include "solat.h"
#include "utils.h"
#include <esp_random.h>

// SD ROOT: suku jam RTC — minit%15==0 ; masuk waktu solat → azan sahaja.
static int           lastWarnIdx      = -1;
static int           lastAzanIdx      = -1;
static unsigned long azanMatchStart   = 0;
static bool          azanBeepDone     = false;
static int           lastMusicTick    = -1;

/**
 * Semak dan mainkan bunyi tepat jam (:00) mengikut mod (HOURLY_CHIME_MODE), 
 * atau bunyi suku jam (:15, :30, :45) mengikut mod (QUARTER_CHIME_MODE).
 */
 static void runQuarterHourChime(const DateTime& now) {
  if (!dfPlayerReady) return; // Keluar awal jika modul audio tidak sedia

  int h = now.hour();
  int m = now.minute();

  // --- Tepat Jam (xx:00) ---
  if (m == 0) {
    switch (HOURLY_CHIME_MODE) {
      case HOURLY_CHIME_MODE_SPEAK_FOLDERS:
        speakTime(h, m);
        Serial.printf("[sound] tepat jam %02d:00 (speakTime folders)\n", h);
        break;

      case HOURLY_CHIME_MODE_FULL_MUSIC:
      default:
        playSound(TRACK_SD_FULL_MUSIC, 400);
        Serial.printf("[sound] tepat jam %02d:00 trek %d (muzik penuh)\n", h, TRACK_SD_FULL_MUSIC);
        break;
    }
  } 
  // --- Suku Jam (xx:15, xx:30, xx:45) ---
  else if (m == 15 || m == 30 || m == 45) {
    switch (QUARTER_CHIME_MODE) {
      case QUARTER_CHIME_MODE_SPEAK_FOLDERS:
        // Akan menyebut "pukul sepuluh lima belas minit"
        speakTime(h, m);
        Serial.printf("[sound] suku jam %02d:%02d (speakTime folders)\n", h, m);
        break;

      case QUARTER_CHIME_MODE_SPECIFIC_TRACK:
        // Mainkan trek spesifik untuk suku jam
        playSound(TRACK_SD_QUARTER_MUSIC, 400);
        Serial.printf("[sound] suku jam %02d:%02d trek %d (muzik spesifik)\n", h, m, TRACK_SD_QUARTER_MUSIC);
        break;

      case QUARTER_CHIME_MODE_SHORT_RANDOM:
      default: {
        int tr = (esp_random() & 1U) ? TRACK_SD_SHORT_B : TRACK_SD_SHORT_A;
        playSound(tr, 400);
        Serial.printf("[sound] suku jam %02d:%02d trek %d (pendek rawak A/B)\n", h, m, tr);
        break;
      }
    }
  }
}

bool currentMinuteMatchesAnySolat(const DateTime& now) {
  if (!solatLoaded)
    return false;
  int nowMin = now.hour() * 60 + now.minute();
  const char* times[] = {
      todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
      todaySolat.asr,  todaySolat.maghrib, todaySolat.isha
  };
  for (int i = 0; i < 6; i++) {
    int sm = timeToMinutes(times[i]);
    if (sm > 0 && sm == nowMin)
      return true;
  }
  return false;
}

void handleSound() {
  if (!solatLoaded)
    return;
  if (currentState == SLEEP)
    return;

  DateTime now = rtc.now();
  int nowMin   = now.hour() * 60 + now.minute();
  const bool solatMinute = currentMinuteMatchesAnySolat(now);

  const char* times[] = {
      todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
      todaySolat.asr,  todaySolat.maghrib, todaySolat.isha
  };

  for (int i = 0; i < 6; i++) {
    int solatMin = timeToMinutes(times[i]);
    if (solatMin == 0)
      continue;

    int diff = solatMin - nowMin;

    if (diff == 1 && now.second() >= 28 && now.second() <= 32) {
      if (lastWarnIdx != i) {
        lastWarnIdx = i;
        if (dfPlayerReady) {
          playSound(TRACK_SD_NOTIFY, 0);
          delay(500);
          playSound(TRACK_SD_NOTIFY, 0);
          delay(500);
          playSound(TRACK_SD_NOTIFY, 0);
        }
        Serial.printf("[sound] warning 30s: %s\n", times[i]);
      }
    }

    if (diff == 0) {
      if (lastAzanIdx != i) {
        lastAzanIdx    = i;
        azanMatchStart = millis();
        azanBeepDone   = false;
      }
      if (!azanBeepDone) {
        azanBeepDone = true;
        if (dfPlayerReady) {
#if AZAN_USE_BEEP_PREAMBLE
          for (int j = 0; j < AZAN_PLAY_COUNT; j++)
            playSound(TRACK_SD_BEEP, 120);
#endif
          playSound(TRACK_SD_AZAN, 0);
        }
#if AZAN_USE_BEEP_PREAMBLE
        Serial.printf("[sound] beep x%d kemudian azan penuh: %s\n", AZAN_PLAY_COUNT, times[i]);
#else
        Serial.printf("[sound] azan penuh sahaja: %s\n", times[i]);
#endif
      }
    }
  }

  if (!solatMinute && now.second() <= 2 && ((now.minute() % 15) == 0)) {
    int tick = now.hour() * 60 + now.minute();
    if (lastMusicTick != tick) {
      lastMusicTick = tick;
      runQuarterHourChime(now);
    }
  }

  bool anyActive = false;
  for (int i = 0; i < 6; i++) {
    int solatMin = timeToMinutes(times[i]);
    if (nowMin == solatMin) {
      anyActive = true;
      break;
    }
  }
  if (!anyActive && azanMatchStart > 0 && millis() - azanMatchStart > 60000UL) {
    azanMatchStart = 0;
    azanBeepDone   = false;
  }
}
