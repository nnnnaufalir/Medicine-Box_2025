#include <HX711.h>
#include "AppConfig.h"

// Create an instance of the HX711 class
HX711 scale;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("HX711 Raw Reading Test");

  // Initialize the scale with the DOUT and SCK pins
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // scale.set_scale(LOADCELL_CALIBRATION_FACTOR);
  // scale.tare(20);
  Serial.println("Waiting for readings...");
  Serial.println("Press on the loadcell to see the value change.");
  scale.set_gain(64);
}

void loop() {
  // Check if the scale is ready with a new reading
  long reading = scale.read();
  float currentWeight = scale.get_units(2);
  Serial.print("Raw HX711 value: ");
  Serial.println(float(reading));
  Serial.print("gram: ");
  Serial.println(currentWeight);

  delay(200);  // Wait a moment before the next read
}
