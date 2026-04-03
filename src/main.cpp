#include "config.h"
#include "secrets.h"
#include "display.h"
#include "utils.h"
#include "solat.h"
#include <WiFi.h>
#include <Wire.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// RTC_DS1307       rtc;
RTC_DS3231       rtc;
Preferences      preferences;

// ─── DFPlayer Mini ────────────────────────────────────
HardwareSerial dfSerial(2);  // UART2 untuk DFPlayer
DFRobotDFPlayerMini dfPlayer;
bool dfPlayerReady   = false;
bool dfPlayerSdOk    = false;
int  dfPlayerSdFileCount = -1;

const char *daysOfWeek[]   = {"Ahad","Isnin","Selasa","Rabu","Khamis","Jumaat","Sabtu"};
const char *monthsOfYear[] = {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const char *monthsOfYearFull[] = {"","Januari","Februari","Mac","April","Mei","Jun","Julai","Ogos","September","Oktober","November","Disember"};
const char *solatW[]       = {"Subuh","Syuruk","Zohor","Asar","Maghrib","Isyak"};
const char *hijriOfYear[]  = {"","Mrm","Saf","R.Aw","R.Ak","J.Aw","J.Ak","Rjb","Sya","Rmd","Syw","ZuQ","ZuH"};
const char *hijriOfYearFull[] = {"","Muharram","Safar","Rabiulawal","Rabiulakhir","Jamadilawal","Jamadilakhir","Rejab","Syaaban","Ramadan","Syawal","Zulkaedah","Zulhijjah"};
const char *menuLabels[]   = {"1. Takwim","2. Date","3. Time","4. WiFi"};

State     currentState  = HOME;
int       menuIndex     = 0;
const int menuItems     = 4;

bool          colonBlink     = true;
unsigned long lastBlinkTime  = 0;
unsigned long lastBeepCheck  = 0;
int           beepCount      = 0;
bool          showHijri      = false;
unsigned long lastDateToggle = 0;

unsigned long lastSolatRetry = 0;

int displayLayout = DISPLAY_LAYOUT;

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

/** Nada ringkas pada buzzer pasif (kemudian pin boleh digunakan semula). */
static void buzzTone(int freqHz, int durationMs) {
  ledcSetup(BUZZER_CHANNEL, freqHz, BUZZER_RES);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 200);
  delay(durationMs);
  ledcDetachPin(BUZZER_PIN);
}

// ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // ── Setup DFPlayer Mini ───────────────────────────
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
    dfPlayerReady = false;
    dfPlayerSdOk    = false;
    dfPlayerSdFileCount = -1;
  }

  pinMode(BTN_MENU, INPUT_PULLUP);
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SET,  INPUT_PULLUP);
  pinMode(BTN_RET,  INPUT_PULLUP);
  pinMode(BTN_LAYOUT, INPUT_PULLUP);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    Serial.println("[oled] not found");

  if (!rtc.begin())
    Serial.println("[rtc] not found");

  showInitializing(1);
  delay(300);
  
  showInitializing(2);
  connectWiFi();
  
  showInitializing(3);
  syncRTCwithNTP();

  showInitializing(4);
  if (preferences.begin("cfg", true)) {
    String z = preferences.getString("zone", SOLAT_ZONE);
    displayLayout = preferences.getInt("layout", DISPLAY_LAYOUT);
    preferences.end();
    if (z.length() > 0) z.toCharArray(savedZone, sizeof(savedZone));
  }

  strcpy(savedZone, SOLAT_ZONE);
  loadSolatFromPrefs(savedZone);
  
  Serial.printf("[layout] loaded layout %d from preferences\n", displayLayout);

  if (!solatLoaded && WiFi.status() == WL_CONNECTED){
    showInitializing(5);
    fetchAndSaveSolat(savedZone);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor((SCREEN_WIDTH - 30) / 2, (SCREEN_HEIGHT / 2) - 4);
  display.print("Ready!");
  display.display();

  static unsigned long lastBuzzPlay = 0;
  lastBuzzPlay = millis();
  buzzTone(BUZZER_ALERT_HZ, BUZZER_ALERT_MS);

  if (dfPlayerReady)
    playSound(TRACK_SD_BEEP, 600);

  Serial.println("[setup] selesai");
}

// ─────────────────────────────────────────────────────
void loop() {
  handleSolatRetry();
  handleButtons();
  handleBlink();

  switch (currentState) {
    case HOME:       drawHome(displayLayout);           break;
    case MENU:       drawMenu();                        break;
    case SET_TAKWIM: drawTakwim(); handleTakwimInput(); break;
    case SET_DATE:   drawSubMenu("DATE");               break;
    case SET_TIME:   drawSubMenu("TIME");               break;
    case SET_WIFI:   drawSubMenu("WIFI");               break;
    case SLEEP:
      display.clearDisplay();
      display.display();
      break;
  }

  handleBuzzer();

  delay(100);
}