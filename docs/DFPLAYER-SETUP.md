# 🔊 DFPlayer Mini MP3-TF-16P Setup Guide

## 📦 Hardware Yang Diperlukan

- **DFPlayer Mini MP3-TF-16P** module
- **MicroSD Card** (FAT32, max 32GB)
- **Speaker** 3W-5W (8Ω recommended)
- **Jumper wires**

---

## 🔌 Wiring Connection

### ESP32 → DFPlayer Mini

```
ESP32          DFPlayer Mini
─────────────  ─────────────
GPIO 16 (RX) → TX
GPIO 17 (TX) → RX
5V           → VCC
GND          → GND
```

### DFPlayer Mini → Speaker

```
DFPlayer Mini  Speaker
─────────────  ─────────
SPK_1        → Speaker (+)
SPK_2        → Speaker (-)
```

### Wiring Diagram

```
┌──────────────┐
│   ESP32      │
│              │
│ GPIO16 (RX)  ├──────┐
│ GPIO17 (TX)  ├────┐ │
│ 5V           ├──┐ │ │
│ GND          ├┐ │ │ │
└──────────────┘│ │ │ │
                │ │ │ │
                │ │ │ └──→ TX
                │ │ └────→ RX
                │ └──────→ VCC
                └────────→ GND
                         ┌──────────────────┐
                         │  DFPlayer Mini   │
                         │                  │
                         │  SPK_1 ────┬─────┤
                         │  SPK_2 ────┼─────┤
                         └────────────┘     │
                                      ┌─────┴─────┐
                                      │  Speaker  │
                                      │   3W 8Ω   │
                                      └───────────┘
```

---

## 💾 SD Card Setup

### 1. Format SD Card

- **Format**: FAT32
- **Allocation size**: 4096 bytes (default)
- **Max size**: 32GB

### 2. Folder Structure

Buat folder `mp3` di root SD card:

```
SD Card Root
└── mp3/
    ├── 001.mp3  ← Warning beep (3x sebelum waktu solat)
    ├── 002.mp3  ← Azan beep panjang (5x masuk waktu)
    ├── 003.mp3  ← Hourly beep (setiap jam)
    ├── 004.mp3  ← Startup sound
    └── 005.mp3  ← Full azan (optional)
```

### 3. File Requirements

- **Format**: MP3 only
- **Naming**: 001.mp3, 002.mp3, 003.mp3, etc. (3 digits)
- **Bitrate**: 128kbps recommended
- **Sample rate**: 44.1kHz or 48kHz

### 4. Recommended Audio Files

#### 001.mp3 - Warning Beep
- **Duration**: ~0.5 second
- **Type**: Short beep/ding sound
- **Usage**: Button press, warning 30s before prayer

#### 002.mp3 - Azan Beep
- **Duration**: ~1 second
- **Type**: Long beep/tone
- **Usage**: Prayer time notification (played 5x)

#### 003.mp3 - Hourly Beep
- **Duration**: ~0.3 second
- **Type**: Soft beep
- **Usage**: Hourly time notification

#### 004.mp3 - Startup Sound
- **Duration**: ~1 second
- **Type**: Startup melody/chime
- **Usage**: Device startup

#### 005.mp3 - Full Azan (Optional)
- **Duration**: ~3-5 minutes
- **Type**: Complete azan recitation
- **Usage**: Full azan at prayer time (instead of 5 beeps)

---

## 🎵 Download Sample Audio Files

### Option 1: Generate Simple Beeps

Guna online tone generator:
- https://www.szynalski.com/tone-generator/
- https://onlinetonegenerator.com/

**Settings untuk beep:**
- 001.mp3: 880Hz, 0.5s
- 002.mp3: 1200Hz, 1.0s
- 003.mp3: 600Hz, 0.3s
- 004.mp3: 1000Hz, 1.0s

### Option 2: Download dari Freesound

- https://freesound.org/
- Search: "beep", "notification", "chime"
- Filter: MP3, Creative Commons

### Option 3: Guna Text-to-Speech

Untuk announcement (contoh: "Waktu Subuh"):
- https://ttsmp3.com/
- Select: Malay language
- Download as MP3

---

## 🔧 Testing DFPlayer

### 1. Check Serial Monitor

Selepas upload code, check serial monitor:

```
[dfplayer] initializing...
[dfplayer] ready! Volume: 25
```

Jika fail:
```
[dfplayer] failed! Using buzzer fallback
```

### 2. Test Startup Sound

Bila ESP32 boot, akan main track 004.mp3 automatically.

### 3. Troubleshooting

#### ❌ "DFPlayer failed"

**Possible causes:**
1. **Wiring salah** - Check RX/TX connection
2. **SD card tidak detect** - Reformat ke FAT32
3. **File MP3 tidak ada** - Check folder `mp3/` dan file `001.mp3` - `004.mp3`
4. **Power insufficient** - Guna 5V external power supply

