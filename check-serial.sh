#!/bin/bash

echo "📡 Capturing ESP32 serial output..."
echo "Press Ctrl+C to stop"
echo ""

pio device monitor --port /dev/cu.wchusbserial22220 --baud 115200
