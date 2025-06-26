#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// Konfigurasi Pin untuk semua komponen hardware

// --- Serial Debug ---
#define SERIAL_BAUD_RATE 115200

// --- Load Cell (HX711) ---
#define PIN_HX711_DOUT 13
#define PIN_HX711_SCK 14

// --- TFT Display (ILI9341) ---
// Koneksi via SPI
#define PIN_TFT_CS 26
#define PIN_TFT_DC 25
#define PIN_TFT_RST 27
#define PIN_TFT_MOSI 23
#define PIN_TFT_SCLK 18
#define PIN_TFT_MISO 19

// --- Buzzer ---
#define PIN_BUZZER 32

// --- Push Buttons ---
#define PIN_BUTTON_1 39 // Tombol untuk interaksi UI
#define PIN_BUTTON_2 36 // Tombol untuk mengaktifkan mode setup/web server

// --- Battery Monitor ---
#define PIN_BATTERY_ADC 34 // ADC Pin untuk membaca voltase baterai

// --- RTC (DS3231) & EEPROM (AT24C32) ---
// Koneksi via I2C
#define I2C_SDA 21
#define I2C_SCL 22

#endif // HARDWARE_CONFIG_H
