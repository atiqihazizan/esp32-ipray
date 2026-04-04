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

// ─── DFPlayer Mini MP3-TF-16P ────────────────────────
// New pins (alternative):
#define DFPLAYER_RX       25  // ESP32 RX → DFPlayer TX
#define DFPLAYER_TX       26  // ESP32 TX → DFPlayer RX
#define DFPLAYER_BUSY_PIN 32  // ESP32 ← kaki BUSY modul (biasanya GPIO input + pull-up)

/** 1 = pin LOW semasa main audio; 0 = HIGH semasa main (terbalik mengikut modul) */
#define DFPLAYER_BUSY_ACTIVE_LOW  1

/** Lepas hantar play, jangan anggap “siap” dalam tempoh ini (ms) — tepi UART */
#define DFPLAYER_BUSY_IGNORE_MS   120

// Hanya untuk appFsmEnqPlay(): 0=mp3/ 1=ROOT 2=folder jam(01) 3=folder minit(02)
// Untuk kod baharu, guna appFsmEnqRoot / EnqMp3 / EnqFolder (bukan bergantung macro ini).
#define DFPLAYER_PLAY_MODE   2

// 0 = UART tanpa ACK (sesetengah modul lebih stabil); 1 = dengan ACK
#define DFPLAYER_USE_ACK     0

/**
 * speakTime(): DFPlayer playFolder — folder `01` jam, `02` minit.
 * 01/: 001–012.mp3 — RTC 24j → 12j; jam 00 & 12 → 012.mp3 (indeks 12).
 * 02/: 001–060.mp3 — minit 00 → 060.mp3 (60); :01–:59 → 001–059 (1–59).
 */
#define DFPLAYER_SPEAK_FOLDER_HOUR    1
#define DFPLAYER_SPEAK_FOLDER_MINUTE  2
/** speakTime: tunggu BUSY idle; nilai ini = had masa maks jika pin tidak bertindak */
#define SPEAK_TIME_MS_AFTER_HOUR      8000
#define SPEAK_TIME_MS_AFTER_MINUTE    8000

/** ROOT play(n) — fail di punca SD: 001.mp3 … 006.mp3 */
#define TRACK_SD_BEEP        1  // beep (startup, layout, dll. — bukan masuk waktu)
#define TRACK_SD_NOTIFY      2  // notify (30s sebelum solat)
#define TRACK_SD_SHORT_A     3  // QUARTER_CHIME_MODE_SHORT_RANDOM: pilih rawak A atau B dalam sound.cpp
#define TRACK_SD_SHORT_B     4
#define TRACK_SD_AZAN        5  // azan penuh — satu fail MP3 (main sekali; beep dulu jika AZAN_USE_BEEP_PREAMBLE)
#define TRACK_SD_FULL_MUSIC  6  // muzik penuh — dipakai pada :00 jika HOURLY_CHIME_MODE=0

/**
 * Tepat jam penuh (:00):
 * 0 = main TRACK_SD_FULL_MUSIC (muzik)
 * 1 = speakTime(jam, minit) — folder DFPLAYER_SPEAK_FOLDER_HOUR / MINUTE pada SD
 */
#define HOURLY_CHIME_MODE_FULL_MUSIC     0
#define HOURLY_CHIME_MODE_SPEAK_FOLDERS  1
#define HOURLY_CHIME_MODE                HOURLY_CHIME_MODE_SPEAK_FOLDERS

// Pilihan Mod Suku Jam (15, 30, 45 minit)
#define QUARTER_CHIME_MODE_SPEAK_FOLDERS 1
#define QUARTER_CHIME_MODE_SHORT_RANDOM  2
#define QUARTER_CHIME_MODE_SPECIFIC_TRACK 3

// Tetapkan mod yang anda mahu gunakan untuk suku jam:
#define QUARTER_CHIME_MODE QUARTER_CHIME_MODE_SPEAK_FOLDERS

// Jika guna mod SPECIFIC_TRACK, tetapkan nombor treknya:
#define TRACK_SD_QUARTER_MUSIC 3 // Contoh: fail 0010.mp3

/**
 * Cara main bila masuk waktu solat:
 * 1 = beep (trek 1) diluang AZAN_PLAY_COUNT kali, lalu azan penuh sekali.
 * 0 = azan penuh sahaja — AZAN_PLAY_COUNT tidak dipakai.
 */
#define AZAN_USE_BEEP_PREAMBLE  0

/** Bilangan beep (trek 1) sebelum azan — hanya jika AZAN_USE_BEEP_PREAMBLE = 1 */
#define AZAN_PLAY_COUNT   5

// Buzz + DFPlayer berkala (ms); 0 = matikan — rujukan trek jika diperlukan
#define PLAY_SOUND_INTERVAL_MS    3000
#define PLAY_SOUND_INTERVAL_TRACK TRACK_SD_BEEP

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