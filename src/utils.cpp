#include "utils.h"
#include "config.h"
#include "secrets.h"
#include "solat.h"
#include <WiFi.h>
#include <time.h>
#include <pgmspace.h>
#include <string.h>

int rightToLen(const char* strText, int charW, int padRight) {
  return 128 - ((int)strlen(strText) * charW) - padRight;
}

void drawSignalBars(int x, int y) {
  int rssi = WiFi.RSSI();
  int bars = (rssi > -50) ? 4 : (rssi > -60) ? 3 : (rssi > -70) ? 2 : (rssi > -80) ? 1 : 0;
  for (int i = 0; i < 4; i++) {
    int h = (i + 1) * 2;
    if (i < bars) display.fillRect(x + i * 4, y - h, 3, h, SSD1306_WHITE);
    else          display.drawRect(x + i * 4, y - h, 3, h, SSD1306_WHITE);
  }
}

// ─── Skrin init / WiFi / NTP ───────────────────────────

void showInitializing(int dots) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  const char* text  = "Initializing";
  const char* text1 = "Initializing...";
  int textWidth = (int)strlen(text1) * 6;
  int x = (SCREEN_WIDTH - textWidth) / 2;
  int y = (SCREEN_HEIGHT / 2) - 4;

  display.setCursor(x, y);
  display.print(text);

  for (int i = 0; i < dots; i++) {
    display.print(".");
  }

  display.display();
}

void connectWiFi() {
  WiFi.begin((char*)WIFI_SSID, (char*)WIFI_PASSWORD);
  Serial.print("[wifi] connecting");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000UL) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[wifi] connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[wifi] timeout — will retry in loop");
  }
}

