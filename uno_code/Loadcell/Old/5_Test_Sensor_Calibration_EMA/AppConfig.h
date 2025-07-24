#pragma once // Prevents the file from being included multiple times

// --- HX711 Loadcell Pins ---
const int LOADCELL_DOUT_PIN = 13;
const int LOADCELL_SCK_PIN = 14;

// --- TFT ILI9341 Pins (VSPI) ---
const int TFT_CS_PIN = 25;
const int TFT_DC_PIN = 26;
const int TFT_RST_PIN = 27;
const int TFT_MOSI_PIN = 23;
const int TFT_MISO_PIN = 19;
const int TFT_SCK_PIN = 18;

// --- I/O Device Pins ---
const int BUZZER_PIN = 32;
const int BATTERY_MONITOR_PIN = 34; // ADC1_CH6, input only

// --- I2C Configuration ---
// Default ESP32 I2C pins are GPIO 21 (SDA) and GPIO 22 (SCL)
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;
const uint8_t RTC_ADDRESS = 0x68;
const uint8_t EEPROM_ADDRESS = 0x57;

// --- System Constants ---
const long SERIAL_BAUD_RATE = 115200;
const float LOADCELL_CALIBRATION_FACTOR = 4119.755f;
const float EMA_ALPHA = 0.1f; // Smoothing factor for the EMA filter
