#pragma once

// --- Pinout Perangkat Keras ---
// Load Cell
const int LOADCELL_DOUT_PIN = 13;
const int LOADCELL_SCK_PIN = 14;

// TFT ILI9341 (Hardware SPI - VSPI)
const int TFT_CS = 26;
const int TFT_DC = 25;
const int TFT_RST = 27;
const int TFT_MOSI = 23;
const int TFT_MISO = 19;
const int TFT_CLK = 18;

// Perangkat I/O
const int BUZZER_PIN = 32;
const int BATTERY_MONITOR_PIN = 34;

// I2C
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;
const uint8_t EEPROM_ADDRESS = 0x57;

// --- Konfigurasi Sistem ---
const long SERIAL_BAUD_RATE = 115200;

// --- Konfigurasi Jaringan (Web Server) ---
const char *WIFI_SSID = "KotakObat_ESP32";
const char *WIFI_PASSWORD = "password123";
