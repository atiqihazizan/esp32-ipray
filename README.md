# ESP32 iPray - Waktu Solat Display

Projek ESP32 untuk paparan waktu solat dengan OLED SSD1306 dan RTC DS3231.

## 🚀 Quick Start

### 1. Setup Secrets File

```bash
# Copy template
cp include/secrets.h.example include/secrets.h

# Edit secrets.h dengan WiFi & zon anda
```

Edit `include/secrets.h`:

```cpp
#define WIFI_SSID     "WIFI_ANDA"
#define WIFI_PASSWORD "PASSWORD_ANDA"
#define SOLAT_ZONE    "PNG01"  // PNG01=Penang, WLY01=KL, dll
```

### 2. Upload ke ESP32

```bash
# Cara mudah - guna script
./upload.sh

# Atau manual
pio run --target upload --target monitor
```

## 📍 Zon Waktu Solat

| Kod | Negeri | Kod | Negeri |
|-----|--------|-----|--------|
| PNG01 | Penang | KTN01 | Kelantan |
| WLY01 | KL/Putrajaya | MLK01 | Melaka |
| JHR01 | Johor | PHG01 | Pahang |
| KDH01 | Kedah | SGR01 | Selangor |

[Senarai penuh zon](https://www.e-solat.gov.my/index.php?r=esolatApi/takwimsolat)

## 🔧 Hardware

- ESP32 Dev Board
- OLED SSD1306 (128x64, I2C)
- RTC DS3231 (I2C)
- **DFPlayer Mini MP3-TF-16P** (optional, untuk audio dari SD card)
- **Speaker 3W-5W** (jika guna DFPlayer)
- Buzzer Passive (backup jika tiada DFPlayer)
- 6x Push Buttons

### Pin Configuration

```
I2C: SDA=GPIO21, SCL=GPIO22
DFPlayer: RX=GPIO16, TX=GPIO17
Buttons: MENU=15, UP=4, DOWN=14, SET=27, RET=5, LAYOUT=23
Buzzer: GPIO13 (backup)
```

### 🔊 Audio Setup (DFPlayer Mini)

Untuk setup DFPlayer Mini dengan SD card dan speaker, lihat:
- **[DFPlayer Setup Guide](docs/DFPLAYER-SETUP.md)** - Wiring & configuration
- **[Generate Audio Files](docs/GENERATE-AUDIO.md)** - Create MP3 files

**Quick setup:**
1. Wire ESP32 GPIO16/17 to DFPlayer RX/TX
2. Connect speaker to DFPlayer SPK_1/SPK_2
3. Format SD card to FAT32
4. Di punca kad SD (ROOT): `001.mp3`–`006.mp3` — beep, notify, short×2, azan, full (lihat `config.h`)
5. Insert SD card dan upload code

System akan auto fallback ke buzzer jika DFPlayer tidak ready.

## 💡 Commands

```bash
pio run              # Build
pio run -t upload    # Upload
pio device monitor   # Monitor serial
./upload.sh          # Helper script
```

## 🐛 Troubleshooting

**Port not found:**
```bash
ls /dev/cu.*  # Check port
sudo chmod 666 /dev/cu.usbserial-*  # Fix permission
```

**Build error:**
```bash
pio run --target clean && pio run
```

**WiFi tidak connect:**
- Pastikan WiFi 2.4GHz (bukan 5GHz)
- Check credentials dalam `src/main.cpp`

## 📊 Build Info

- Firmware: 996 KB
- RAM: 14.7% (48 KB)
- Flash: 77.3% (1 MB)
- Platform: ESP32 Arduino

## 📚 Dokumentasi Lanjut

| Fail | Keterangan |
|------|------------|
| [docs/QUICK-START.md](docs/QUICK-START.md) | Mula cepat (3 langkah) |
| [docs/TROUBLESHOOT-USB.md](docs/TROUBLESHOOT-USB.md) | Masalah port USB / driver |
| [docs/CARA-UPLOAD.md](docs/CARA-UPLOAD.md) | Panduan upload |
| Folder [docs/](docs/) | Setup DFPlayer, wiring, dll. |

## 🔗 Credits

Original project: [Wokwi Simulator](https://wokwi.com/projects/459628503419433985)
