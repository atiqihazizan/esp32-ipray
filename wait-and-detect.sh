#!/bin/bash

echo "╔════════════════════════════════════════════╗"
echo "║   WAITING FOR ESP32 CONNECTION...          ║"
echo "╚════════════════════════════════════════════╝"
echo ""
echo "⏳ Menunggu ESP32 disambung..."
echo "   (Tekan Ctrl+C untuk cancel)"
echo ""

# Function to check ESP32
check_esp32() {
    ls /dev/cu.* 2>/dev/null | grep -E "(usbserial|SLAB|wch|usbmodem)" | head -1
}

# Wait for ESP32
counter=0
while true; do
    port=$(check_esp32)
    
    if [ ! -z "$port" ]; then
        echo ""
        echo "✅ ESP32 detected at: $port"
        echo ""
        
        # Update platformio.ini with detected port
        sed -i.bak "s|upload_port = .*|upload_port = $port|g" platformio.ini
        
        echo "🔍 Starting board detection..."
        sleep 2
        
        # Run detector
        ./detect-board.sh
        break
    fi
    
    # Show waiting animation
    case $((counter % 4)) in
        0) echo -ne "\r⏳ Waiting.   " ;;
        1) echo -ne "\r⏳ Waiting..  " ;;
        2) echo -ne "\r⏳ Waiting... " ;;
        3) echo -ne "\r⏳ Waiting...." ;;
    esac
    
    counter=$((counter + 1))
    sleep 1
done

echo ""
echo "✅ Done!"
