# Setup Secrets File

## Kenapa Guna Secrets File?

✅ **Keselamatan** - Credentials tidak hardcode dalam code
✅ **Git-friendly** - secrets.h tidak di-commit ke repository
✅ **Mudah manage** - Satu file untuk semua credentials
✅ **Team-friendly** - Setiap developer boleh guna credentials sendiri

---

## Cara Setup

### 1. Copy Template

```bash
cp include/secrets.h.example include/secrets.h
```

### 2. Edit Credentials

Edit `include/secrets.h`:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID     "WiFi_Rumah_Anda"
#define WIFI_PASSWORD "password_wifi_anda"

// Solat Zone
#define SOLAT_ZONE    "PNG01"  // Tukar mengikut zon anda

#endif
```

### 3. Build & Upload

```bash
pio run --target upload
```

---

## Senarai Zon Waktu Solat

| Kod | Lokasi |
|-----|--------|
| PNG01 | Penang |
| WLY01 | Kuala Lumpur / Putrajaya / Labuan |
| JHR01 | Johor Bahru |
| KDH01 | Kedah |
| KTN01 | Kelantan |
| MLK01 | Melaka |
| NSN01 | Negeri Sembilan |
| PHG01 | Pahang |
| PRK01 | Perak |
| PLS01 | Perlis |
| SBH01 | Sabah |
| SGR01 | Selangor |
| SWK01 | Sarawak |
| TRG01 | Terengganu |

Senarai penuh: https://www.e-solat.gov.my/index.php?r=esolatApi/takwimsolat

---

## Git & Security

File `secrets.h` sudah ditambah dalam `.gitignore`, jadi ia **TIDAK akan di-commit** ke repository.

Ini bermakna:
- ✅ Credentials anda selamat
- ✅ Tidak terdedah dalam public repository
- ✅ Setiap developer boleh guna credentials sendiri

---

## Troubleshooting

### Error: secrets.h not found

```bash
# Pastikan file wujud
ls include/secrets.h

# Jika tidak wujud, copy dari template
cp include/secrets.h.example include/secrets.h
```

### WiFi tidak connect

1. Check SSID & password betul
2. Pastikan WiFi 2.4GHz (ESP32 tidak support 5GHz)
3. Check signal strength
4. Monitor serial: `screen /dev/cu.usbserial-0001 115200`

---

## Best Practices

1. **Jangan commit secrets.h** - Sudah di-ignore by default
2. **Guna WiFi 2.4GHz** - ESP32 tidak support 5GHz
3. **Check zon betul** - Pastikan zon waktu solat sesuai lokasi anda
4. **Backup secrets.h** - Simpan backup di tempat selamat (bukan di git!)

---

## Contoh Penggunaan

### Development
```cpp
#define WIFI_SSID     "WiFi_Rumah"
#define WIFI_PASSWORD "password123"
#define SOLAT_ZONE    "PNG01"
```

### Production
```cpp
#define WIFI_SSID     "WiFi_Masjid"
#define WIFI_PASSWORD "masjid_secure_pass"
#define SOLAT_ZONE    "WLY01"
```

### Testing
```cpp
#define WIFI_SSID     "WiFi_Test"
#define WIFI_PASSWORD "test123"
#define SOLAT_ZONE    "PNG01"
```

---

Selamat mencuba! 🔐