void syncRTCwithNTP() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ntp] skip — no WiFi");
    return;
  }

  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("[ntp] syncing");
  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  Serial.println();

  if (retry >= 20) {
    Serial.println("[ntp] sync failed, using RTC as-is");
    return;
  }

  rtc.adjust(DateTime(
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon  + 1,
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  ));

  Serial.printf("[ntp] RTC updated: %04d-%02d-%02d %02d:%02d:%02d\n",
    timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

// ─── Input / solat semula ───────────────────────────────

void handleButtons(){
  if (digitalRead(BTN_MENU) == LOW) {
    delay(200);
    if      (currentState == SLEEP) currentState = HOME;
    else if (currentState == HOME)  currentState = MENU;
    else if (currentState == MENU)  currentState = HOME;
  }
  if (digitalRead(BTN_UP) == LOW) {
    delay(200);
    if (currentState == MENU)
      menuIndex = (menuIndex - 1 + menuItems) % menuItems;
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    delay(200);
    if (currentState == MENU)
      menuIndex = (menuIndex + 1) % menuItems;
  }
  if (digitalRead(BTN_SET) == LOW) {
    delay(200);
    if      (currentState == SLEEP) currentState = HOME;
    else if (currentState == HOME)  currentState = SLEEP;
    else if (currentState == MENU) {
      if      (menuIndex == 0) currentState = SET_TAKWIM;
      else if (menuIndex == 1) currentState = SET_DATE;
      else if (menuIndex == 2) currentState = SET_TIME;
      else if (menuIndex == 3) currentState = SET_WIFI;
    }
  }
  if (digitalRead(BTN_RET) == LOW) {
    delay(200);
    if      (currentState == SLEEP) currentState = HOME;
    else if (currentState == MENU)  currentState = HOME;
    else if (currentState != HOME)  currentState = MENU;
  }
  if (digitalRead(BTN_LAYOUT) == LOW) {
    delay(200);
    if (currentState == HOME) {
      displayLayout = (displayLayout == 1) ? 2 : 1;

      if (preferences.begin("cfg", false)) {
        preferences.putInt("layout", displayLayout);
        preferences.end();
      }

      if (dfPlayerReady && !currentMinuteMatchesAnySolat(rtc.now()))
        playSound(TRACK_SD_MUSIC_B, 100);
      Serial.printf("[layout] switched to layout %d\n", displayLayout);
    }
  }
}

void handleSolatRetry(){
  if (solatLoaded) return;
  if (strlen(savedZone) == 0) return;
  if (millis() - lastSolatRetry < SOLAT_RETRY_MS) return;

  lastSolatRetry = millis();
  loadSolatFromPrefs(savedZone);

  if (!solatLoaded && WiFi.status() == WL_CONNECTED) {
    fetchAndSaveSolat(savedZone);
  } else if (!solatLoaded && WiFi.status() != WL_CONNECTED) {
    Serial.println("[wifi] disconnected, reconnecting...");
    connectWiFi();
  }
}

// ─── Kipas masa / audio solat ─────────────────────────
// SD ROOT: 001 azan, 002 warning, 003/004 music pada :00 dan :15 (trek berbeza dalam sejam).
// Pada minit mana-mana waktu solat: DFPlayer hanya main trek azan (tiada muzik :00/:15 & bunyi layout).

static int           lastWarnIdx    = -1;
static int           lastAzanIdx    = -1;
static unsigned long azanMatchStart = 0;
static bool          azanBeepDone   = false;
static int           lastMusicTick  = -1;

/** true jika minit semasa sepadan dengan mana-mana waktu solat (Subuh–Isyak): hanya azan dibenarkan pada DFPlayer. */
static bool currentMinuteMatchesAnySolat(const DateTime& now) {
  if (!solatLoaded) return false;
  int nowMin = now.hour() * 60 + now.minute();
  const char* times[] = {
    todaySolat.fajr,    todaySolat.syuruk, todaySolat.dhuhr,
    todaySolat.asr,     todaySolat.maghrib, todaySolat.isha
  };
  for (int i = 0; i < 6; i++) {
    int sm = timeToMinutes(times[i]);
    if (sm > 0 && sm == nowMin) return true;
  }
  return false;
}

void handleBlink() {
  if (millis() - lastBlinkTime >= 500UL) {
    colonBlink    = !colonBlink;
    lastBlinkTime = millis();
  }
}

void handleBuzzer() {
  if (!solatLoaded) return;
  if (currentState == SLEEP) return;

  DateTime now = rtc.now();
  int nowMin   = now.hour() * 60 + now.minute();
  const bool  solatMinute = currentMinuteMatchesAnySolat(now);

  const char* times[] = {
    todaySolat.fajr,    todaySolat.syuruk, todaySolat.dhuhr,
    todaySolat.asr,     todaySolat.maghrib, todaySolat.isha
  };

  for (int i = 0; i < 6; i++) {
    int solatMin = timeToMinutes(times[i]);
    if (solatMin == 0) continue;

    int diff = solatMin - nowMin;

    if (diff == 1 && now.second() >= 28 && now.second() <= 32) {
      if (lastWarnIdx != i) {
        lastWarnIdx = i;
        if (dfPlayerReady) {
          playSound(TRACK_SD_WARNING, 0); delay(500);
          playSound(TRACK_SD_WARNING, 0); delay(500);
          playSound(TRACK_SD_WARNING, 0);
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
          for (int j = 0; j < AZAN_PLAY_COUNT; j++) {
            playSound(TRACK_SD_AZAN, 0);
            delay(1500);
          }
        }
        Serial.printf("[sound] azan: %s\n", times[i]);
      }
    }
  }

  /* Muzik pada :00 / :15 — jangan main jika minit ini waktu solat (hanya azan dibenarkan) */
  if (!solatMinute && now.second() <= 2 && (now.minute() == 0 || now.minute() == 15)) {
    int tick = now.hour() * 60 + now.minute();
    if (lastMusicTick != tick) {
      lastMusicTick = tick;
      int atHourStart =
          ((now.hour() % 2) == 0) ? TRACK_SD_MUSIC_A : TRACK_SD_MUSIC_B;
      int tr = (now.minute() == 0)
                   ? atHourStart
                   : ((atHourStart == TRACK_SD_MUSIC_A) ? TRACK_SD_MUSIC_B
                                                        : TRACK_SD_MUSIC_A);
      if (dfPlayerReady)
        playSound(tr, 400);
      Serial.printf("[sound] music %02d:%02d trek %d\n", now.hour(), now.minute(), tr);
    }
  }

  bool anyActive = false;
  for (int i = 0; i < 6; i++) {
    int solatMin = timeToMinutes(times[i]);
    if (nowMin == solatMin) { anyActive = true; break; }
  }
  if (!anyActive && azanMatchStart > 0 && millis() - azanMatchStart > 60000UL) {
    azanMatchStart = 0;
    azanBeepDone   = false;
  }
}

// ─── Hijri helpers ────────────────────────────────────

static bool hijriMonthIs30(int monthIndex) {
  if (monthIndex < 0) return false;
  int byteIdx = monthIndex / 8;
  int bitIdx  = 7 - (monthIndex % 8);  // MSB first
  uint8_t b   = pgm_read_byte(&HIJRI_DATA[byteIdx]);
  return (b >> bitIdx) & 1;
}

static int hijriMonthLen(int monthIndex) {
  return hijriMonthIs30(monthIndex) ? 30 : 29;
}

// ─── calculateHijri ───────────────────────────────────
// Tukar tarikh Masihi → Hijri menggunakan bitmap HIJRI_DATA
void calculateHijri(int gYear, int gMonth, int gDay,int &hDay, int &hMonth, int &hYear) {

  // Kira Julian Day Number (JDN) untuk tarikh input
  int  a   = (14 - gMonth) / 12;
  int  y   = gYear + 4800 - a;
  int  m   = gMonth + 12 * a - 3;
  long jdn = gDay + (153L*m + 2)/5 + 365L*y + y/4 - y/100 + y/400 - 32045L;

  // Jarak hari dari titik mula bitmap
  long daysDiff = jdn - HIJRI_JDN_REF;

  // Titik mula: 25 Syaaban 1420H
  // Offset: maju ke awal bulan Syaaban 1420H (tolak 23 hari)
	//   long remaining = daysDiff + (HIJRI_START_DAY - 2);
  long remaining = daysDiff + HIJRI_DAY_OFFSET;

  int refYear  = HIJRI_START_YEAR;
  int refMonth = HIJRI_START_MONTH;
  int mIdx     = 0;  // indeks bitmap

  // Langkau bulan penuh
  while (remaining >= hijriMonthLen(mIdx)) {
    remaining -= hijriMonthLen(mIdx);
    mIdx++;
    refMonth++;
    if (refMonth > 12) {
      refMonth = 1;
      refYear++;
    }
  }

  hYear  = refYear;
  hMonth = refMonth;
  hDay   = (int)remaining + 1;

  // Pastikan dalam had selamat
  if (hDay < 1)  hDay = 1;
  if (hDay > 30) hDay = 30;
  if (hMonth < 1)  hMonth = 1;
  if (hMonth > 12) hMonth = 12;
}

// Tukar "HH:MM" (24h) → "H:MM" (12h) tanpa AM/PM
// Contoh: "19:29" → "7:29"  |  "06:11" → "6:11"  |  "00:30" → "12:30"
void to12h(const char* time24, char* out) {
  if (!time24 || strlen(time24) < 5) {
    strcpy(out, time24 ? time24 : "");
    return;
  }
  int h = (time24[0] - '0') * 10 + (time24[1] - '0');
  int m = (time24[3] - '0') * 10 + (time24[4] - '0');

  int h12 = h % 12;
  if (h12 == 0) h12 = 12;

  sprintf(out, "%d:%02d", h12, m);
}