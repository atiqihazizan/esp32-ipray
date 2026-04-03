#!/bin/bash

echo "🧪 DFPlayer Direct Test"
echo "======================="
echo ""
echo "This will test DFPlayer communication directly"
echo ""
echo "Expected behaviors:"
echo "1. If SD card OK: DFPlayer should initialize"
echo "2. If SD card fail: Will show error message"
echo ""
echo "Check serial monitor for results..."
echo ""

cd ~/IoT/esp32-ipray
./check-serial.sh
