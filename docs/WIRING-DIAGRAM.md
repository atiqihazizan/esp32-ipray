# 🔌 iPray Complete Wiring Diagram

## Full System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP32 DevKit                        │
│                                                             │
│  3V3 ────────────┬──────────────┬──────────────────────────│
│                  │              │                          │
│  GND ────────────┼──────────────┼──────────────┬───────────│
│                  │              │              │           │
│  GPIO21 (SDA) ───┼──────────────┼──────────────┼───────────│
│  GPIO22 (SCL) ───┼──────────────┼──────────────┼───────────│
│                  │              │              │           │
│  GPIO16 (RX) ────┼──────────────┼──────────────┼───────────│
│  GPIO17 (TX) ────┼──────────────┼──────────────┼───────────│
│                  │              │              │           │
│  GPIO13 ─────────┼──────────────┼──────────────┼───────────│
│                  │              │              │           │
│  GPIO15 ─────────┼──────────────┼──────────────┼───────────│
│  GPIO4  ─────────┼──────────────┼──────────────┼───────────│
│  GPIO14 ─────────┼──────────────┼──────────────┼───────────│
│  GPIO27 ─────────┼──────────────┼──────────────┼───────────│
│  GPIO5  ─────────┼──────────────┼──────────────┼───────────│
│  GPIO23 ─────────┼──────────────┼──────────────┼───────────│
│                  │              │              │           │
│  5V ─────────────┼──────────────┼──────────────┼───────────│
└──────────────────┼──────────────┼──────────────┼───────────┘
                   │              │              │
                   │              │              │
         ┌─────────┴────┐  ┌──────┴──────┐  ┌───┴──────────┐
         │              │  │             │  │              │
    ┌────┴────┐    ┌────┴────┐    ┌─────┴─────┐    ┌──────┴──────┐
    │  OLED   │    │   RTC   │    │ DFPlayer  │    │   Buttons   │
    │ SSD1306 │    │ DS3231  │    │   Mini    │    │   (6x)      │
    └─────────┘    └─────────┘    └───────────┘    └─────────────┘
                                         │
                                    ┌────┴────┐
                                    │ Speaker │
                                    │  3W 8Ω  │
                                    └─────────┘
```

---

## Component Connections

### 1. OLED Display (SSD1306)

```
ESP32          OLED SSD1306
─────────────  ────────────
3.3V         → VCC
GND          → GND
GPIO21 (SDA) → SDA
GPIO22 (SCL) → SCL
```

**I2C Address**: `0x3C`

---

### 2. RTC Module (DS3231)

```
ESP32          RTC DS3231
─────────────  ──────────
3.3V         → VCC
GND          → GND
GPIO21 (SDA) → SDA
GPIO22 (SCL) → SCL
```

**I2C Address**: `0x68`

---

### 3. DFPlayer Mini MP3-TF-16P

```
ESP32          DFPlayer Mini
─────────────  ─────────────
5V           → VCC
GND          → GND
GPIO16 (RX)  → TX
GPIO17 (TX)  → RX
```

**Speaker Connection:**
```
DFPlayer Mini  Speaker
─────────────  ───────
SPK_1        → (+)
SPK_2        → (-)
```

**Notes:**
- ⚠️ DFPlayer needs **5V** (NOT 3.3V)
- Speaker: 3W-5W, 8Ω recommended
- SD Card: FAT32, max 32GB

---

### 4. Buttons (6x Push Buttons)

```
ESP32    Button    Description
───────  ────────  ────────────────
GPIO15 → BTN_MENU  Menu navigation
GPIO4  → BTN_UP    Scroll up
GPIO14 → BTN_DOWN  Scroll down
GPIO27 → BTN_SET   Select/Sleep
GPIO5  → BTN_RET   Return/Back
GPIO23 → BTN_LAYOUT Switch layout
```

**Button Wiring:**
```
ESP32 GPIO ──┬── Button ── GND
             │
             └── 10kΩ ── 3.3V (pull-up)
```

**Note**: ESP32 has internal pull-up, external resistor optional.

---

### 5. Buzzer (Backup)

```
ESP32      Buzzer
─────────  ──────
GPIO13   → (+)
GND      → (-)
```

**Type**: Passive buzzer (PWM controlled)

**Note**: Buzzer auto fallback jika DFPlayer fail.

---

## Complete Pin Mapping Table

| ESP32 Pin | Function      | Connected To          | Notes                |
|-----------|---------------|-----------------------|----------------------|
| 3.3V      | Power         | OLED VCC, RTC VCC     | Max 600mA total      |
| 5V        | Power         | DFPlayer VCC          | For DFPlayer only    |
| GND       | Ground        | All GND pins          | Common ground        |
| GPIO21    | I2C SDA       | OLED SDA, RTC SDA     | Shared I2C bus       |
| GPIO22    | I2C SCL       | OLED SCL, RTC SCL     | Shared I2C bus       |
| GPIO16    | UART2 RX      | DFPlayer TX           | Serial communication |
| GPIO17    | UART2 TX      | DFPlayer RX           | Serial communication |
| GPIO13    | PWM           | Buzzer (+)            | Backup audio         |
| GPIO15    | Digital Input | Button MENU           | Pull-up enabled      |
| GPIO4     | Digital Input | Button UP             | Pull-up enabled      |
| GPIO14    | Digital Input | Button DOWN           | Pull-up enabled      |
| GPIO27    | Digital Input | Button SET            | Pull-up enabled      |
| GPIO5     | Digital Input | Button RET            | Pull-up enabled      |
| GPIO23    | Digital Input | Button LAYOUT         | Pull-up enabled      |

---

## Breadboard Layout

```
                    ESP32 DevKit
                ┌─────────────────┐
                │                 │
                │  ┌───────────┐  │
                │  │           │  │
    OLED ───────┼──┤ GPIO21/22 │  │
    RTC  ───────┼──┤  (I2C)    │  │
                │  │           │  │
                │  └───────────┘  │
                │                 │
                │  ┌───────────┐  │
    DFPlayer ───┼──┤ GPIO16/17 │  │
                │  │  (UART2)  │  │
                │  └───────────┘  │
                │                 │
                │  ┌───────────┐  │
    Buttons ────┼──┤ GPIO15,4, │  │
                │  │ 14,27,5,23│  │
                │  └───────────┘  │
                │                 │
                │  ┌───────────┐  │
    Buzzer ─────┼──┤  GPIO13   │  │
                │  └───────────┘  │
                │                 │
                └─────────────────┘
