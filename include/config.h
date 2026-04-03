#pragma once
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Preferences.h>

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1

// ─── Pin config ───────────────────────────────────────
#define BTN_MENU        15
#define BTN_UP           4
#define BTN_DOWN        14
#define BTN_SET         27
#define BTN_RET          5
#define BTN_LAYOUT      23

// Buzzer pasif (LEDC PWM). Tukar pin jika bertindih dengan wayar lain.
#define BUZZER_PIN       13
#define BUZZER_CHANNEL   0
#define BUZZER_RES       8
#define BUZZER_ALERT_HZ    880
#define BUZZER_ALERT_MS   120

// ─── DFPlayer Mini MP3-TF-16P ────────────────────────
// Old pins (jika tidak berfungsi):
// #define DFPLAYER_RX     16  // ESP32 RX → DFPlayer TX
// #define DFPLAYER_TX     17  // ESP32 TX → DFPlayer RX

// New pins (alternative):
#define DFPLAYER_RX     25  // ESP32 RX → DFPlayer TX
#define DFPLAYER_TX     26  // ESP32 TX → DFPlayer RX

// Main DFPlayer: tetap mode 1 — dfPlayer.play(n), fail MP3 di punca kad (ROOT), cth 001.mp3 / 0003.mp3
#define DFPLAYER_PLAY_MODE   1

// 0 = UART tanpa ACK (sesetengah modul lebih stabil); 1 = dengan ACK
#define DFPLAYER_USE_ACK     0

/** ROOT play(n): 1=azan 2=warning 3/4=music (:00 & :15 — :15 trek lain daripada :00 sejam) */
#define TRACK_SD_AZAN     1
#define TRACK_SD_WARNING  2
#define TRACK_SD_MUSIC_A  3
#define TRACK_SD_MUSIC_B  4
/** Bilangan main trek azan bila masuk waktu solat */
#define AZAN_PLAY_COUNT   5

// Buzz + DFPlayer berkala (ms); 0 = matikan — trek muzik jika DFPlayer dihidupkan semula
#define PLAY_SOUND_INTERVAL_MS    3000
#define PLAY_SOUND_INTERVAL_TRACK TRACK_SD_MUSIC_B

extern Adafruit_SSD1306 display;
// extern RTC_DS1307 rtc;
extern RTC_DS3231 rtc;
extern Preferences preferences;

extern const char* daysOfWeek[];
extern const char* monthsOfYear[];
extern const char* monthsOfYearFull[];
extern const char* solatW[];
extern const char* hijriOfYear[];
extern const char* hijriOfYearFull[];

enum State { HOME, MENU, SET_TAKWIM, SET_DATE, SET_TIME, SET_WIFI, SLEEP };
extern State currentState;
extern int menuIndex;
extern const int menuItems;
extern const char* menuLabels[];

extern bool colonBlink;
extern unsigned long lastBlinkTime;
extern bool showHijri;
extern unsigned long lastDateToggle;
extern int beepCount;
extern unsigned long lastBeepCheck;

// ─── AT24C32 EEPROM pada modul DS3231 (ZS-042) ────────
// Kongsi bas I2C dengan RTC (0x68) & OLED. Tukar jika A0–A2 lain.
#define RTC_MODULE_EEPROM_ADDR  0x57

// ─── Solat struct ─────────────────────────────────────
struct SolatTime {
  char fajr[6];
  char syuruk[6];
  char dhuhr[6];
  char asr[6];
  char maghrib[6];
  char isha[6];
};

extern char savedZone[8];
extern SolatTime todaySolat;
extern bool solatLoaded;

extern int displayLayout;

extern bool dfPlayerReady;
/** true hanya jika modul lapor ada fail MP3 pada SD (readFileCounts > 0) */
extern bool dfPlayerSdOk;
/** Bilangan fail yang dilapor modul; -1 = ralat/tiada maklumat */
extern int  dfPlayerSdFileCount;

extern unsigned long lastSolatRetry;
/** Sela minimum sebelum cuba load/fetch solat semula (ms) */
#define SOLAT_RETRY_MS 5000UL

// ─── Hijri Calendar Bitmap ────────────────────────────
#define HIJRI_START_YEAR  1420
#define HIJRI_START_MONTH 8
#define HIJRI_START_DAY   25
#define HIJRI_DAY_OFFSET  23
#define HIJRI_JDN_REF     2451516L

static const uint8_t HIJRI_DATA[] PROGMEM = {
  0x2B,0x75,0xA5,0xB6,0x54,0xA7,0x6A,0x55,0xD5,0xAA,
  0x5C,0xA5,0x6D,0xD4,0x95,0xDA,0x52,0x5D,0xAA,0x4D,
  0xD5,0xAA,0x6A,0x95,0xB6,0x52,0x57,0x6A,0x4B,0x76,
  0xC9,0x36,0x65,0xAB,0xAC,0x56,0xD9,0x4A,0x5D,0xA9,
  0x2D,0xD5,0x25,0xBB,0xA4,0x9B,0xB2,0x55,0xD5,0x2A,
  0x6D,0xA5,0xB6,0x54,0x97,0xF4,0x92,0x6E,0xD2,0x56,
  0x69,0xAB,0xB4,0x95,0xDA,0x52,0x5D,0xD2,0x2B,0xBA,
  0x49,0x5B,0xA9,0xAB,0xB4,0x95,0x5A,0x4B,0x6D,0xA9,
  0x36,0xE9,0x16,0xED,0xA4,0xAE,0xD4,0x96,0x6A,0x4B,
  0xB5,0xA5,0xDA,0xA4,0x9B,0xB4,0x93,0xBA,0x52,0x5B,
  0xAA,0x4D,0xB5,0xAA,0x6A,0x55,0x6D,0xD2,0x56,0xEA,
  0x4A,0x6D,0xA9,0x2E,0xD5,0xAA,0x6A,0x55,0xB5,0x4A,
  0x5B,0xA9,0x2B,0xB5,0x25,0xBB,0x54,0x9B,0xAA,0x55,
  0xD5,0x2A,0x6D,0xA5,0xAE,0xD4,0x96,0xEC,0x92,0x5D,
  0xD2,0x55,0xD9,0x2A,0x6D,0x95,0xB6,0x52,0x5B,0xB2,
  0x4B,0x7A,0x49,0x37,0x29
};