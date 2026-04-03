# Detect Extend Board ESP32

Panduan untuk detect extend board/shield yang disambung ke ESP32.

---

## 🚀 Quick Start

```bash
./detect-board.sh
```

Script ini akan:
1. Backup kod semasa
2. Upload board detector
3. Scan semua devices
4. Restore kod asal

---

## 📡 Jenis Detection

### 1. I2C Devices

Scanner akan detect:
- **0x3C/0x3D** - OLED Display (SSD1306)
- **0x68** - RTC DS3231 / MPU6050
- **0x57** - RTC EEPROM
- **0x27/0x3F** - LCD I2C
- **0x76/0x77** - BMP280/BME280
- **0x48** - ADS1115 ADC
- **0x20-0x27** - PCF8574 I/O Expander

### 2. GPIO Pins

Scanner akan check:
- Button pins (0, 32, 33, 25, 26, 27)
- Input/Output states
- Pull-up/Pull-down status

### 3. Chip Info

Scanner akan show:
- Chip model & revision
- CPU frequency
- Flash size
- Free heap memory

---

## 🔧 Manual Detection

### I2C Scanner Sahaja

```bash
# Copy scanner
cp test/i2c_scanner.cpp src/main.cpp

# Upload
pio run -t upload -t monitor

# Restore
git checkout src/main.cpp
```

### GPIO Scanner Sahaja

```bash
# Copy scanner
cp test/gpio_scanner.cpp src/main.cpp

# Upload
pio run -t upload -t monitor

# Restore
git checkout src/main.cpp
```

---

## 📊 Expected Output

```
╔════════════════════════════════════════════╗
║   ESP32 BOARD DETECTOR v1.0                ║
╚════════════════════════════════════════════╝

💾 ESP32 Chip Info
================================================
Chip Model: ESP32-D0WD-V3
Chip Revision: 3
CPU Frequency: 240 MHz
Flash Size: 4 MB
Free Heap: 280 KB

📡 I2C Device Scanner
================================================
Addr | Device
-----|----------------------------------
0x3C | ✓ OLED Display (SSD1306)
0x57 | ✓ RTC DS3231 EEPROM
0x68 | ✓ RTC DS3231 atau MPU6050

Total: 3 I2C device(s) found

🔘 Button Scanner
================================================
Pin  | State
-----|-------
GPIO0  | HIGH (Not pressed)
GPIO32 | HIGH (Not pressed)
GPIO33 | HIGH (Not pressed)
GPIO25 | HIGH (Not pressed)
GPIO26 | HIGH (Not pressed)
GPIO27 | HIGH (Not pressed)

🔍 Board Detection
================================================
✓ OLED Display detected at 0x3C
✓ RTC DS3231 detected at 0x68

Possible Board Types:
- ESP32 Dev Board with OLED & RTC
- Custom iPray Board

================================================
✅ Scan Complete!
================================================
```

---

## 🔍 Common I2C Addresses

| Address | Device |
|---------|--------|
| 0x20-0x27 | PCF8574 I/O Expander |
| 0x3C, 0x3D | OLED SSD1306 |
| 0x48 | ADS1115 ADC |
| 0x50-0x57 | EEPROM |
| 0x68 | RTC DS3231, MPU6050 |
| 0x76, 0x77 | BMP280, BME280 |

---

## 🐛 Troubleshooting

### No I2C devices found

1. Check wiring:
   - SDA = GPIO 21
   - SCL = GPIO 22
   - VCC = 3.3V
   - GND = GND

2. Check pull-up resistors (4.7kΩ)
3. Try different I2C speed

### Device detected but not working

1. Check voltage (3.3V vs 5V)
2. Check library compatibility
3. Try different I2C address

### Scanner tidak upload

```bash
# Check port
ls /dev/cu.*

# Manual upload
pio run -t upload
```

---

## 📝 Notes

- Scanner akan auto-restore kod asal selepas scan
- Kod asal di-backup sebagai `src/main.cpp.temp.bak`
- Scanner akan rescan setiap 10 saat
- Tekan Ctrl+C untuk keluar dari monitor

---

## 🎯 Use Cases

### 1. New Board Setup
Guna scanner untuk identify semua components sebelum coding.

### 2. Troubleshooting
Guna scanner untuk verify hardware connections.

### 3. Documentation
Guna output scanner untuk document board configuration.

---

Selamat mencuba! 🔍
