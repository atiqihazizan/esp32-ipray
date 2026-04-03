#!/bin/bash

echo "🔍 ESP32 Board Scanner"
echo "======================"
echo ""
echo "Pilih scanner:"
echo "1. I2C Scanner (detect OLED, RTC, sensors)"
echo "2. GPIO Scanner (detect buttons, pins)"
echo "3. Complete Board Detector (all-in-one)"
echo ""
read -p "Pilihan [1-3]: " choice

# Backup main.cpp
cp ../src/main.cpp ../src/main.cpp.backup

case $choice in
    1)
        echo "📡 Running I2C Scanner..."
        cp i2c_scanner.cpp ../src/main.cpp
        ;;
    2)
        echo "🔘 Running GPIO Scanner..."
        cp gpio_scanner.cpp ../src/main.cpp
        ;;
    3)
        echo "🔍 Running Complete Board Detector..."
        cp board_detector.cpp ../src/main.cpp
        ;;
    *)
        echo "❌ Invalid choice!"
        exit 1
        ;;
esac

# Upload
cd ..
echo ""
echo "Uploading to ESP32..."
pio run --target upload --target monitor

# Restore main.cpp
echo ""
echo "Restoring original main.cpp..."
mv src/main.cpp.backup src/main.cpp

echo "✅ Done!"
