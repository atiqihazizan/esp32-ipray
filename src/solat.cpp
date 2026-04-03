#include "solat.h"
#include "config.h"
#include <string.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

char savedZone[8] = "";
SolatTime todaySolat;
bool solatLoaded = false;

// ─── AT24C32 (memori pada modul DS3231, contoh ZS-042) ─
static constexpr uint16_t kSolatEepromMagic = 0x6950;
static constexpr uint8_t  kSolatEepromVer   = 1;
static constexpr uint16_t kSolatEepromAddr  = 0;

struct SolatRtcEepromPackage {
  uint16_t magic;
  uint8_t  version;
  char     zone[8];
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
  char     fajr[6];
  char     syuruk[6];
  char     dhuhr[6];
  char     asr[6];
  char     maghrib[6];
  char     isha[6];
} __attribute__((packed));

static bool rtcEepromWriteBytes(uint16_t addr, const uint8_t* buf, size_t len) {
  for (size_t off = 0; off < len; ) {
    uint16_t a = static_cast<uint16_t>(addr + off);
    size_t pageRoom = 32 - (a % 32);
    size_t chunk = pageRoom;
    if (chunk > len - off) chunk = len - off;
    if (chunk > 16) chunk = 16;
    Wire.beginTransmission(RTC_MODULE_EEPROM_ADDR);
    Wire.write(static_cast<uint8_t>(a >> 8));
    Wire.write(static_cast<uint8_t>(a & 0xFF));
    for (size_t k = 0; k < chunk; k++) Wire.write(buf[off + k]);
    if (Wire.endTransmission() != 0) return false;
    delay(10);
    off += chunk;
  }
  return true;
}

static bool rtcEepromReadBytes(uint16_t addr, uint8_t* buf, size_t len) {
  for (size_t off = 0; off < len; ) {
    uint16_t a = static_cast<uint16_t>(addr + off);
    size_t chunk = len - off;
    if (chunk > 32) chunk = 32;
    Wire.beginTransmission(RTC_MODULE_EEPROM_ADDR);
    Wire.write(static_cast<uint8_t>(a >> 8));
    Wire.write(static_cast<uint8_t>(a & 0xFF));
    if (Wire.endTransmission(false) != 0) return false;
    size_t req = Wire.requestFrom(static_cast<int>(RTC_MODULE_EEPROM_ADDR), static_cast<int>(chunk));
    if (req != chunk) return false;
    for (size_t k = 0; k < chunk; k++) buf[off + k] = static_cast<uint8_t>(Wire.read());
    off += chunk;
  }
  return true;
}

static void fillEepromPackage(SolatRtcEepromPackage& p, const char* zone,
    uint16_t y, uint8_t mo, uint8_t d, const SolatTime& s) {
  memset(&p, 0, sizeof(p));
  p.magic = kSolatEepromMagic;
  p.version = kSolatEepromVer;
  strncpy(p.zone, zone ? zone : "", sizeof(p.zone) - 1);
  p.year = y;
  p.month = mo;
  p.day = d;
  memcpy(p.fajr,    s.fajr,    sizeof(p.fajr));
  memcpy(p.syuruk,  s.syuruk,  sizeof(p.syuruk));
  memcpy(p.dhuhr,   s.dhuhr,   sizeof(p.dhuhr));
  memcpy(p.asr,     s.asr,     sizeof(p.asr));
  memcpy(p.maghrib, s.maghrib, sizeof(p.maghrib));
  memcpy(p.isha,    s.isha,    sizeof(p.isha));
}

static void packageToSolat(const SolatRtcEepromPackage& p, SolatTime& s) {
  memcpy(s.fajr,    p.fajr,    sizeof(s.fajr));
  memcpy(s.syuruk,  p.syuruk,  sizeof(s.syuruk));
  memcpy(s.dhuhr,   p.dhuhr,   sizeof(s.dhuhr));
  memcpy(s.asr,     p.asr,     sizeof(s.asr));
  memcpy(s.maghrib, p.maghrib, sizeof(s.maghrib));
  memcpy(s.isha,    p.isha,    sizeof(s.isha));
  s.fajr[5]    = '\0';
  s.syuruk[5]  = '\0';
  s.dhuhr[5]   = '\0';
  s.asr[5]     = '\0';
  s.maghrib[5] = '\0';
  s.isha[5]    = '\0';
}

