#include "AppConfig.h"

// Use the default on-board LED. For some ESP32 WROVER kits, this might be GPIO 2.
// If LED_BUILTIN doesn't work, try replacing it with '2'.
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(SERIAL_BAUD_RATE);
  
  // Set pin modes
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.println("System Check Initialized...");
}

void loop() {
  // Visual feedback: Blink the LED
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Audible feedback: Short beep
  tone(BUZZER_PIN, 1000, 200); // frequency 1000Hz, duration 200ms
  
  // Debug message
  Serial.println("Loop is running. LED ON, Buzzer Beep.");
  
  delay(2000); // Wait for 2 seconds
  
  // Turn off LED
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("LED OFF.");
  
  delay(2000); // Wait for 2 seconds
}