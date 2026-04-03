#include "utils.h"
#include "config.h"
#include "secrets.h"
#include "solat.h"
#include <WiFi.h>
#include <time.h>
#include <pgmspace.h>
#include <string.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

HardwareSerial     dfSerial(2);
DFRobotDFPlayerMini dfPlayer;
bool               dfPlayerReady       = false;
bool               dfPlayerSdOk        = false;
int                dfPlayerSdFileCount = -1;

/** Satu arahan main: 0=mp3/ 1=ROOT 2=folder01 */
static void dfPlayerPlayOneMode(int trackNumber, int mode) {
  switch (mode) {
    case 1: dfPlayer.play(trackNumber); break;
    case 2: dfPlayer.playFolder(1, static_cast<uint8_t>(trackNumber)); break;
    default: dfPlayer.playMp3Folder(trackNumber); break;
  }
}

static void dfPlayerPlayIndexedTrack(int trackNumber) {
  dfPlayerPlayOneMode(trackNumber, DFPLAYER_PLAY_MODE);
}

void initDfPlayer() {
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  delay(500);

#if DFPLAYER_USE_ACK
  const bool dfUseAck = true;
#else
  const bool dfUseAck = false;
#endif

  if (dfPlayer.begin(dfSerial, dfUseAck)) {
    dfPlayerReady = true;
    dfPlayerSdOk  = false;
    delay(500);

    dfPlayer.volume(30);
    delay(150);
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    delay(150);
    dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    delay(150);
    /* Jangan guna start() di sini — ia boleh picu main sebelum masa; stop() matikan autoplay selepas reset */
    dfPlayer.stop();
    delay(250);

    dfPlayerSdFileCount = dfPlayer.readFileCounts(DFPLAYER_DEVICE_SD);
    dfPlayer.stop();
    delay(150);

    dfPlayerSdOk = (dfPlayerSdFileCount > 0);
    if (!dfPlayerSdOk)
      dfPlayerReady = false;
  } else {
    dfPlayerReady       = false;
    dfPlayerSdOk        = false;
    dfPlayerSdFileCount = -1;
  }
}

void playSound(int trackNumber, int delayMs) {
  if (!dfPlayerReady)
    return;
  dfPlayer.stop();
  delay(80);
  dfPlayer.volume(30);
  delay(50);
  dfPlayerPlayIndexedTrack(trackNumber);
  delay(200);
  if (delayMs > 0)
    delay(delayMs);
}

void speakTime(int hours, int minutes) {
  if (!dfPlayerReady)
    return;
  if (hours < 0)
    hours = 0;
  else if (hours > 23)
    hours = 23;
  if (minutes < 0)
    minutes = 0;
  else if (minutes > 59)
    minutes = 59;

  /*
   * Folder 01 jam: 001.mp3 … 012.mp3 (12j). RTC jam 00 & 12 → trek 12 (012.mp3).
   * Folder 02 minit: 001.mp3 … 060.mp3. Minit 00 → 060.mp3 (indeks 60); lain 1–59.
   */
  int h12 = hours % 12;
  if (h12 == 0)
    h12 = 12;
  const int minTrack = (minutes == 0) ? 60 : minutes;

  dfPlayer.stop();
  delay(80);
  dfPlayer.volume(30);
  delay(50);
  dfPlayer.playFolder((uint8_t)DFPLAYER_SPEAK_FOLDER_HOUR, (uint8_t)h12);
  delay(SPEAK_TIME_MS_AFTER_HOUR);
  dfPlayer.playFolder((uint8_t)DFPLAYER_SPEAK_FOLDER_MINUTE, (uint8_t)minTrack);
  delay(SPEAK_TIME_MS_AFTER_MINUTE);
}

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

// ─── Solat semula ───────────────────────────────────────

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

void handleBlink() {
  if (millis() - lastBlinkTime >= 500UL) {
    colonBlink    = !colonBlink;
    lastBlinkTime = millis();
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