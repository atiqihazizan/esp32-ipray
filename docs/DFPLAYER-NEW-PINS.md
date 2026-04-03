# 🔄 DFPlayer Mini - New Pin Configuration

## Pin Change: GPIO16/17 → GPIO25/26

---

## 🔌 NEW WIRING DIAGRAM

```
┌─────────────────────────────────────────────────────────────┐
│                        ESP32 DevKit                         │
│                                                             │
│  5V  ────────────────────────────────────────→ VCC         │
│  GND ────────────────────────────────────────→ GND         │
│                                                             │
│  GPIO25 (RX) ──→ [1kΩ] ──→ TX (pin 3)                      │
│  GPIO26 (TX) ──→ [1kΩ] ──→ RX (pin 2)                      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
                              │
                              │
                    ┌─────────┴──────────┐
                    │   DFPlayer Mini    │
                    │   MP3-TF-16P       │
                    │                    │
                    │  SPK_1 ──→ (+)     │
                    │  SPK_2 ──→ (-)     │
                    └────────────────────┘
                              │
                        ┌─────┴─────┐
                        │  Speaker  │
                        │  3W-5W 8Ω │
                        └───────────┘
```

---

## 📝 Pin Mapping

### Old Configuration (GPIO16/17):
```
ESP32 GPIO16 (RX) → 1kΩ → DFPlayer TX  ❌ Not working
ESP32 GPIO17 (TX) → 1kΩ → DFPlayer RX  ❌ Not working
```

### New Configuration (GPIO25/26):
```
ESP32 GPIO25 (RX) → 1kΩ → DFPlayer TX  ✅ New pins
ESP32 GPIO26 (TX) → 1kΩ → DFPlayer RX  ✅ New pins
```

---

## 🔧 Migration Steps

### Step 1: Upload New Code

```bash
cd ~/IoT/esp32-ipray
./upload.sh
# Select option 3 (Upload ke ESP32)
```

### Step 2: Power Off ESP32

Disconnect USB or power supply.

### Step 3: Rewire DFPlayer

**Remove old wires:**
- Disconnect wire from GPIO16
- Disconnect wire from GPIO17

**Connect new wires:**
- DFPlayer TX (pin 3) → 1kΩ resistor → ESP32 GPIO25
- DFPlayer RX (pin 2) → 1kΩ resistor → ESP32 GPIO26

**Keep unchanged:**
- ✅ 5V power connection
- ✅ GND connection
- ✅ Speaker connections (SPK_1/SPK_2)
- ✅ 1kΩ resistors (reuse them)

### Step 4: Verify SD Card

- Ensure SD card inserted in DFPlayer
- Files: 001.mp3 - 004.mp3 in mp3/ folder
- No hidden files (._*.mp3)

### Step 5: Power On & Test

```bash
# Power on ESP32
# Check serial monitor
cd ~/IoT/esp32-ipray
./upload.sh
# Pilih 5 (Monitor sahaja) atau: pio device monitor
```

---

## 🎯 Expected Results

### ✅ Success (GPIO25/26 works):
```
[dfplayer] initializing...
[dfplayer] ready! Volume: 30 (MAX) | EQ: BASS
[dfplayer] playing track 4 at MAX volume
```
**→ You should hear startup sound from speaker!**

### ❌ Still fails:
```
[dfplayer] initializing...
[dfplayer] failed! Using buzzer fallback
```
**→ Try alternative pins (see below)**

---

## 🔄 Alternative Pin Options

If GPIO25/26 also doesn't work, try these:

### Option A: GPIO32 & GPIO33
```cpp
#define DFPLAYER_RX     32
#define DFPLAYER_TX     33
```

### Option B: GPIO18 & GPIO19
```cpp
#define DFPLAYER_RX     18
#define DFPLAYER_TX     19
```

### Option C: GPIO12 & GPIO13
```cpp
#define DFPLAYER_RX     12
#define DFPLAYER_TX     13
```
⚠️ Note: GPIO13 is buzzer pin, need to move buzzer to another pin

