# 🎵 Generate Audio Files untuk DFPlayer Mini

## Quick Method: Guna Online Tools

### 1. Generate Beep Tones

**Website**: https://www.szynalski.com/tone-generator/

#### 001.mp3 - Warning Beep (880Hz)
1. Go to tone generator
2. Set frequency: **880 Hz**
3. Play and record for **0.5 seconds**
4. Save as `001.mp3`

#### 002.mp3 - Azan Beep (1200Hz)
1. Set frequency: **1200 Hz**
2. Record for **1.0 second**
3. Save as `002.mp3`

#### 003.mp3 - Hourly Beep (600Hz)
1. Set frequency: **600 Hz**
2. Record for **0.3 seconds**
3. Save as `003.mp3`

#### 004.mp3 - Startup Sound (1000Hz)
1. Set frequency: **1000 Hz**
2. Record for **1.0 second**
3. Save as `004.mp3`

---

## Alternative: Guna FFmpeg (Command Line)

Jika ada `ffmpeg` installed:

```bash
# Install ffmpeg (macOS)
brew install ffmpeg

# Generate 001.mp3 - Warning beep (880Hz, 0.5s)
ffmpeg -f lavfi -i "sine=frequency=880:duration=0.5" -b:a 128k 001.mp3

# Generate 002.mp3 - Azan beep (1200Hz, 1.0s)
ffmpeg -f lavfi -i "sine=frequency=1200:duration=1.0" -b:a 128k 002.mp3

# Generate 003.mp3 - Hourly beep (600Hz, 0.3s)
ffmpeg -f lavfi -i "sine=frequency=600:duration=0.3" -b:a 128k 003.mp3

# Generate 004.mp3 - Startup sound (1000Hz, 1.0s)
ffmpeg -f lavfi -i "sine=frequency=1000:duration=1.0" -b:a 128k 004.mp3
```

---

## Download Azan Audio (005.mp3)

### Option 1: YouTube

1. Search: "Azan MP3" atau "Adhan audio"
2. Guna YouTube downloader: https://ytmp3.cc/
3. Download as MP3
4. Rename to `005.mp3`

### Option 2: Islamic Audio Sites

- https://www.islamicfinder.org/prayer-times/
- https://www.islamcan.com/audio/azan/
- https://archive.org/ (search "azan")

### Option 3: Record Sendiri

Guna smartphone:
1. Record azan dari radio/TV
2. Transfer ke computer
3. Convert to MP3 (if needed)
4. Rename to `005.mp3`

---

## Setup SD Card Structure

```bash
# Create folder structure
mkdir -p /Volumes/SD_CARD/mp3

# Copy files
cp 001.mp3 /Volumes/SD_CARD/mp3/
cp 002.mp3 /Volumes/SD_CARD/mp3/
cp 003.mp3 /Volumes/SD_CARD/mp3/
cp 004.mp3 /Volumes/SD_CARD/mp3/
cp 005.mp3 /Volumes/SD_CARD/mp3/  # Optional

# Verify
ls -lh /Volumes/SD_CARD/mp3/
```

Expected output:
```
001.mp3  (warning beep)
002.mp3  (azan beep)
003.mp3  (hourly beep)
004.mp3  (startup sound)
005.mp3  (full azan - optional)
```

---

## Audio Specifications

| File     | Frequency | Duration | Usage                    |
|----------|-----------|----------|--------------------------|
| 001.mp3  | 880 Hz    | 0.5s     | Warning, button press    |
| 002.mp3  | 1200 Hz   | 1.0s     | Azan notification (5x)   |
| 003.mp3  | 600 Hz    | 0.3s     | Hourly notification      |
| 004.mp3  | 1000 Hz   | 1.0s     | Startup sound            |
| 005.mp3  | Variable  | 3-5 min  | Full azan (optional)     |

---

## Test Audio Files

Sebelum copy ke SD card, test dulu di computer:

```bash
# Play di macOS
afplay 001.mp3
afplay 002.mp3
afplay 003.mp3
afplay 004.mp3
afplay 005.mp3
```

Pastikan semua files boleh play dengan baik!

---

## Troubleshooting

### ❌ File tidak play di DFPlayer

**Solution:**
1. Convert to standard MP3 format:
   ```bash
   ffmpeg -i input.mp3 -b:a 128k -ar 44100 output.mp3
   ```

2. Ensure 3-digit naming: `001.mp3`, `002.mp3`, etc.

3. Check file size (recommended < 5MB per file)

### ❌ SD card tidak detect

1. Reformat to FAT32
2. Try different SD card reader
3. Check SD card capacity (max 32GB)

---

**Ready!** Sekarang anda ada semua audio files untuk DFPlayer Mini! 🎵
