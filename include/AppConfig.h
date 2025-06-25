#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// --- General Application Configuration ---

// EMA Filter constants
#define EMA_FILTER_ALPHA 0.6f
#define EMA_FILTER_RESET_VALUE 0.0f

// --- Load Cell Specific Configuration ---
#define WEIGHT_CALIBRATION 4119.755f
#define WEIGHT_SAMPLING 10
#define WEIGHT_TOLERANCE_G 1.2f
#define AUTO_TARE_DELAY_MS 5000

// KNN constants
#define KNN_K_VALUE 21

// Button debounce delay (milliseconds)
#define BUTTON_DEBOUNCE_MS 10

// Task/Sensor read intervals (milliseconds)
#define LOADCELL_READ_INTERVAL_MS 15
#define DISPLAY_UPDATE_INTERVAL_MS 50
#define BATTERY_CHECK_INTERVAL_MS 1000
#define SCHEDULE_CHECK_INTERVAL_MS 100

// --- RTC and EEPROM Configuration ---
#define EEPROM_SIZE_BYTES 4096
#define EEPROM_ADDR_CALIBRATION 0
#define EEPROM_ADDR_KNN_DATA 100
#define EEPROM_ADDR_SCHEDULE 300

#endif // APP_CONFIG_H