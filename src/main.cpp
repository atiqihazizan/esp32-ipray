#include "config.h"
#include "secrets.h"
#include "display.h"
#include "utils.h"
#include "app_fsm.h"
#include "buttons.h"
#include "sound.h"
#include "solat.h"
#include <WiFi.h>
#include <Wire.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// RTC_DS1307       rtc;
RTC_DS3231       rtc;
Preferences      preferences;

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

// ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  appFsmInit();
  initDfPlayer();

  pinMode(BTN_MENU, INPUT_PULLUP);
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SET,  INPUT_PULLUP);
  pinMode(BTN_RET,  INPUT_PULLUP);
  pinMode(BTN_LAYOUT, INPUT_PULLUP);
  pinMode(DFPLAYER_BUSY_PIN, INPUT_PULLUP);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    Serial.println("[oled] not found");

  if (!rtc.begin())
    Serial.println("[rtc] not found");

  // showInitializing(1);
  // delay(300);
  
  showInitializing(1);
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

  if (dfPlayerReady) {
    DateTime now = rtc.now();
    speakTime(now.hour(), now.minute());
  }

  Serial.println("[setup] selesai");
}

// ─────────────────────────────────────────────────────
void loop() {
  handleSolatRetry();
  handleButtons();

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

  handleSound();

  appFsmUpdate();

  delay(100);
}