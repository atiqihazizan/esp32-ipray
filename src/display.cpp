#include "display.h"
#include "config.h"
#include "utils.h"
#include "solat.h"
#include <WiFi.h>
#include <RTClib.h>

void toggleHijriDisplay()
{
  if (millis() - lastDateToggle >= 2000)
  {
    showHijri = !showHijri;
    lastDateToggle = millis();
  }
}

void getActiveSolatInfo(char* label, char* time)
{
  if (solatLoaded)
  {
    int activeSolat = getCurrentSolatIndex(rtc.now());
    if (activeSolat >= 0)
    {
      getSolatLabel(activeSolat, label);
      getSolatTime(activeSolat, todaySolat, time);
    }
    else
    {
      int nextIdx = getNextSolatIndex(rtc.now());
      getSolatLabel(nextIdx, label);
      getSolatTime(nextIdx, todaySolat, time);
    }
  }
  else
  {
    strcpy(label, solatW[0]);
    strcpy(time, "");
  }
}

void initDisplay()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void drawClock(int x, int y)
{
  DateTime now = rtc.now();
  int h12 = now.hour() % 12;
  if (h12 == 0)
    h12 = 12;
  display.setTextSize(2);
  display.setCursor(x, y);
  if (colonBlink)
    display.printf("%2d:%02d", h12, now.minute());
  else
    display.printf("%2d %02d", h12, now.minute());
}

void drawClock2(int x, int y)
{
  DateTime now = rtc.now();
  int h12 = now.hour() % 12;
  if (h12 == 0) h12 = 12;
  char hourStr[3];
  bool use24h = true;
  sprintf(hourStr, "%2d", h12);   // contoh: " 3" atau "12"
  if (use24h) {
    sprintf(hourStr, "%02d", now.hour());
  }
  char minStr[3];
  sprintf(minStr, "%02d", now.minute());
  const char* ampm = now.hour() < 12 ? "AM" : "PM";
  const int charW1 = 6;
  const int charW2 = 12;
  const int charW3 = 18;
  int hourW = strlen(hourStr) * charW3;
  int minW  = strlen(minStr) * charW1;


  // [jam][minit]
  display.setTextSize(3);
  display.setCursor(x, y);
  display.print(hourStr);
  
  display.setTextSize(2);
  display.setCursor((int)rightToLen(ampm, charW2, 2), y);   // minit kecil, sedikit ke bawah
  display.print(minStr);
  
  display.setTextSize(1);
  // Colon blink (saiz ikut jam)
  display.setCursor(x + hourW, y + 18);
  // display.print(colonBlink ? ":" : " ");
  char secStr[3];
  sprintf(secStr, "%02d", now.second());
  display.setCursor(x + hourW, y + 18);
  display.print(secStr);
  
  if (!use24h){
    display.setTextSize(1);
    // AM/PM diratakan ke kanan seperti paparan asal layout-2.
    display.setCursor((int)rightToLen(ampm, charW1, 2), y + 18);
    display.print(ampm);
  }
}

