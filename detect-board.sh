#!/bin/bash
# ESP32: board detector, --wait (USB), atau menu scanner I2C/GPIO.

cd "$(dirname "$0")"

show_help() {
    echo "ESP32 board tools"
    echo "  ./detect-board.sh           — full board detector (swap main, upload, monitor, restore)"
    echo "  ./detect-board.sh --wait    — tunggu USB, kemas kini upload_port dalam platformio.ini, lalu detector"
    echo "  ./detect-board.sh scan      — menu: I2C scanner / GPIO scanner / board detector"
    echo "  ./detect-board.sh --help    — papar bantuan ini"
}

check_esp32_port() {
    ls /dev/cu.* 2>/dev/null | grep -E "(usbserial|SLAB|wch|usbmodem)" | head -1
}

wait_for_connection() {
    echo "╔════════════════════════════════════════════╗"
    echo "║   WAITING FOR ESP32 CONNECTION...          ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""
    echo "⏳ Menunggu ESP32 disambung... (Ctrl+C untuk batal)"
    echo ""

    counter=0
    while true; do
        port=$(check_esp32_port)
        if [ -n "$port" ]; then
            echo ""
            echo "✅ ESP32 detected at: $port"
            sed -i.bak "s|upload_port = .*|upload_port = $port|g" platformio.ini
            echo "📝 platformio.ini: upload_port dikemas kini"
            sleep 1
            return 0
        fi
        case $((counter % 4)) in
            0) echo -ne "\r⏳ Waiting.   " ;;
            1) echo -ne "\r⏳ Waiting..  " ;;
            2) echo -ne "\r⏳ Waiting... " ;;
            3) echo -ne "\r⏳ Waiting...." ;;
        esac
        counter=$((counter + 1))
        sleep 1
    done
}

run_full_detector() {
    cleanup_detector() {
        if [ -f src/main.cpp.temp.bak ]; then
            mv -f src/main.cpp.temp.bak src/main.cpp
            echo "♻️  main.cpp dipulihkan."
        fi
    }
    trap cleanup_detector INT TERM

    echo "╔════════════════════════════════════════════╗"
    echo "║   ESP32 BOARD DETECTOR                     ║"
    echo "╚════════════════════════════════════════════╝"
    echo ""

    echo "📦 Backing up current code..."
    cp src/main.cpp src/main.cpp.temp.bak

    echo "🔍 Loading board detector..."
    cp test/board_detector.cpp src/main.cpp

    echo "🔨 Building..."
    if ! pio run > /dev/null 2>&1; then
        trap - INT TERM
        cleanup_detector
        echo "❌ Build failed!"
        exit 1
    fi
    echo "✅ Build success!"

    echo ""
    echo "📤 Uploading to ESP32..."
    if ! pio run --target upload; then
        trap - INT TERM
        cleanup_detector
        echo "❌ Upload failed!"
        exit 1
    fi

    echo ""
    echo "✅ Upload success!"
    echo ""
    echo "📺 Opening serial monitor... (Ctrl+C untuk keluar)"
    echo ""
    sleep 2
    pio device monitor || true

    trap - INT TERM
    echo ""
    if [ -f src/main.cpp.temp.bak ]; then
        echo "♻️  Restoring original code..."
        mv src/main.cpp.temp.bak src/main.cpp
    fi
    echo "✅ Done!"
}

run_scan_menu() {
    cleanup_scan() {
        if [ -f src/main.cpp.backup ]; then
            mv -f src/main.cpp.backup src/main.cpp
            echo "♻️  main.cpp dipulihkan."
        fi
    }
    trap cleanup_scan INT TERM

    echo "🔍 ESP32 Board Scanner"
    echo "======================"
    echo ""
    echo "1. I2C Scanner"
    echo "2. GPIO Scanner"
    echo "3. Complete Board Detector (sama seperti ./detect-board.sh)"
    echo ""
    read -p "Pilihan [1-3]: " choice

    cp src/main.cpp src/main.cpp.backup

    case $choice in
        1)
            echo "📡 Running I2C Scanner..."
            cp test/i2c_scanner.cpp src/main.cpp
            ;;
        2)
            echo "🔘 Running GPIO Scanner..."
            cp test/gpio_scanner.cpp src/main.cpp
            ;;
        3)
            echo "🔍 Running Complete Board Detector..."
            cp test/board_detector.cpp src/main.cpp
            ;;
        *)
            trap - INT TERM
            rm -f src/main.cpp.backup
            echo "❌ Pilihan tidak sah!"
            exit 1
            ;;
    esac

    echo ""
    echo "Uploading to ESP32..."
    pio run --target upload --target monitor || true

    trap - INT TERM
    echo ""
    if [ -f src/main.cpp.backup ]; then
        echo "Restoring original main.cpp..."
        mv src/main.cpp.backup src/main.cpp
    fi
    echo "✅ Done!"
}

case "${1:-}" in
    --help|-h)
        show_help
        ;;
    --wait)
        wait_for_connection
        echo ""
        run_full_detector
        ;;
    scan)
        run_scan_menu
        ;;
    "")
        run_full_detector
        ;;
    *)
        echo "❌ Arg tidak dikenali: $1"
        echo ""
        show_help
        exit 1
        ;;
esac