```

---

## Power Requirements

| Component      | Voltage | Current  | Notes                    |
|----------------|---------|----------|--------------------------|
| ESP32          | 5V USB  | ~250mA   | Via USB or VIN           |
| OLED SSD1306   | 3.3V    | ~20mA    | From ESP32 3.3V          |
| RTC DS3231     | 3.3V    | ~1mA     | From ESP32 3.3V          |
| DFPlayer Mini  | 5V      | ~200mA   | From ESP32 5V pin        |
| Speaker        | -       | ~500mA   | Powered by DFPlayer      |
| Buzzer         | 3.3V    | ~30mA    | From ESP32 GPIO13        |
| Buttons        | 3.3V    | <1mA     | Minimal current          |

**Total**: ~1A recommended (2A power supply for safety margin)

---

## Wiring Tips

### ✅ Best Practices

1. **Use color-coded wires:**
   - Red: 3.3V / 5V
   - Black: GND
   - Yellow: SDA (I2C)
   - Blue: SCL (I2C)
   - Green: TX/RX
   - Other colors: Buttons

2. **Keep I2C wires short** (< 20cm for best reliability)

3. **Twist I2C wires together** to reduce interference

4. **Use common ground** for all components

5. **Test each component separately** before connecting all

### ⚠️ Common Mistakes

1. ❌ **Wrong voltage**: DFPlayer needs 5V, not 3.3V
2. ❌ **Swapped TX/RX**: ESP32 RX → DFPlayer TX (not RX to RX)
3. ❌ **No common ground**: All GND must connect together
4. ❌ **I2C address conflict**: OLED (0x3C) and RTC (0x68) must differ
5. ❌ **Insufficient power**: Use 2A power supply for full system

---

## Testing Checklist

### Step 1: Test I2C Devices

```bash
# Upload I2C scanner
cd ~/IoT/esp32-ipray
./detect-board.sh
```

Expected output:
```
I2C device found at 0x3C (OLED)
I2C device found at 0x68 (RTC)
```

### Step 2: Test DFPlayer

Check serial monitor:
```
[dfplayer] initializing...
[dfplayer] ready! Volume: 25
```

### Step 3: Test Buttons

Press each button and check serial monitor for response.

### Step 4: Test Audio

Startup sound should play automatically on boot.

---

## Troubleshooting

### ❌ OLED not detected

1. Check I2C wiring (SDA/SCL)
2. Verify I2C address: `0x3C`
3. Try different OLED module
4. Check power: 3.3V

### ❌ RTC not detected

1. Check I2C wiring (SDA/SCL)
2. Verify I2C address: `0x68`
3. Check battery on RTC module
4. Check power: 3.3V

### ❌ DFPlayer failed

1. Check wiring: RX/TX swapped?
2. Verify power: 5V (NOT 3.3V)
3. Check SD card: FAT32 format?
4. Check MP3 files: 001.mp3 - 004.mp3?

### ❌ No sound from speaker

1. Check speaker connection (SPK_1/SPK_2)
2. Adjust volume: `dfPlayer.volume(25);`
3. Test speaker with different audio source
4. Check MP3 files playable on computer

### ❌ Buttons not responding

1. Check button wiring to correct GPIO
2. Verify pull-up resistors (internal enabled)
3. Test button continuity with multimeter
4. Check serial monitor for button press logs

---

## Photos Reference

### Expected Setup

```
┌─────────────────────────────────────┐
│                                     │
│  ┌──────┐  ┌──────┐  ┌──────────┐  │
│  │ OLED │  │ RTC  │  │ DFPlayer │  │
│  │      │  │      │  │          │  │
│  └──┬───┘  └──┬───┘  └────┬─────┘  │
│     │         │           │         │
│     └─────────┴───────────┘         │
│                 │                   │
│          ┌──────┴──────┐            │
│          │   ESP32     │            │
│          │   DevKit    │            │
│          └─────────────┘            │
│                                     │
│  [BTN] [BTN] [BTN] [BTN] [BTN] [BTN]│
│                                     │
└─────────────────────────────────────┘
```

---

**Complete!** Ikut wiring diagram ini untuk setup iPray system dengan DFPlayer Mini! 🎵