void drawHome(int layout)
{
  if (layout == 2)
  {
    drawHomeLayout2();
    return;
  }

  DateTime now = rtc.now();
  initDisplay();

  const int dateRow = 2;
  const int charW1 = 6;
  const int charW2 = 12;
  const int solatRow = 38;
  const int clockRow = 48;
  const double padLeft = 2;
  const double padRight = 2;

  char dayShort[7];
  strncpy(dayShort, daysOfWeek[now.dayOfTheWeek()], 6);
  dayShort[6] = '\0';
  display.setTextSize(2);
  display.setCursor(padLeft, dateRow);
  display.print(dayShort);

  toggleHijriDisplay();

  // char dateRight[22];
  // if (!showHijri)
  // {
  //   sprintf(dateRight, "%02d %s %04d", now.day(), monthsOfYear[now.month()], now.year());
  // }
  // else
  // {
  //   int hDay, hMonth, hYear;
  //   calculateHijri(now.year(), now.month(), now.day(), hDay, hMonth, hYear);
  //   sprintf(dateRight, "%02d %s %04d", hDay, hijriOfYear[hMonth], hYear);
  // }
  // display.setTextSize(1);
  // display.setCursor((int)rightToLen(dateRight, charW1, (int)padRight), dateRow);
  // display.print(dateRight);
  
  // drawDatePanelStyle2(now, padRight, dateRow);

  int dateDay = now.day();
  const char* monthName = monthsOfYearFull[now.month()];
  int dateYear = now.year();
  if (showHijri)
  {
    int hDay, hMonth, hYear;
    calculateHijri(now.year(), now.month(), now.day(), hDay, hMonth, hYear);
    dateDay = hDay;
    monthName = hijriOfYearFull[hMonth];
    dateYear = hYear;
  }
  char dayNum[3];
  sprintf(dayNum, "%02d", dateDay);
  display.setTextSize(2);
  display.setCursor((int)rightToLen(dayNum, charW2, (int)padRight), dateRow);
  display.print(dayNum);

  //--------------------------------
  char monthShort[7];
  strncpy(monthShort, monthName, 6);
  monthShort[6] = '\0';
  display.setTextSize(2);
  display.setCursor(padLeft, dateRow + 18);
  display.print(monthShort);

  char yearLine[5];
  sprintf(yearLine, "%04d", dateYear);
  display.setCursor((int)rightToLen(yearLine, charW2, (int)padRight), dateRow + 18);
  display.print(yearLine);
  //--------------------------------
  

  display.setTextSize(1);
  char solatLabel[10], solatTime[6];
  getActiveSolatInfo(solatLabel, solatTime);

  display.setCursor(padLeft, solatRow);
  display.print(solatLabel);

  const char *ampm = now.hour() < 12 ? "AM" : "PM";
  display.setCursor(rightToLen(ampm, charW1, padRight), solatRow);
  display.print(ampm);

  display.setTextSize(2);
  if (solatLoaded && strlen(solatTime) > 0) {
    char solatTime12[6];
    to12h(solatTime, solatTime12);
    display.setCursor(padLeft, clockRow);
    display.print(solatTime12);
  }
  drawClock((int)rightToLen("00:00", charW2, padRight), clockRow);

  display.display();
}

void drawDateInfo(int day, const char* month, int year, int yDateMonth, int yYear, int textSize, const double padRight)
{
  const int charW1 = 6;
  const int charW2 = 12;
  
  char dateMonth[16];
  sprintf(dateMonth, "%02d %s", day, month);
  // sprintf(dateMonth, "%02d %s %04d", day, month, year);
  display.setTextSize(textSize);
  int charWidth = (textSize == 2) ? charW2 : charW1;
  display.setCursor((int)rightToLen(dateMonth, charWidth, (int)padRight), yDateMonth);
  display.print(dateMonth);
  
  char yearStr[5];
  sprintf(yearStr, "%04d", year);
  display.setTextSize(1);
  display.setCursor((int)rightToLen(yearStr, charW1, (int)padRight), yYear);
  display.print(yearStr);
}

