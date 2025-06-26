#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// --- Filter Exponential Moving Average (EMA) ---
#define EMA_ALPHA 0.6f

// --- Algoritma K-Nearest Neighbors (KNN) ---
#define KNN_K 3

// --- Toleransi Berat & Stabilitas ---
#define WEIGHT_TOLERANCE 0.5f
#define STABLE_READING_THRESHOLD 0.1f

// --- Konfigurasi Tugas FreeRTOS ---
#define TASK_LOAD_CELL_INTERVAL_MS 200
#define TASK_UI_UPDATE_INTERVAL_MS 100
#define TASK_SYSTEM_MONITOR_INTERVAL_MS 500
#define TASK_SCHEDULER_INTERVAL_MS 1000

// --- Alokasi Alamat EEPROM (AT24C32) ---
#define EEPROM_ADDR_HX711_CAL_FACTOR 0
#define EEPROM_ADDR_KNN_DATASET_START 10
#define EEPROM_ADDR_SCHEDULE_START 200

// --- Konfigurasi Web Server ---
#define WIFI_SSID "KotakObatPintar_AP"
#define WIFI_PASSWORD "12345678"

#endif // APP_CONFIG_H
