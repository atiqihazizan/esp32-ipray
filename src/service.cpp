#include "service.h"
#include "config.h"
#include "secrets.h"
#include "utils.h"
#include "solat.h"
#include "app_fsm.h"
#include <WiFi.h>
#include <time.h>

// ════════════════════════════════════════════════════════════════════════
//  WiFi — state machine
// ════════════════════════════════════════════════════════════════════════
enum WiFiSvc : uint8_t {
  WF_IDLE = 0,    // belum cuba sambung
  WF_CONNECTING,  // tunggu WL_CONNECTED
  WF_CONNECTED,   // disambung, pantau putus
  WF_WAIT_RETRY,  // cooling-down sebelum cuba semula
};

static WiFiSvc       wfState = WF_IDLE;
static unsigned long wfTimer = 0;

// ════════════════════════════════════════════════════════════════════════
//  NTP — state machine (non-blocking; poll getLocalTime selepas configTime)
// ════════════════════════════════════════════════════════════════════════
enum NtpSvc : uint8_t { NTP_IDLE = 0, NTP_WAITING, NTP_DONE };
static NtpSvc        ntpState = NTP_IDLE;
static unsigned long ntpTimer = 0;
static uint8_t       ntpRetry = 0;

// ════════════════════════════════════════════════════════════════════════
//  SD / DFPlayer — state machine
// ════════════════════════════════════════════════════════════════════════
enum SdSvc : uint8_t {
  SD_UNKNOWN = 0,  // status belum diketahui / tidak sedia
  SD_READY,        // dfPlayerReady == true
  SD_WAIT_REINIT,  // jeda sebelum cuba reinit
};

static SdSvc         sdState = SD_UNKNOWN;
static unsigned long sdTimer = 0;

// ════════════════════════════════════════════════════════════════════════
//  Startup speak — tangguh enqueue sehingga SD sedia
// ════════════════════════════════════════════════════════════════════════
static bool pendingStartupSpeak = false;
static bool startupSpeakDone    = false;

// ════════════════════════════════════════════════════════════════════════
//  Solat retry — dipindah dari utils.cpp + main.cpp
// ════════════════════════════════════════════════════════════════════════
static unsigned long solatRetryTimer = 0;


// ────────────────────────────────────────────────────────────────────────
//  connectWiFi() — dipindah dari utils.cpp (blocking, untuk setup())
// ────────────────────────────────────────────────────────────────────────
void connectWiFi() {
  WiFi.begin((char*)WIFI_SSID, (char*)WIFI_PASSWORD);
  Serial.print("[wifi] connecting");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000UL) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED)
    Serial.printf("[wifi] connected, IP: %s\n", WiFi.localIP().toString().c_str());
  else
    Serial.println("[wifi] timeout — service akan cuba semula");
}

// ────────────────────────────────────────────────────────────────────────
//  syncRTCwithNTP() — dipindah dari utils.cpp (blocking, untuk setup())
// ────────────────────────────────────────────────────────────────────────
void syncRTCwithNTP() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ntp] skip — tiada WiFi");
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
    Serial.println("[ntp] gagal, guna RTC sedia ada");
    return;
  }
  rtc.adjust(DateTime(
    timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
    timeinfo.tm_hour,        timeinfo.tm_min,      timeinfo.tm_sec
  ));
  Serial.printf("[ntp] RTC dikemas kini: %04d-%02d-%02d %02d:%02d:%02d\n",
    timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  ntpState = NTP_DONE; // tandakan sudah selesai supaya service tidak ulang
}


// ────────────────────────────────────────────────────────────────────────
//  Internal: NTP async (untuk reconnect WiFi semasa operasi)
// ────────────────────────────────────────────────────────────────────────
static void svcNtpStart() {
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  ntpTimer = millis();
  ntpRetry = 0;
  ntpState = NTP_WAITING;
  Serial.println("[svc/ntp] async configTime dicetuskan");
}

static void svcNtpTick() {
  if (ntpState != NTP_WAITING) return;
  if (millis() - ntpTimer < 500UL) return;
  ntpTimer = millis();

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.adjust(DateTime(
      timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
      timeinfo.tm_hour,        timeinfo.tm_min,      timeinfo.tm_sec
    ));
    Serial.printf("[svc/ntp] RTC disegerakkan: %04d-%02d-%02d %02d:%02d:%02d\n",
      timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    ntpState = NTP_DONE;
  } else if (++ntpRetry >= 20) {
    Serial.println("[svc/ntp] async sync gagal");
    ntpState = NTP_DONE;
  }
}