bool saveSolatToRtcModule(const char* zone, uint16_t y, uint8_t m, uint8_t d, const SolatTime& s) {
  SolatRtcEepromPackage pkg;
  fillEepromPackage(pkg, zone, y, m, d, s);
  if (!rtcEepromWriteBytes(kSolatEepromAddr,
        reinterpret_cast<const uint8_t*>(&pkg), sizeof(pkg))) {
    Serial.println("[solat] EEPROM modul RTC: gagal tulis (semak wayar / alamat 0x57)");
    return false;
  }
  Serial.println("[solat] EEPROM AT24C32 (DS3231): waktu solat hari ini disimpan");
  return true;
}

bool loadSolatFromRtcModule(const char* zone, uint16_t y, uint8_t m, uint8_t d, SolatTime& s) {
  SolatRtcEepromPackage pkg;
  if (!rtcEepromReadBytes(kSolatEepromAddr,
        reinterpret_cast<uint8_t*>(&pkg), sizeof(pkg)))
    return false;
  if (pkg.magic != kSolatEepromMagic || pkg.version != kSolatEepromVer)
    return false;
  if (pkg.year != y || pkg.month != m || pkg.day != d)
    return false;
  if (strncmp(pkg.zone, zone, sizeof(pkg.zone)) != 0)
    return false;
  packageToSolat(pkg, s);
  return true;
}


// ─── Helper ───────────────────────────────────────────

int timeToMinutes(const char* t) {
  if (!t || strlen(t) < 5) return 0;
  int h = (t[0]-'0')*10 + (t[1]-'0');
  int m = (t[3]-'0')*10 + (t[4]-'0');
  return h * 60 + m;
}

void getSolatLabel(int idx, char* out) {
  const char* labels[] = {"Subuh","Syuruk","Zohor","Asar","Maghrib","Isyak"};
  if (idx >= 0 && idx < 6) strcpy(out, labels[idx]);
  else strcpy(out, "");
}

void getSolatTime(int idx, SolatTime& s, char* out) {
  const char* times[] = {s.fajr, s.syuruk, s.dhuhr, s.asr, s.maghrib, s.isha};
  if (idx >= 0 && idx < 6) {
    strncpy(out, times[idx], 5);
    out[5] = '\0';  // ← null terminator yang betul
  } else {
    out[0] = '\0';
  }
}

int getCurrentSolatIndex(DateTime now) {
  int nowMin = now.hour() * 60 + now.minute();
  const char* times[] = {
    todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
    todaySolat.asr,  todaySolat.maghrib, todaySolat.isha
  };
  for (int i = 0; i < 6; i++) {
    int solatMin = timeToMinutes(times[i]);
    if (nowMin >= (solatMin - 5) && nowMin <= (solatMin + 1)) return i;
  }
  return -1;
}

int getNextSolatIndex(DateTime now) {
  int nowMin = now.hour() * 60 + now.minute();
  const char* times[] = {
    todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
    todaySolat.asr,  todaySolat.maghrib, todaySolat.isha
  };
  for (int i = 0; i < 6; i++) {
    if (nowMin < timeToMinutes(times[i]) - 1) return i;
  }
  return 0; // Subuh esok
}


// ─── Parse ────────────────────────────────────────────

void parseSolatEntry(JsonObject entry, SolatTime& out) {
  // Ambil HH:MM sahaja (5 char pertama dari "HH:MM:SS")
  const char* src;

  src = entry["fajr"]    | "00:00";
  strncpy(out.fajr,    src, 5); out.fajr[5]    = '\0';

  src = entry["syuruk"]  | "00:00";
  strncpy(out.syuruk,  src, 5); out.syuruk[5]  = '\0';

  src = entry["dhuhr"]   | "00:00";
  strncpy(out.dhuhr,   src, 5); out.dhuhr[5]   = '\0';

  src = entry["asr"]     | "00:00";
  strncpy(out.asr,     src, 5); out.asr[5]     = '\0';

  src = entry["maghrib"] | "00:00";
  strncpy(out.maghrib, src, 5); out.maghrib[5] = '\0';

  src = entry["isha"]    | "00:00";
  strncpy(out.isha,    src, 5); out.isha[5]    = '\0';
}


