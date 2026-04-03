# ⚡ Quick Start

## 3 Langkah Mudah

### 1. Setup Secrets

```bash
cp include/secrets.h.example include/secrets.h
```

Edit `include/secrets.h`:

```cpp
#define WIFI_SSID     "WIFI_ANDA"
#define WIFI_PASSWORD "PASSWORD_ANDA"
#define SOLAT_ZONE    "PNG01"
```

### 2. Sambung ESP32

```bash
ls /dev/cu.*  # Check port
```

### 3. Upload

```bash
./upload.sh
# Pilih: 4 (Upload + Monitor)
```

## Perintah Berguna

```bash
pio run                 # Build
pio run -t upload       # Upload
pio device monitor      # Monitor
./upload.sh             # Helper menu
```

## Zon Solat

PNG01=Penang | WLY01=KL | JHR01=Johor | KDH01=Kedah | KTN01=Kelantan | MLK01=Melaka | SGR01=Selangor | PHG01=Pahang

---

**Status:** ✅ Ready to upload (996KB firmware)