// blinkMode: 0=none, 1=highlight solid, 2=warning blink (500ms), 3=azan blink (200ms)
void drawSolatRow(const char* label, const char* time24, int yPos, double padLeft, int blinkMode, bool use12h)
{
  char labelShort[6];
  strncpy(labelShort, label, 5);
  labelShort[5] = '\0';

  char timeToShow[6];
  if (time24 && strlen(time24) > 0)
  {
    if (use12h)
      to12h(time24, timeToShow);
    else
    {
      strncpy(timeToShow, time24, 5);
      timeToShow[5] = '\0';
    }
  }
  else
  {
    strcpy(timeToShow, "--:--");
  }

  bool showBox = false;
  if (blinkMode == 1)
    showBox = true;
  else if (blinkMode == 2)
    showBox = (millis() / 500) % 2 == 0;
  else if (blinkMode == 3)
    showBox = (millis() / 200) % 2 == 0;

  if (showBox)
  {
    int timeX = 36;
    int contentWidth = (timeX - (int)padLeft) + (strlen(timeToShow) * 6);
    int boxWidth = contentWidth + 1;
    display.fillRect(padLeft, yPos - 1, boxWidth, 9, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  }

  display.setCursor(padLeft, yPos);
  display.print(labelShort);
  display.setCursor(36, yPos);
  display.print(timeToShow);

  if (showBox)
    display.setTextColor(SSD1306_WHITE);
}

void drawDatePanelStyle1(const DateTime& now, double padRight)
{
  const int charW1 = 6;
  const int charW3 = 18;
  const int rowDate = 14;

  int dateDay = now.day();
  const char* monthName = monthsOfYear[now.month()];
  int dateYear = now.year();
  if (showHijri)
  {
    int hDay, hMonth, hYear;
    calculateHijri(now.year(), now.month(), now.day(), hDay, hMonth, hYear);
    dateDay = hDay;
    monthName = hijriOfYear[hMonth];
    dateYear = hYear;
  }

  char dayNum[3];
  sprintf(dayNum, "%02d", dateDay);
  int dayW = strlen(dayNum) * charW3;
  int yearW = 4 * charW1;
  int xStart = SCREEN_WIDTH - yearW - (int)padRight - 1 - dayW;
  int xMonth = xStart + dayW + 1;

  display.setTextSize(3);
  display.setCursor(xStart, rowDate);
  display.print(dayNum);

  display.setTextSize(1);
  display.setCursor(xMonth, rowDate);
  display.print(monthName);

  char yearLine[5];
  sprintf(yearLine, "%04d", dateYear);
  display.setCursor((int)rightToLen(yearLine, charW1, (int)padRight), rowDate + 11);
  display.print(yearLine);
}
void drawDatePanelStyle2(const DateTime& now, double padRight, int rowDate)
{
  const int charW1 = 6;
  const int charW2 = 12;
  const int charW3 = 18;

  int dateDay = now.day();
  const char* monthName = monthsOfYear[now.month()];
  int dateYear = now.year();
  if (showHijri)
  {
    int hDay, hMonth, hYear;
    calculateHijri(now.year(), now.month(), now.day(), hDay, hMonth, hYear);
    dateDay = hDay;
    monthName = hijriOfYear[hMonth];
    dateYear = hYear;
  }

  char dayNum[3];
  sprintf(dayNum, "%02d", dateDay);
  int dayW = strlen(dayNum) * charW2;
  int yearW = 4 * charW1;
  int xStart = SCREEN_WIDTH - yearW - (int)padRight - 1 - dayW;
  int xMonth = xStart + dayW + 1;

  display.setTextSize(2);
  display.setCursor(xStart, rowDate);
  display.print(dayNum);

  display.setTextSize(1);
  display.setCursor(xMonth, rowDate);
  display.print(monthName);

  char yearLine[5];
  sprintf(yearLine, "%04d", dateYear);
  display.setCursor((int)rightToLen(yearLine, charW1, (int)padRight), rowDate + 11);
  display.print(yearLine);
}

void drawHomeLayout2()
{
  DateTime now = rtc.now();
  initDisplay();

  const int charW1 = 6;
  const int charW2 = 12;
  const double padLeft = 2;
  const double padRight = 2;
  const int yDayName = 2;

  toggleHijriDisplay();

  const char* dayLine = daysOfWeek[now.dayOfTheWeek()];
  display.setTextSize(1);
  display.setCursor((int)rightToLen(dayLine, charW1, (int)padRight), yDayName);
  display.print(dayLine);

  drawDatePanelStyle1(now, padRight);

  const int solatIndices[] = {0, 1, 2, 3, 4, 5};
  const int solatListStartY = 2;
  const int solatLineSpacing = 10;
  
  int activeSolatIdx = -1;
  int nextSolatIdx = -1;
  if (solatLoaded) {
    activeSolatIdx = getCurrentSolatIndex(now);
    if (activeSolatIdx < 0) {
      nextSolatIdx = getNextSolatIndex(now);
    }
  }

  int nowMin = now.hour() * 60 + now.minute();
  int nowSec = now.second();

  const char* solatTimes[] = {
    todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
    todaySolat.asr,  todaySolat.maghrib, todaySolat.isha
  };

  display.setTextSize(1);
  for (int i = 0; i < 6; i++)
  {
    int idx = solatIndices[i];
    int yPos = solatListStartY + (i * solatLineSpacing);

    char label[10], time[6];
    getSolatLabel(idx, label);

    int blinkMode = 0;
    if (solatLoaded)
    {
      getSolatTime(idx, todaySolat, time);
      int solatMin = timeToMinutes(solatTimes[idx]);
      int diff = solatMin - nowMin;

      if (diff == 0)
        blinkMode = 3;
      else if (diff == 1 && nowSec >= 28)
        blinkMode = 2;
      else if (idx == activeSolatIdx || idx == nextSolatIdx)
        blinkMode = 1;

      drawSolatRow(label, time, yPos, padLeft, blinkMode, true);
    }
    else
    {
      drawSolatRow(label, "--:--", yPos, padLeft, 0, true);
    }
  }

  char solatLabel[10], solatTime[6];
  getActiveSolatInfo(solatLabel, solatTime);

  drawClock2((int)rightToLen("00:00", charW2, padRight), 36);

  display.display();
}

void drawMenu()
{
  initDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("---- MAIN MENU ----");
  for (int i = 0; i < menuItems; i++)
  {
    if (i == menuIndex)
      display.print("> ");
    else
      display.print("  ");
    display.println(menuLabels[i]);
  }
  display.setCursor(0, 56);
  display.print("Sel:SET Back:RET");
  display.display();
}

void drawSubMenu(const char *title)
{
  initDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(title);
  display.setCursor(0, 20);
  display.println("(Coming soon)");
  display.setCursor(0, 56);
  display.print("Back:RET");
  display.display();
}

// ─── Senarai zon JAKIM ────────────────────────────────
const char *zoneList[] = {
    "WLY01", "WLY02",
    "JHR01", "JHR02", "JHR03", "JHR04",
    "KDH01", "KDH02", "KDH03", "KDH04",
    "KTN01", "KTN02",
    "MLK01",
    "NGS01", "NGS02",
    "PHG01", "PHG02", "PHG03",
    "PNG01",
    "PRK01", "PRK02", "PRK03", "PRK04", "PRK05", "PRK06", "PRK07",
    "SBH01", "SBH02", "SBH03", "SBH04", "SBH05", "SBH06", "SBH07", "SBH08", "SBH09",
    "SGR01", "SGR02", "SGR03",
    "SWK01", "SWK02", "SWK03", "SWK04", "SWK05", "SWK06", "SWK07", "SWK08", "SWK09", "SWK10",
    "TRG01", "TRG02", "TRG03", "TRG04",
    "PLS01"};
const int zoneCount = sizeof(zoneList) / sizeof(zoneList[0]);

static int zoneScroll = 0;
static int takwimSubMode = 0; // 0=senarai zon, 1=submenu
static int takwimSubIdx = 0;  // 0=Pilih Zon, 1=Sync Semula
static bool zoneInitDone = false;

static int findZoneIndex(const char *zone)
{
  for (int i = 0; i < zoneCount; i++)
  {
    if (strcmp(zoneList[i], zone) == 0)
      return i;
  }
  return 0;
}

// ─── drawTakwim ───────────────────────────────────────
void drawTakwim()
{
  initDisplay();
  display.setTextSize(1);

  if (WiFi.status() != WL_CONNECTED)
  {
    display.setCursor(0, 0);
    display.print("SET ZON SOLAT");
    display.setCursor(0, 20);
    display.print("Tiada Internet!");
    display.setCursor(0, 35);
    display.print("Sambung WiFi dulu.");
    display.setCursor(0, 56);
    display.print("Back:RET");
    display.display();
    return;
  }

  // ── Mode 0: senarai zon ───────────────────────────
  if (takwimSubMode == 0)
  {
    display.setCursor(0, 0);
    display.print("SET ZON SOLAT");

    for (int i = 0; i < 4; i++)
    {
      int idx = zoneScroll + i;
      if (idx >= zoneCount)
        break;
      display.setCursor(0, 12 + i * 11);
      display.print(idx == zoneScroll ? "> " : "  ");
      display.print(zoneList[idx]);
      if (strcmp(zoneList[idx], savedZone) == 0)
        display.print(" *");
    }

    display.setCursor(0, 56);
    display.print("UP/DN:skrol SET:pilih");

    // ── Mode 1: submenu pilih/sync ────────────────────
  }
  else
  {
    display.setCursor(0, 0);
    display.print("ZON: ");
    display.print(zoneList[zoneScroll]);

    display.setCursor(0, 16);
    display.print(takwimSubIdx == 0 ? "> " : "  ");
    display.print("Pilih Zon");

    display.setCursor(0, 28);
    display.print(takwimSubIdx == 1 ? "> " : "  ");
    display.print("Sync Semula");

    display.setCursor(0, 44);
    display.print("Aktif: ");
    display.print(savedZone);

    display.setCursor(0, 56);
    display.print("UP/DN sel  SET:ok  RET:bk");
  }

  display.display();
}

// ─── handleTakwimInput ────────────────────────────────
void handleTakwimInput()
{

  // ── Mode 0: navigasi senarai zon ──────────────────
  if (takwimSubMode == 0)
  {

    // Auto-scroll ke zone aktif semasa pertama masuk
    if (!zoneInitDone && strlen(savedZone) > 0)
    {
      zoneScroll = findZoneIndex(savedZone);
      zoneInitDone = true;
    }

    if (digitalRead(BTN_UP) == LOW)
    {
      delay(200);
      if (zoneScroll > 0)
        zoneScroll--;
    }
    if (digitalRead(BTN_DOWN) == LOW)
    {
      delay(200);
      if (zoneScroll < zoneCount - 1)
        zoneScroll++;
    }
    if (digitalRead(BTN_SET) == LOW)
    {
      delay(200);
      takwimSubMode = 1;
      takwimSubIdx = 0;
    }
    if (digitalRead(BTN_RET) == LOW)
    {
      delay(200);
      takwimSubMode = 0;
      zoneInitDone = false; // ← reset supaya next kali masuk, scroll ke zone aktif semula
      currentState = MENU;
    }

    // ── Mode 1: pilih/sync ────────────────────────────
  }
  else
  {

    if (digitalRead(BTN_UP) == LOW)
    {
      delay(200);
      takwimSubIdx = 0;
    }
    if (digitalRead(BTN_DOWN) == LOW)
    {
      delay(200);
      takwimSubIdx = 1;
    }
    if (digitalRead(BTN_SET) == LOW)
    {
      delay(200);
      display.clearDisplay();
      display.setCursor(0, 20);

      if (takwimSubIdx == 0)
      {
        // Pilih zon baru & fetch
        display.print("Memuat turun...");
        display.setCursor(0, 32);
        display.print(zoneList[zoneScroll]);
        display.display();
        fetchAndSaveSolat(zoneList[zoneScroll]);

        // Hardware sebenar — simpan zone ke NVS:
        if (preferences.begin("cfg", false)) {
          preferences.putString("zone", zoneList[zoneScroll]);
          preferences.end();
        }
      }
      else
      {
        // Sync semula — re-fetch zone aktif
        display.print("Sync semula...");
        display.setCursor(0, 32);
        display.print(savedZone);
        display.display();
        solatLoaded = false;
        fetchAndSaveSolat(savedZone);
      }

      zoneInitDone = false; // ← reset supaya selepas pilih zone baru, scroll ke zone terkini
      takwimSubMode = 0;
      currentState = HOME;
    }

    if (digitalRead(BTN_RET) == LOW)
    {
      delay(200);
      takwimSubMode = 0; // kembali ke senarai zon
    }
  }
}