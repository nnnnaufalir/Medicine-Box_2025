#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// --- Hardware Pin Configuration ---

#define BUZZER_PIN 32

// TFT Display (ILI9341) Pins
#define TFT_MOSI_PIN 23
#define TFT_MISO_PIN 19
#define TFT_SCLK_PIN 18
#define TFT_DC_PIN 25
#define TFT_CS_PIN 26
#define TFT_RST_PIN 27
#define TFT_LED_PIN 33

// Sensor & Input Pins
#define BATT_MONITOR_PIN 34
#define LOCK_CHECKER_PIN 35
#define PB1_PIN 39
#define PB2_PIN 36

// HX711 Load Cell Pins
#define HX711_SCK_PIN 14
#define HX711_DT_PIN 13

// --- General Hardware Configuration ---
#define SERIAL_BAUD_RATE 115200

#endif // HARDWARE_CONFIG_H