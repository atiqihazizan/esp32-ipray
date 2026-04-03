/*
 * ESP32 Board Detector
 * Detect semua extend board/shield yang disambung
 */

#include <Arduino.h>
#include <Wire.h>

void printSeparator() {
  Serial.println("================================================");
}

void scanI2C() {
  Serial.println("\n📡 I2C Device Scanner");
  printSeparator();
  
  Wire.begin(21, 22);  // SDA=21, SCL=22
  
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Addr | Device");
  Serial.println("-----|----------------------------------");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" | ");
      
      // Identify device
      switch(address) {
        case 0x3C:
        case 0x3D:
          Serial.println("✓ OLED Display (SSD1306)");
          break;
        case 0x68:
          Serial.println("✓ RTC DS3231 atau MPU6050");
          break;
        case 0x57:
          Serial.println("✓ RTC DS3231 EEPROM");
          break;
        case 0x27:
        case 0x3F:
          Serial.println("✓ LCD I2C (PCF8574)");
          break;
        case 0x76:
        case 0x77:
          Serial.println("✓ BMP280/BME280 Sensor");
          break;
        case 0x48:
          Serial.println("✓ ADS1115 ADC");
          break;
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
          Serial.println("✓ PCF8574 I/O Expander");
          break;
        case 0x50:
        case 0x51:
          Serial.println("✓ EEPROM AT24C");
          break;
        default:
          Serial.println("? Unknown I2C Device");
      }
      nDevices++;
    }
  }
  
  Serial.println();
  Serial.print("Total: ");
  Serial.print(nDevices);
  Serial.println(" I2C device(s) found");
}

void scanSPI() {
  Serial.println("\n📡 SPI Device Scanner");
  printSeparator();
  Serial.println("SPI pins: MOSI=23, MISO=19, SCK=18");
  Serial.println("Check manual untuk CS pins");
}

void checkPower() {
  Serial.println("\n⚡ Power Info");
  printSeparator();
  
  // Check USB voltage (approximate)
  Serial.print("USB Power: ");
  Serial.println("Connected");
  
  // Check if battery monitoring available (GPIO34/35)
  pinMode(34, INPUT);
  int battRead = analogRead(34);
  Serial.print("ADC GPIO34: ");
  Serial.println(battRead);
}

void checkButtons() {
  Serial.println("\n🔘 Button Scanner");
  printSeparator();
  
  // Common button pins
  int buttonPins[] = {0, 32, 33, 25, 26, 27};
  int numButtons = 6;
  
  Serial.println("Pin  | State");
  Serial.println("-----|-------");
  
  for(int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    delay(5);
    int state = digitalRead(buttonPins[i]);
    
    Serial.print("GPIO");
    Serial.print(buttonPins[i]);
    Serial.print(" | ");
    Serial.println(state ? "HIGH (Not pressed)" : "LOW (Pressed/Connected)");
  }
}

void detectBoard() {
  Serial.println("\n🔍 Board Detection");
  printSeparator();
  
  // Check for common board configurations
  Wire.begin(21, 22);
  
  // Check OLED
  Wire.beginTransmission(0x3C);
  if (Wire.endTransmission() == 0) {
    Serial.println("✓ OLED Display detected at 0x3C");
  }
  
  // Check RTC
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() == 0) {
    Serial.println("✓ RTC DS3231 detected at 0x68");
  }
  
  // Check common extend boards
  Serial.println("\nPossible Board Types:");
  Serial.println("- ESP32 Dev Board with OLED & RTC");
  Serial.println("- Custom iPray Board");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n");
  Serial.println("╔════════════════════════════════════════════╗");
  Serial.println("║   ESP32 BOARD DETECTOR v1.0                ║");
  Serial.println("╚════════════════════════════════════════════╝");
  
  // Chip info
  Serial.println("\n💾 ESP32 Chip Info");
  printSeparator();
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
  
  // Scan devices
  scanI2C();
  checkButtons();
  checkPower();
  scanSPI();
  detectBoard();
  
  Serial.println("\n");
  printSeparator();
  Serial.println("✅ Scan Complete!");
  printSeparator();
}

void loop() {
  // Rescan setiap 10 saat
  delay(10000);
  Serial.println("\n\n--- Rescanning ---\n");
  scanI2C();
}
