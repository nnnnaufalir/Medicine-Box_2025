#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// --- Pin Konfigurasi Hardware ---

// Buzzer
#define BUZZER_PIN          32

// TFT Display (ILI9341)
// Sesuaikan dengan koneksi pin di ESP32 Anda
#define TFT_MOSI_PIN        23  // SPI MOSI (Master Out Slave In)
#define TFT_MISO_PIN        19  // SPI MISO (Master In Slave Out)
#define TFT_SCLK_PIN        18  // SPI SCLK (Serial Clock)
#define TFT_DC_PIN          25  // Data/Command (DC)
#define TFT_CS_PIN          26  // Chip Select (CS)
#define TFT_RST_PIN         27  // Reset (RST)
#define TFT_LED_PIN         33  // Pin untuk kontrol Backlight LED TFT 

// Sensor & Input
#define BATT_MONITOR_PIN    34  // Pin untuk monitoring tegangan baterai (ADC)
#define LOCK_CHECKER_PIN    35  // Pin untuk mendeteksi status kunci 
#define PB1_PIN             39  // Push Button 1
#define PB2_PIN             36  // Push Button 2

// HX711 Load Cell
#define HX711_SCK_PIN       14  // Serial Clock untuk HX711
#define HX711_DT_PIN        13  // Data pin untuk HX711

// --- Konfigurasi Umum ---
#define SERIAL_BAUD_RATE    115200 // Baud rate untuk Serial Monitor

// ... Anda bisa menambahkan konstanta konfigurasi lain di sini nanti
// Contoh: Konstan untuk kalibrasi HX711, threshold, dll.

#endif // HARDWARE_CONFIG_H