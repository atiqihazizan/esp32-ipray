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

/** Satu arahan main DFPlayer: 0=mp3/ 1=ROOT 2=folder jam(01) 3=folder minit(02) */
void dfPlay(int trackNumber, int mode) {
  switch (mode) {
    case 1:
      dfPlayer.play(trackNumber);
      break;
    case 2:
      dfPlayer.playFolder(DFPLAYER_SPEAK_FOLDER_HOUR, static_cast<uint8_t>(trackNumber));
      break;
    case 3:
      dfPlayer.playFolder(DFPLAYER_SPEAK_FOLDER_MINUTE, static_cast<uint8_t>(trackNumber));
      break;
    default:
      dfPlayer.playMp3Folder(trackNumber);
      break;
  }
}

void dfPlayFolder(int folderNumber, int trackInFolder) {
  uint8_t f = (folderNumber < 1) ? 1 : (folderNumber > 99 ? 99 : (uint8_t)folderNumber);
  uint8_t t = (trackInFolder < 1) ? 1 : (trackInFolder > 255 ? 255 : (uint8_t)trackInFolder);
  dfPlayer.playFolder(f, t);
}

void utilsDfPlayerSendStop() {
  dfPlayer.stop();
}

void utilsDfPlayerSendVolume(int v) {
  dfPlayer.volume((uint8_t)v);
}

bool utilsDfPlayerOutputIdle() {
#if DFPLAYER_BUSY_ACTIVE_LOW
  return digitalRead(DFPLAYER_BUSY_PIN) == HIGH;
#else
  return digitalRead(DFPLAYER_BUSY_PIN) == LOW;
#endif
}

bool utilsDfPlayerScanNumberedFolders(int* outFolderCount, int* outFilesPerFolder, int maxFolders) {
  if (!dfPlayerReady || !outFolderCount || !outFilesPerFolder || maxFolders < 1)
    return false;

  dfPlayer.stop();
  delay(120);

  int fc = dfPlayer.readFolderCounts();
  if (fc < 0)
    fc = 0;
  if (fc > maxFolders)
    fc = maxFolders;
  *outFolderCount = fc;

  for (int i = 1; i <= fc; i++) {
    int n = dfPlayer.readFileCountsInFolder(i);
    outFilesPerFolder[i - 1] = (n < 0) ? 0 : n;
    delay(100);
  }
  return true;
}

void initDfPlayer() {
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  delay(DFPLAYER_BOOT_DELAY_MS);

#if DFPLAYER_USE_ACK
  const bool dfUseAck = true;
#else
  const bool dfUseAck = false;
#endif

  bool begun = false;
  for (int attempt = 0; attempt < DFPLAYER_BEGIN_RETRY_COUNT; attempt++) {
    if (dfPlayer.begin(dfSerial, dfUseAck)) {
      begun = true;
      break;
    }
    delay(DFPLAYER_BEGIN_RETRY_DELAY_MS);
  }

  if (begun) {
    dfPlayerReady = true;
    dfPlayerSdOk  = false;
    delay(500);

    dfPlayer.volume(30);
    delay(150);
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    delay(150);
    dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    delay(500);
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