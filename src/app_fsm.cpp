#include "app_fsm.h"
#include "config.h"
#include "utils.h"

// ─── Blink (colon) — 500 ms ────────────────────────────────────────────
static void appFsmBlinkTick() {
  if (millis() - lastBlinkTime >= 500UL) {
    colonBlink    = !colonBlink;
    lastBlinkTime = millis();
  }
}

// ─── Audio: baris gilir + sub-keadaan (tiada delay dalam loop) ──────────
enum AudioJobType : uint8_t { AJ_PlayTrack = 0, AJ_Gap = 1, AJ_SpeakTime = 2 };

enum AudioPlayHow : uint8_t {
  AP_ConfigMode = 0, /* dfPlay(..., DFPLAYER_PLAY_MODE) */
  AP_Root,
  AP_Mp3Folder,
  AP_Folder,
};

struct AudioJob {
  AudioJobType type;
  uint8_t      playHow; /* AudioPlayHow — hanya AJ_PlayTrack */
  uint16_t     a;
  uint16_t     b;
  uint16_t     c; /* folder: a=folder, b=trek, c=tailMs; lain: a=trek, b=tailMs */
};

static constexpr uint8_t kAudioQCap = 14;
static AudioJob          audioQ[kAudioQCap];
static uint8_t           qHead = 0;
static uint8_t           qTail = 0;
static uint8_t           qCount = 0;

static bool audioQPush(const AudioJob& j) {
  if (qCount >= kAudioQCap)
    return false;
  audioQ[qTail] = j;
  qTail         = (uint8_t)((qTail + 1) % kAudioQCap);
  qCount++;
  return true;
}

static bool audioQPop(AudioJob* out) {
  if (qCount == 0)
    return false;
  *out   = audioQ[qHead];
  qHead  = (uint8_t)((qHead + 1) % kAudioQCap);
  qCount = (uint8_t)(qCount - 1);
  return true;
}

enum ExecKind : uint8_t { ExNone = 0, ExGap, ExPlayTrack, ExSpeakTime };

static ExecKind      execKind   = ExNone;
static unsigned long phaseStart = 0;

static uint8_t ptPlayHow = AP_ConfigMode;
static int     ptArg0    = 0;
static int     ptArg1    = 0;
static int     ptTail    = 0;
static uint8_t ptPhase   = 0;

static unsigned gapMs = 0;

static int     spH12    = 1;
static int     spMinTrk = 0;
static uint8_t spPhase  = 0;

static void execFinish() {
  execKind = ExNone;
}

static void startPlayTrackJob(uint8_t how, int a, int b, int c) {
  execKind   = ExPlayTrack;
  phaseStart = millis();
  ptPlayHow  = how;
  if (how == AP_Folder) {
    ptArg0 = a;
    ptArg1 = b;
    ptTail = c;
  } else {
    ptArg0 = a;
    ptArg1 = 0;
    ptTail = b;
  }
  ptPhase = 0;
  utilsDfPlayerSendStop();
}

static void startGapJob(unsigned ms) {
  execKind   = ExGap;
  phaseStart = millis();
  gapMs      = ms;
}

static void startSpeakTimeJob(int hours, int minutes) {
  int h = hours;
  if (h < 0)
    h = 0;
  else if (h > 23)
    h = 23;
  int m = minutes;
  if (m < 0)
    m = 0;
  else if (m > 59)
    m = 59;
  int h12 = h % 12;
  if (h12 == 0)
    h12 = 12;
  const int minTr = (m == 0) ? 60 : m;

  execKind   = ExSpeakTime;
  phaseStart = millis();
  spH12      = h12;
  spMinTrk   = minTr;
  spPhase    = 0;
  utilsDfPlayerSendStop();
}

static void tryStartNextJob() {
  if (execKind != ExNone)
    return;
  if (!dfPlayerReady)
    return;
  AudioJob j;
  if (!audioQPop(&j))
    return;
  if (j.type == AJ_PlayTrack)
    startPlayTrackJob(j.playHow, (int)j.a, (int)j.b, (int)j.c);
  else if (j.type == AJ_Gap)
    startGapJob((unsigned)j.a);
  else
    startSpeakTimeJob((int)j.a, (int)j.b);
}

