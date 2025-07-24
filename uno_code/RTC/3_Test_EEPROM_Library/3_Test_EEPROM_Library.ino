#include <Wire.h>
#include <extEEPROM.h>
#include "AppConfig.h"

// A struct to hold all our settings data in one place.
// This makes it easy to read and write all settings at once.
struct SettingsData {
  float calibrationFactor;
  int medicineCount;
  // You can add more variables here in the future
};

// Initialize the EEPROM object
extEEPROM eeprom(kbits_32, 1, 32, EEPROM_ADDRESS);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  Serial.println("\nEEPROM Test (Multiple Values using Struct)...");

  byte i2cStat = eeprom.begin(eeprom.twiClock100kHz);
  if (i2cStat != 0) {
    Serial.print("ERROR: eeprom.begin() failed, status = ");
    Serial.println(i2cStat);
    while (1); // Halt
  }

  // // --- Data to be written ---
  // SettingsData dataToWrite;
  // dataToWrite.calibrationFactor = LOADCELL_CALIBRATION_FACTOR;
  // dataToWrite.medicineCount = 25; // An example integer value
  
  uint16_t memoryAddress = 0;

  // // --- Writing the entire struct to EEPROM ---
  // Serial.println("Writing struct to EEPROM...");
  // Serial.print(" -> Calibration Factor: ");
  // Serial.println(dataToWrite.calibrationFactor, 3);
  // Serial.print(" -> Medicine Count: ");
  // Serial.println(dataToWrite.medicineCount);
  
  // i2cStat = eeprom.write(memoryAddress, (byte*)&dataToWrite, sizeof(dataToWrite));
  // if (i2cStat != 0) {
  //   Serial.print("ERROR during write, status = ");
  //   Serial.println(i2cStat);
  // } else {
  //   Serial.println("Write complete.");
  // }
  
  // Serial.println("------------------------------------");
  // delay(1000);

  // --- Reading the entire struct from EEPROM ---
  SettingsData dataRead; // Create a new, empty struct to read into
  Serial.println("Reading struct from EEPROM...");

  i2cStat = eeprom.read(memoryAddress, (byte*)&dataRead, sizeof(dataRead));
  if (i2cStat != 0) {
    Serial.print("ERROR during read, status = ");
    Serial.println(i2cStat);
  }

  Serial.println("Values read:");
  Serial.print(" -> Calibration Factor: ");
  Serial.println(dataRead.calibrationFactor, 3);
  Serial.print(" -> Medicine Count: ");
  Serial.println(dataRead.medicineCount);

  // // --- Verification ---
  // if (abs(dataRead.calibrationFactor - dataToWrite.calibrationFactor) < 0.001 &&
  //     dataRead.medicineCount == dataToWrite.medicineCount) {
  //   Serial.println("\nSUCCESS: Data matches. Struct read/write is working.");
  // } else {
  //   Serial.println("\nERROR: Data mismatch.");
  // }
}

void loop() {
  // Test runs only once in setup()
}