// ─── Preferences (aktif jika guna hardware ESP32) ─────

void saveSolatToPrefs(const char* dateKey, SolatTime& s) {
  // Uncomment bila guna hardware sebenar (bukan Wokwi sim)
  if (!preferences.begin("solat", false)) return;
  char val[48];
  snprintf(val, sizeof(val), "%s,%s,%s,%s,%s,%s",
    s.fajr, s.syuruk, s.dhuhr, s.asr, s.maghrib, s.isha);
  preferences.putString(dateKey, val);
  preferences.end();
}

bool loadSolatFromPrefsKey(const char* dateKey, SolatTime& s) {
  // Baca dari Preferences/NVS
  if (!preferences.begin("solat", true)) return false;
  String v = preferences.getString(dateKey, "");
  preferences.end();
  if (v.length() < 10) return false;
  char val[48];
  v.toCharArray(val, sizeof(val));
  char* tok = strtok(val, ",");
  if (tok) { strncpy(s.fajr,    tok, 5); s.fajr[5]    = '\0'; } tok = strtok(NULL, ",");
  if (tok) { strncpy(s.syuruk,  tok, 5); s.syuruk[5]  = '\0'; } tok = strtok(NULL, ",");
  if (tok) { strncpy(s.dhuhr,   tok, 5); s.dhuhr[5]   = '\0'; } tok = strtok(NULL, ",");
  if (tok) { strncpy(s.asr,     tok, 5); s.asr[5]     = '\0'; } tok = strtok(NULL, ",");
  if (tok) { strncpy(s.maghrib, tok, 5); s.maghrib[5] = '\0'; } tok = strtok(NULL, ",");
  if (tok) { strncpy(s.isha,    tok, 5); s.isha[5]    = '\0'; }
  return true;
}

void loadSolatFromPrefs(const char* zone) {
  DateTime now = rtc.now();

  if (loadSolatFromRtcModule(zone,
        static_cast<uint16_t>(now.year()),
        static_cast<uint8_t>(now.month()),
        static_cast<uint8_t>(now.day()),
        todaySolat)) {
    solatLoaded = true;
    Serial.print("[solat] baca dari EEPROM modul DS3231: ");
    Serial.printf("%s %s %s %s %s %s\n",
      todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
      todaySolat.asr,  todaySolat.maghrib, todaySolat.isha);
    return;
  }

  char keyToday[20];
  snprintf(keyToday, sizeof(keyToday),
    "%s_%04d%02d%02d", zone, now.year(), now.month(), now.day());

  solatLoaded = loadSolatFromPrefsKey(keyToday, todaySolat);

  if (solatLoaded) {
    Serial.print("[solat] loaded from prefs (NVS): ");
    Serial.printf("%s %s %s %s %s %s\n",
      todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
      todaySolat.asr,  todaySolat.maghrib, todaySolat.isha);
  } else {
    strcpy(todaySolat.fajr,    "--:--");
    strcpy(todaySolat.syuruk,  "--:--");
    strcpy(todaySolat.dhuhr,   "--:--");
    strcpy(todaySolat.asr,     "--:--");
    strcpy(todaySolat.maghrib, "--:--");
    strcpy(todaySolat.isha,    "--:--");
    solatLoaded = false;
    Serial.println("[solat] no prefs data, using default fallback");
  }
}


// ─── Copy helper ──────────────────────────────────────

void copySolatTime(const SolatTime& src, SolatTime& dst) {
  strcpy(dst.fajr,    src.fajr);
  strcpy(dst.syuruk,  src.syuruk);
  strcpy(dst.dhuhr,   src.dhuhr);
  strcpy(dst.asr,     src.asr);
  strcpy(dst.maghrib, src.maghrib);
  strcpy(dst.isha,    src.isha);
}


