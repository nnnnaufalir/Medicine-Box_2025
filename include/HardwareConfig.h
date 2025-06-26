#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// Serial
#define SERIAL_BAUD_RATE 115200

// HX711 Load Cell
#define PIN_HX711_DOUT 27
#define PIN_HX711_SCK 26

// TFT Display (Hardware SPI)
#define PIN_TFT_CS 5
#define PIN_TFT_DC 4
#define PIN_TFT_RST 2
#define PIN_TFT_SCLK 18
#define PIN_TFT_MOSI 23

// Buzzer
#define PIN_BUZZER 15

// Buttons
#define PIN_BUTTON_1 34
#define PIN_BUTTON_2 35

// Battery Monitor
#define PIN_BATTERY_ADC 32

// RTC (I2C)
#define I2C_SDA 21
#define I2C_SCL 22

#endif // HARDWARE_CONFIG_H
