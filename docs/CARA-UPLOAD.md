# Cara Upload ke ESP32

## ✅ Build Berjaya!

Projek anda sudah berjaya di-build. Saiz firmware:
- **RAM**: 14.7% (48,192 bytes)
- **Flash**: 77.3% (1,013,033 bytes)

---

## 📋 Langkah-langkah Upload

### 1. Sambung ESP32 ke Komputer

Sambungkan ESP32 anda menggunakan kabel USB.

### 2. Cari Port ESP32

```bash
# macOS
ls /dev/cu.*

# Cari port seperti:
# /dev/cu.usbserial-0001
# /dev/cu.SLAB_USBtoUART
# /dev/cu.wchusbserial*
```

### 3. Update platformio.ini (Optional)

Jika anda mahu set port tetap, uncomment dan update baris ini dalam `platformio.ini`:

```ini
upload_port = /dev/cu.usbserial-0001
monitor_port = /dev/cu.usbserial-0001
```

### 4. Upload ke ESP32

```bash
# Upload sahaja
pio run --target upload

# Upload + Monitor (disyorkan)
pio run --target upload --target monitor
```

### 5. Monitor Serial Output

Jika sudah upload, untuk monitor sahaja:

```bash
pio device monitor
```

Untuk keluar dari monitor: tekan `Ctrl + C`

---

## ⚙️ Configuration Penting

### WiFi Settings

Edit dalam `src/main.cpp` baris 15-16:

```cpp
const char *ssid     = "unifi_fiber_2G";      // ← Tukar dengan WiFi anda
const char *password = "0187669492";          // ← Tukar dengan password WiFi
```

### Zon Waktu Solat

Edit dalam `src/main.cpp` baris 144:

```cpp
strcpy(savedZone, "PNG01");  // ← Tukar mengikut zon anda
```

**Contoh Zon:**
- PNG01 - Penang
- WLY01 - Kuala Lumpur
- JHR01 - Johor Bahru
- KDH01 - Kedah
- KTN01 - Kelantan
- MLK01 - Melaka
- dll.

---

## 🔧 Troubleshooting

### Error: Port not found

1. Check sambungan USB
2. Install driver CH340/CP2102 jika perlu:
   - macOS: https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver
3. Cuba port lain: `ls /dev/cu.*`

### Error: Permission denied

```bash
# macOS/Linux - tambah permission
sudo chmod 666 /dev/cu.usbserial-*
```

### WiFi Tidak Connect

1. Pastikan SSID dan password betul
2. Pastikan WiFi 2.4GHz (bukan 5GHz)
3. Check signal strength
4. Monitor serial untuk debug: `pio device monitor`

### Build Error

```bash
# Clean dan build semula
pio run --target clean
pio run
```

---

## 📊 Serial Monitor Output

Apabila ESP32 running, anda akan nampak output seperti:

```
[wifi] connecting....
[wifi] connected, IP: 192.168.1.100
[ntp] syncing....
[ntp] RTC updated: 2026-03-31 12:30:45
[solat] fetching PNG01...
[solat] saved to NVS
[setup] selesai
```

---

## 🎯 Perintah Berguna

```bash
# Build sahaja (tanpa upload)
pio run

# Upload ke ESP32
pio run --target upload

# Monitor serial
pio device monitor

# Upload + Monitor (all-in-one)
pio run -t upload && pio device monitor

# Clean build
pio run --target clean

# List devices
pio device list

# Update libraries
pio pkg update
```

---

## 📝 Notes

- Firmware size: ~1MB (cukup untuk ESP32 4MB)
- Baud rate: 115200
- Upload speed: 921600 (boleh tukar dalam platformio.ini)
- Framework: Arduino
- Platform: ESP32

---

Selamat mencuba! 🚀