// ────────────────────────────────────────────────────────────────────────
//  WiFi tick
// ────────────────────────────────────────────────────────────────────────
static void wifiTick() {
  const unsigned long now = millis();
  switch (wfState) {

    case WF_IDLE:
      // setup() sudah connect — sync state dengan status semasa
      wfState = (WiFi.status() == WL_CONNECTED) ? WF_CONNECTED : WF_WAIT_RETRY;
      wfTimer = now;
      break;

    case WF_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[svc/wifi] disambung semula: %s\n",
          WiFi.localIP().toString().c_str());
        wfState = WF_CONNECTED;
        // Re-sync NTP & solat selepas reconnect
        if (ntpState == NTP_DONE) svcNtpStart();
        if (!solatLoaded && strlen(savedZone) > 0)
          fetchAndSaveSolat(savedZone);
      } else if (now - wfTimer >= 15000UL) {
        Serial.println("[svc/wifi] timeout, cuba lagi nanti");
        wfState = WF_WAIT_RETRY;
        wfTimer = now;
      }
      break;

    case WF_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[svc/wifi] terputus, jadual semula");
        wfState = WF_WAIT_RETRY;
        wfTimer = now;
      }
      break;

    case WF_WAIT_RETRY:
      if (now - wfTimer >= 30000UL) {
        Serial.println("[svc/wifi] cuba sambung semula...");
        WiFi.begin((char*)WIFI_SSID, (char*)WIFI_PASSWORD);
        wfTimer = now;
        wfState = WF_CONNECTING;
      }
      break;
  }
}


// ────────────────────────────────────────────────────────────────────────
//  SD / DFPlayer tick
// ────────────────────────────────────────────────────────────────────────
static void sdTick() {
  const unsigned long now = millis();
  switch (sdState) {

    case SD_UNKNOWN:
      if (dfPlayerReady) {
        sdState = SD_READY;
        Serial.println("[svc/sd] sedia");
      } else if (now - sdTimer >= 5000UL) {
        // Belum sedia selepas 5 s — cuba init semula
        Serial.println("[svc/sd] cuba init semula...");
        initDfPlayer();
        sdTimer = now;
        if (dfPlayerReady) {
          sdState = SD_READY;
          Serial.println("[svc/sd] init semula berjaya");
        }
        // Jika masih gagal, kekal SD_UNKNOWN — timer akan cuba lagi
      }
      break;

    case SD_READY:
      if (!dfPlayerReady) {
        Serial.println("[svc/sd] SD hilang!");
        sdState = SD_WAIT_REINIT;
        sdTimer = now;
      }
      break;

    case SD_WAIT_REINIT:
      if (now - sdTimer >= 3000UL) {
        Serial.println("[svc/sd] reinit...");
        initDfPlayer();
        sdTimer = now;
        if (dfPlayerReady) {
          sdState = SD_READY;
          Serial.println("[svc/sd] reinit OK");
        } else {
          // Kembali ke UNKNOWN supaya timer 5 s bermula semula
          sdState = SD_UNKNOWN;
          Serial.println("[svc/sd] reinit gagal, cuba lagi");
        }
      }
      break;
  }
}


// ────────────────────────────────────────────────────────────────────────
//  Startup speak tick — enqueue hanya apabila SD sudah sedia
// ────────────────────────────────────────────────────────────────────────
static void startupSpeakTick() {
  if (startupSpeakDone || !pendingStartupSpeak) return;
  if (!dfPlayerReady) return;  // tunggu SD sedia

  DateTime now = rtc.now();
  appFsmEnqSpeak(now.hour(), now.minute());
  startupSpeakDone    = true;
  pendingStartupSpeak = false;
  Serial.printf("[svc] startup speak: %02d:%02d\n", now.hour(), now.minute());
}


// ────────────────────────────────────────────────────────────────────────
//  Solat retry tick — dipindah dari utils.cpp handleSolatRetry()
// ────────────────────────────────────────────────────────────────────────
static void solatRetryTick() {
  if (solatLoaded) return;
  if (strlen(savedZone) == 0) return;
  if (millis() - solatRetryTimer < (unsigned long)SOLAT_RETRY_MS) return;

  solatRetryTimer = millis();
  loadSolatFromPrefs(savedZone);

  if (!solatLoaded) {
    if (WiFi.status() == WL_CONNECTED)
      fetchAndSaveSolat(savedZone);
    else
      Serial.println("[svc/solat] tiada WiFi, tangguh");
  }
}


// ════════════════════════════════════════════════════════════════════════
//  Public API
// ════════════════════════════════════════════════════════════════════════

void serviceInit() {
  // Sync state WiFi dengan status semasa (setup() sudah connect secara blocking)
  wfState = (WiFi.status() == WL_CONNECTED) ? WF_CONNECTED : WF_IDLE;
  wfTimer = millis();

  // Jika NTP belum disegerakkan oleh syncRTCwithNTP(), mulakan async
  // (ntpState == NTP_DONE bermaksud syncRTCwithNTP() sudah berjaya)
  if (ntpState == NTP_IDLE) svcNtpStart();

  sdState  = dfPlayerReady ? SD_READY : SD_UNKNOWN;
  sdTimer  = millis();

  solatRetryTimer  = 0;
  startupSpeakDone = false;

  Serial.printf("[svc] init — wifi:%s sd:%s\n",
    wfState == WF_CONNECTED ? "OK" : "putus",
    sdState  == SD_READY    ? "OK" : "tidak sedia");
}

void serviceTick() {
  wifiTick();
  svcNtpTick();
  sdTick();
  startupSpeakTick();  // ← betulkan masalah startup speak
  solatRetryTick();
}

void serviceRequestStartupSpeak() {
  pendingStartupSpeak = true;
  startupSpeakDone    = false;
}