---

## 📊 ESP32 Pin Reference

### Safe Pins for DFPlayer (General Purpose):
- ✅ GPIO25, GPIO26 (Recommended)
- ✅ GPIO32, GPIO33 (Good alternative)
- ✅ GPIO18, GPIO19 (Also good)
- ✅ GPIO12, GPIO13, GPIO14 (If not used)

### Avoid These Pins:
- ❌ GPIO0 (Boot button)
- ❌ GPIO2 (Boot mode)
- ❌ GPIO6-11 (Flash pins)
- ❌ GPIO34-39 (Input only, no pull-up)

### Already Used Pins (Don't use):
- GPIO21, GPIO22 (I2C for OLED & RTC)
- GPIO15, GPIO4, GPIO14, GPIO27, GPIO5, GPIO23 (Buttons)
- GPIO13 (Buzzer)

---

## 💡 Why GPIO16/17 Might Not Work

Possible reasons:
1. **PSRAM conflict** - GPIO16/17 used by PSRAM on some ESP32 boards
2. **UART2 hardware issue** - Some ESP32 variants have UART2 issues
3. **Pin damage** - Physical damage to GPIO16/17
4. **Board design** - Some boards use these pins internally

GPIO25/26 are safe general-purpose pins that don't have these issues.

---

## 🧪 Troubleshooting

### Issue: Still "[dfplayer] failed!" after pin change

**Check:**
1. ✅ Wiring correct? (GPIO25 → DFPlayer TX, GPIO26 → DFPlayer RX)
2. ✅ 1kΩ resistors in place?
3. ✅ 5V power connected?
4. ✅ GND connected?
5. ✅ SD card inserted?
6. ✅ MP3 files in mp3/ folder?
7. ✅ No hidden files (._*.mp3)?

**Try:**
- Different SD card
- Different DFPlayer module
- Alternative pins (GPIO32/33)
- Buzzer as temporary solution

---

## 📸 Visual Reference

### DFPlayer Pinout:
```
     ┌─────────────────┐
  1  │ VCC    (5V)     │ ← ESP32 5V
  2  │ RX     (Input)  │ ← ESP32 GPIO26 (TX) via 1kΩ
  3  │ TX     (Output) │ ← ESP32 GPIO25 (RX) via 1kΩ
  4  │ DAC_R           │
  5  │ DAC_L           │
  6  │ SPK_1           │ ← Speaker (+)
  7  │ GND             │ ← ESP32 GND
  8  │ SPK_2           │ ← Speaker (-)
  9  │ IO_1            │
 10  │ GND             │
 11  │ IO_2            │
 12  │ ADKEY_1         │
 13  │ ADKEY_2         │
 14  │ USB+            │
 15  │ USB-            │
 16  │ BUSY            │
     └─────────────────┘
```

---

## ✅ Checklist

- [ ] Code uploaded with new pins (GPIO25/26)
- [ ] ESP32 powered off
- [ ] Wire moved from GPIO16 to GPIO25 (DFPlayer TX line)
- [ ] Wire moved from GPIO17 to GPIO26 (DFPlayer RX line)
- [ ] 1kΩ resistors still in place
- [ ] 5V and GND connections unchanged
- [ ] Speaker connections unchanged
- [ ] SD card inserted with MP3 files
- [ ] ESP32 powered on
- [ ] Serial monitor checked
- [ ] Sound tested

---

## 🎯 Quick Commands

```bash
# Upload new code
cd ~/IoT/esp32-ipray
./upload.sh

# Serial monitor
./upload.sh
# Pilih 5, atau: pio device monitor

# If need to change pins again, edit:
nano include/config.h
# Change DFPLAYER_RX and DFPLAYER_TX values
# Then upload again
```

---

**Status:** ✅ Pin configuration updated to GPIO25/26

**Next:** Upload code → Rewire → Test → Enjoy audio! 🎵
