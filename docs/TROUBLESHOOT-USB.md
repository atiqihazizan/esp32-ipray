# Troubleshooting ESP32 USB Connection

## ❌ Masalah: ESP32 Port Tidak Muncul

### 🔍 Diagnosis

ESP32 sudah disambung tapi port tidak muncul di `/dev/cu.*`

---

## 🛠️ Penyelesaian

### 1. Check Driver USB-to-Serial

ESP32 menggunakan chip USB-to-Serial. Ada 2 jenis common:

#### **CH340/CH341 Driver** (Paling common untuk ESP32 murah)

```bash
# Download & install:
# https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver

# Atau guna Homebrew:
brew tap adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver
brew install --cask ch340g-ch34g-ch34x-mac-os-x-driver
```

#### **CP2102/CP2104 Driver** (Untuk ESP32 branded)

```bash
# Download dari Silicon Labs:
# https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

# Install .dmg file yang di-download
```

### 2. Restart Mac Selepas Install Driver

```bash
sudo reboot
```

### 3. Check Permission

```bash
# Check System Preferences > Security & Privacy
# Allow driver yang baru di-install
```

### 4. Check Kabel USB

- ✅ Guna kabel **DATA**, bukan charge-only
- ✅ Cuba kabel lain
- ✅ Pastikan kabel tidak rosak

### 5. Check Port USB Mac

- ✅ Cuba port USB lain
- ✅ Jika guna USB hub, cuba direct ke Mac
- ✅ Restart Mac

### 6. Check ESP32 Board

- ✅ LED power menyala?
- ✅ Cuba tekan button BOOT sambil plug in
- ✅ Cuba board ESP32 lain (jika ada)

---

## 🔎 Cara Verify Driver Installed

### Check CH340 Driver:

```bash
# Check if driver loaded
kextstat | grep ch34

# Should show something like:
# com.wch.usbserial (version)
```

### Check CP2102 Driver:

```bash
# Check if driver loaded
kextstat | grep silabs

# Should show something like:
# com.silabs.driver.CP210xVCPDriver
```

---

## 📋 Check USB Devices

```bash
# Method 1: Check /dev
ls /dev/cu.*

# Method 2: System profiler
system_profiler SPUSBDataType | grep -A 10 "Serial"

# Method 3: ioreg
ioreg -p IOUSB -l -w 0 | grep -i "serial\|uart"
```

---

## ✅ Expected Output Bila Berjaya

```bash
$ ls /dev/cu.*

/dev/cu.Bluetooth-Incoming-Port
/dev/cu.usbserial-0001          ← ESP32 port!
/dev/cu.debug-console
/dev/cu.wlan-debug
```

---

## 🎯 Specific ESP32 Board Types

### ESP32 DevKit V1 (NodeMCU-32S)
- Chip: CH340G
- Driver: CH340 driver required

### ESP32-WROOM-32
- Chip: CP2102 atau CH340
- Check board markings

### ESP32-S2/S3/C3
- Usually: CP2102 atau built-in USB
- S2/S3/C3 ada native USB, tak perlu driver

---

## 🔧 Advanced Troubleshooting

### 1. Reset NVRAM (macOS)

```bash
# Restart Mac dan tekan: Command + Option + P + R
# Tahan sehingga dengar startup sound 2 kali
```

### 2. Reset SMC (macOS Intel)

```bash
# Shutdown Mac
# Tekan: Shift + Control + Option + Power
# Tahan 10 saat, lepas, then power on
```

### 3. Check Kernel Extensions

```bash
# List all loaded kernel extensions
kextstat | grep -i usb

# Check for conflicts
kextstat | grep -i serial
```

### 4. Enable Verbose USB Logging

```bash
# Enable USB logging
sudo nvram boot-args="usb=0x800"

# Reboot
sudo reboot

# Check logs
log show --predicate 'process == "kernel"' --last 5m | grep -i usb
```

---

## 📱 Alternative: Use Arduino IDE

Jika masih tak jumpa port:

1. Install Arduino IDE
2. Tools > Board > ESP32 Arduino
3. Tools > Port - check if port appears
4. Arduino IDE sometimes has better driver detection

---

## 🆘 Last Resort

### 1. Try Windows/Linux

- Boot Windows via Boot Camp
- Try Linux Live USB
- Check if ESP32 detected there

### 2. Try Different ESP32 Board

- Board mungkin rosak
- USB-to-Serial chip mungkin rosak
- Cuba board lain

### 3. Use External USB-to-Serial Adapter

- Beli FTDI FT232RL adapter
- Sambung direct ke ESP32 TX/RX pins
- Bypass built-in USB chip

---

## 📞 Get Help

Jika masih tak berjaya:

1. **Check board markings** - ambil gambar chip USB-to-Serial
2. **Run diagnostics**: `./detect-board.sh` (bila port dah muncul)
3. **Post di forum**: 
   - ESP32 Forum: https://esp32.com
   - Reddit: r/esp32

---

## 🎓 Resources

- CH340 Driver: https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver
- CP2102 Driver: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- ESP32 Docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/

---

**Good luck!** 🍀