**Solutions:**
```bash
# Check wiring:
ESP32 GPIO16 (RX) → DFPlayer TX ✓
ESP32 GPIO17 (TX) → DFPlayer RX ✓

# Check SD card:
- Format: FAT32 ✓
- Folder: /mp3/ ✓
- Files: 001.mp3, 002.mp3, 003.mp3, 004.mp3 ✓

# Check power:
- VCC: 5V (NOT 3.3V) ✓
- GND: Connected ✓
```

#### ❌ "No sound from speaker"

1. Check speaker connection (SPK_1, SPK_2)
2. Check volume: `dfPlayer.volume(25);` (0-30)
3. Test dengan speaker lain
4. Check MP3 files playable di computer

#### ❌ "SD card not detected"

1. Reformat SD card to FAT32
2. Try different SD card (max 32GB)
3. Check SD card slot di DFPlayer
4. Ensure SD card fully inserted

---

## 🎚️ Volume Control

Default volume: **25** (range 0-30)

Untuk adjust volume, edit di `main.cpp`:

```cpp
dfPlayer.volume(25);  // Change 25 to 0-30
```

**Recommended volumes:**
- **15-20**: Quiet environment
- **20-25**: Normal environment (default)
- **25-30**: Noisy environment

---

## 🔄 Fallback ke Buzzer

Jika DFPlayer fail, system akan auto fallback ke buzzer:

```cpp
if (dfPlayerReady) {
  playSound(1, 0);  // Main MP3
} else {
  buzzerTone(880, 150);  // Guna buzzer
}
```

Buzzer pin: **GPIO 13** (masih active sebagai backup)

---

## 📝 Code Reference

### Main Functions

#### `playSound(trackNumber, delayMs)`

Main MP3 track dari SD card:

```cpp
playSound(1, 0);    // Track 001.mp3 - warning
playSound(2, 0);    // Track 002.mp3 - azan beep
playSound(3, 500);  // Track 003.mp3 - hourly (wait 500ms)
playSound(4, 0);    // Track 004.mp3 - startup
playSound(5, 0);    // Track 005.mp3 - full azan
```

#### Enable Full Azan

Untuk main azan penuh instead of 5 beeps, edit di `main.cpp`:

```cpp
// Line ~310
if (dfPlayerReady) {
  // Option 1: Main azan penuh (uncomment ini)
  playSound(5, 0);  // Track 005.mp3 - full azan
  
  // Option 2: Main beep 5x (comment out ini)
  // for (int j = 0; j < 5; j++) {
  //   playSound(2, 0);
  //   delay(1500);
  // }
}
```

---

## 🎯 Quick Start Checklist

- [ ] Wire ESP32 GPIO16/17 to DFPlayer RX/TX
- [ ] Connect 5V and GND
- [ ] Connect speaker to SPK_1/SPK_2
- [ ] Format SD card to FAT32
- [ ] Create folder `mp3/` di SD card
- [ ] Copy MP3 files: 001.mp3 - 004.mp3
- [ ] Insert SD card into DFPlayer
- [ ] Upload code ke ESP32
- [ ] Check serial monitor for "[dfplayer] ready!"
- [ ] Test startup sound

---

## 🔊 Audio Examples

### Sample Beep Sounds

Anda boleh guna audio files ini sebagai contoh:

**001.mp3 - Warning Beep:**
```
Frequency: 880Hz
Duration: 0.5s
Type: Single beep
```

**002.mp3 - Azan Beep:**
```
Frequency: 1200Hz
Duration: 1.0s
Type: Long tone
```

**003.mp3 - Hourly Beep:**
```
Frequency: 600Hz
Duration: 0.3s
Type: Soft beep
```

**004.mp3 - Startup:**
```
Type: Two-tone chime
Duration: 1.0s
Notes: C5 → E5
```

---

## 📚 Additional Resources

- **DFPlayer Mini Datasheet**: [Download](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299)
- **Library Documentation**: [DFRobotDFPlayerMini](https://github.com/DFRobot/DFRobotDFPlayerMini)
- **Audio Converter**: [Online Audio Converter](https://online-audio-converter.com/)

---

## 💡 Tips

1. **Test SD card di computer** - Pastikan MP3 files boleh play
2. **Guna short audio files** - Faster response time
3. **Backup buzzer** - System auto fallback jika DFPlayer fail
4. **Volume adjustment** - Start dengan volume 20, adjust mengikut keperluan
5. **Power supply** - Guna 5V yang stabil untuk best audio quality

---

**Status**: ✅ DFPlayer Mini integration complete!

Sekarang iPray boleh main audio dari SD card terus ke speaker tanpa buzzer! 🎵