static void appFsmAudioTick() {
  if (!dfPlayerReady) {
    if (execKind != ExNone)
      execFinish();
    qHead = qTail = qCount = 0;
    return;
  }

  const unsigned long now = millis();

  if (execKind == ExGap) {
    if (now - phaseStart >= gapMs)
      execFinish();
  } else if (execKind == ExPlayTrack) {
    if (ptPhase == 0) {
      if (now - phaseStart >= 80UL) {
        utilsDfPlayerSendVolume(30);
        phaseStart = now;
        ptPhase    = 1;
      }
    } else if (ptPhase == 1) {
      if (now - phaseStart >= 50UL) {
        if (ptPlayHow == AP_ConfigMode)
          dfPlay(ptArg0, DFPLAYER_PLAY_MODE);
        else if (ptPlayHow == AP_Root)
          dfPlay(ptArg0, 1);
        else if (ptPlayHow == AP_Mp3Folder)
          dfPlay(ptArg0, 0);
        else if (ptPlayHow == AP_Folder)
          dfPlayFolder(ptArg0, ptArg1);
        phaseStart = now;
        ptPhase    = 2;
      }
    } else if (ptPhase == 2) {
      if (now - phaseStart >= 200UL) {
        if (ptTail > 0) {
          phaseStart = now;
          ptPhase    = 3;
        } else {
          execFinish();
        }
      }
    } else if (ptPhase == 3) {
      if (now - phaseStart >= (unsigned long)ptTail)
        execFinish();
    }
  } else if (execKind == ExSpeakTime) {
    if (spPhase == 0) {
      if (now - phaseStart >= 80UL) {
        utilsDfPlayerSendVolume(30);
        phaseStart = now;
        spPhase    = 1;
      }
    } else if (spPhase == 1) {
      if (now - phaseStart >= 50UL) {
        dfPlay(spH12, 2);
        phaseStart = now;
        spPhase    = 2;
      }
    } else if (spPhase == 2) {
      /* Tunggu trek jam habis: pin BUSY idle, atau timeout */
      const unsigned long elapsed = now - phaseStart;
      const bool pastGuard       = elapsed >= (unsigned long)DFPLAYER_BUSY_IGNORE_MS;
      const bool busyTimedOut    = elapsed >= (unsigned long)SPEAK_TIME_MS_AFTER_HOUR;
      if ((pastGuard && utilsDfPlayerOutputIdle()) || busyTimedOut) {
        dfPlay(spMinTrk, 3);
        phaseStart = now;
        spPhase    = 3;
      }
    } else if (spPhase == 3) {
      const unsigned long elapsed = now - phaseStart;
      const bool pastGuard       = elapsed >= (unsigned long)DFPLAYER_BUSY_IGNORE_MS;
      const bool busyTimedOut    = elapsed >= (unsigned long)SPEAK_TIME_MS_AFTER_MINUTE;
      if ((pastGuard && utilsDfPlayerOutputIdle()) || busyTimedOut)
        execFinish();
    }
  }

  if (execKind == ExNone)
    tryStartNextJob();
}

bool appFsmAudioBusy() {
  return execKind != ExNone || qCount > 0;
}

bool appFsmEnqPlay(int track, int tailMs) {
  if (!dfPlayerReady)
    return false;
  return audioQPush({ AJ_PlayTrack, AP_ConfigMode, (uint16_t)track, (uint16_t)tailMs, 0 });
}

bool appFsmEnqRoot(int track, int tailMs) {
  if (!dfPlayerReady)
    return false;
  return audioQPush({ AJ_PlayTrack, AP_Root, (uint16_t)track, (uint16_t)tailMs, 0 });
}

bool appFsmEnqMp3(int track, int tailMs) {
  if (!dfPlayerReady)
    return false;
  return audioQPush({ AJ_PlayTrack, AP_Mp3Folder, (uint16_t)track, (uint16_t)tailMs, 0 });
}

bool appFsmEnqFolder(int folderNumber, int trackInFolder, int tailMs) {
  if (!dfPlayerReady)
    return false;
  return audioQPush({ AJ_PlayTrack, AP_Folder, (uint16_t)folderNumber, (uint16_t)trackInFolder,
                      (uint16_t)tailMs });
}

bool appFsmEnqGap(unsigned ms) {
  if (!dfPlayerReady)
    return false;
  return audioQPush({ AJ_Gap, 0, (uint16_t)ms, 0, 0 });
}

bool appFsmEnqSpeak(int hours, int minutes) {
  if (!dfPlayerReady)
    return false;
  return audioQPush({ AJ_SpeakTime, 0, (uint16_t)hours, (uint16_t)minutes, 0 });
}

void appFsmInit() {
  qHead = qTail = qCount = 0;
  execKind = ExNone;
}

void appFsmUpdate() {
  appFsmBlinkTick();
  appFsmAudioTick();
}
