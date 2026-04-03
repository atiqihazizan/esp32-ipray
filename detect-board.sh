#!/bin/bash

echo "╔════════════════════════════════════════════╗"
echo "║   ESP32 BOARD DETECTOR                     ║"
echo "╚════════════════════════════════════════════╝"
echo ""

# Backup current main.cpp
echo "📦 Backing up current code..."
cp src/main.cpp src/main.cpp.temp.bak

# Copy board detector
echo "🔍 Loading board detector..."
cp test/board_detector.cpp src/main.cpp

# Build and upload
echo "🔨 Building..."
pio run > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✅ Build success!"
    echo ""
    echo "📤 Uploading to ESP32..."
    pio run --target upload
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Upload success!"
        echo ""
        echo "📺 Opening serial monitor..."
        echo "   (Press Ctrl+C to exit)"
        echo ""
        sleep 2
        pio device monitor
    else
        echo "❌ Upload failed!"
    fi
else
    echo "❌ Build failed!"
fi

# Restore original main.cpp
echo ""
echo "♻️  Restoring original code..."
mv src/main.cpp.temp.bak src/main.cpp

echo "✅ Done!"