// ─── Fetch dari JAKIM ─────────────────────────────────

void fetchAndSaveSolat(const char* zone) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[solat] WiFi not connected, skip fetch");
    return;
  }

  DateTime now = rtc.now();
  bool todayFound = false;

  Serial.printf("[solat] RTC today: %04d-%02d-%02d\n",
    now.year(), now.month(), now.day());

  char url[120];
  snprintf(url, sizeof(url),
    "https://www.e-solat.gov.my/index.php?r=esolatApi/takwimsolat&period=year&zone=%s", zone);

  Serial.print("[solat] fetching: "); Serial.println(url);

  HTTPClient http;
  http.useHTTP10(true);  // ← WAJIB: disable chunked, enable stream parsing
  http.begin(url);
  int code = http.GET();
  Serial.print("[solat] HTTP code: "); Serial.println(code);

  if (code != 200) {
    Serial.print("[solat] HTTP error: "); Serial.println(code);
    http.end();
    return;
  }

  // ─── Filter: ambil field yang perlu sahaja (jimat RAM) ───
  JsonDocument filter;
  JsonObject filterEntry = filter["prayerTime"].add<JsonObject>();
  filterEntry["date"]    = true;
  filterEntry["fajr"]    = true;
  filterEntry["syuruk"]  = true;
  filterEntry["dhuhr"]   = true;
  filterEntry["asr"]     = true;
  filterEntry["maghrib"] = true;
  filterEntry["isha"]    = true;

  // ─── Parse terus dari stream, TANPA getString() ──────────
  JsonDocument doc;
  DeserializationError error = deserializeJson(
    doc, http.getStream(),
    DeserializationOption::Filter(filter)
  );

  http.end();  // ← Tutup connection SELEPAS deserializeJson selesai

  if (error) {
    Serial.print("[solat] JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }

  Serial.println("[solat] JSON parsed OK");

  JsonArray arr = doc["prayerTime"].as<JsonArray>();
  Serial.print("[solat] total entries: "); Serial.println(arr.size());

  for (JsonObject entry : arr) {
    const char* dateStr = entry["date"] | "";

    // Parse format: "29-Mar-2026"
    int d = 0, m = 0, y = 0;
    char mStr[4] = "";
    if (sscanf(dateStr, "%d-%3s-%d", &d, mStr, &y) == 3) {
      const char* months[] = {
        "","Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
      };
      for (int i = 1; i <= 12; i++) {
        if (strcmp(mStr, months[i]) == 0) { m = i; break; }
      }
    }

    SolatTime s;
    parseSolatEntry(entry, s);

    // Jangan simpan 365 hari ke NVS — ruang NVS (~24KB) tidak mencukupi.
    // Hanya hari ini disimpan selepas gelung (lihat bawah).

    // Semak sama ada ini hari ini
    if (y == now.year() && m == now.month() && d == now.day()) {
      copySolatTime(s, todaySolat);
      todayFound = true;
      Serial.printf("[solat] TODAY %s: %s %s %s %s %s %s\n",
        dateStr,
        todaySolat.fajr, todaySolat.syuruk, todaySolat.dhuhr,
        todaySolat.asr,  todaySolat.maghrib, todaySolat.isha);
    }
  }

  if (!todayFound) {
    Serial.println("[solat] WARNING: today not found in data!");
  } else {
    char keyToday[20];
    snprintf(keyToday, sizeof(keyToday),
      "%s_%04d%02d%02d", zone, now.year(), now.month(), now.day());
    saveSolatToRtcModule(zone,
      static_cast<uint16_t>(now.year()),
      static_cast<uint8_t>(now.month()),
      static_cast<uint8_t>(now.day()),
      todaySolat);
    saveSolatToPrefs(keyToday, todaySolat);
    Serial.println("[solat] NVS: sandaran hari ini (jika ruang ada)");
  }

  strncpy(savedZone, zone, sizeof(savedZone) - 1);
  savedZone[sizeof(savedZone) - 1] = '\0';

  solatLoaded = todayFound;

  Serial.printf("[solat] done. zone=%s loaded=%s\n",
    savedZone, solatLoaded ? "YES" : "NO");
}