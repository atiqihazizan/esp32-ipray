/*
 * GPIO Scanner untuk ESP32
 * Test semua GPIO pins untuk detect extend board
 */

#include <Arduino.h>

// GPIO pins yang boleh guna di ESP32
int gpioPins[] = {0, 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39};
int numPins = sizeof(gpioPins) / sizeof(gpioPins[0]);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== GPIO Scanner ===");
  Serial.println("Testing GPIO pins...\n");
  
  Serial.println("Pin | Mode      | State");
  Serial.println("----|-----------|-------");
  
  for(int i = 0; i < numPins; i++) {
    int pin = gpioPins[i];
    
    // Test as input with pullup
    pinMode(pin, INPUT_PULLUP);
    delay(10);
    int state = digitalRead(pin);
    
    Serial.print("GPIO");
    if(pin < 10) Serial.print(" ");
    Serial.print(pin);
    Serial.print(" | INPUT_PU  | ");
    Serial.println(state ? "HIGH" : "LOW");
  }
  
  Serial.println("\n=== Scan Complete ===");
  Serial.println("\nNote: LOW dengan INPUT_PULLUP bermakna ada button/sensor disambung");
}

void loop() {
  delay(5000);
}
