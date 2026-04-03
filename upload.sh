#!/bin/bash

# ESP32 Upload Helper Script
# Memudahkan proses upload ke ESP32

echo "🚀 ESP32 iPray Upload Helper"
echo "=============================="
echo ""

# Function to list available ports
list_ports() {
    echo "📡 Port yang tersedia:"
    ls /dev/cu.* 2>/dev/null | grep -E "(usbserial|SLAB|wch)" || echo "   Tiada port ditemui!"
    echo ""
}

# Function to build only
build_only() {
    echo "🔨 Building projek..."
    pio run
}

# Function to upload
upload_esp32() {
    echo "📤 Uploading ke ESP32..."
    pio run --target upload
}

# Function to upload and monitor
upload_monitor() {
    echo "📤 Uploading ke ESP32 dan monitor serial..."
    pio run --target upload --target monitor
}

# Function to monitor only
monitor_only() {
    echo "📺 Monitoring serial output..."
    echo "   (Tekan Ctrl+C untuk keluar)"
    pio device monitor
}

# Function to clean build
clean_build() {
    echo "🧹 Cleaning build files..."
    pio run --target clean
    echo "✅ Clean selesai!"
}

# Main menu
echo "Pilih operasi:"
echo "1. List ports yang tersedia"
echo "2. Build sahaja (tanpa upload)"
echo "3. Upload ke ESP32"
echo "4. Upload + Monitor"
echo "5. Monitor sahaja"
echo "6. Clean build"
echo "7. Keluar"
echo ""
read -p "Pilihan [1-7]: " choice

case $choice in
    1)
        list_ports
        ;;
    2)
        build_only
        ;;
    3)
        list_ports
        upload_esp32
        ;;
    4)
        list_ports
        upload_monitor
        ;;
    5)
        monitor_only
        ;;
    6)
        clean_build
        ;;
    7)
        echo "👋 Selamat tinggal!"
        exit 0
        ;;
    *)
        echo "❌ Pilihan tidak sah!"
        exit 1
        ;;
esac

echo ""
echo "✅ Selesai!"
