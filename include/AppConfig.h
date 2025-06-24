#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// --- Konfigurasi Aplikasi Umum ---

// Konstanta untuk EMA Filter
#define EMA_FILTER_ALPHA 0.6f // Nilai alpha untuk EMA filter (0.0f - 1.0f) // Semakin kecil, semakin halus tapi lambat respon
#define EMA_FILTER_RESET 0.0f

// Konstanta untuk KNN
#define KNN_K_VALUE 15 // Nilai 'K' untuk algoritma K-Nearest Neighbors

// Konstanta untuk Debounce Tombol (dalam milidetik)
#define BUTTON_DEBOUNCE_MS 10 // Waktu tunda untuk debounce tombol

// Konstanta untuk Interval Pembacaan Sensor/Task (dalam milidetik)
#define LOADCELL_READ_INTERVAL_MS 10   // Interval pembacaan load cell dan proses KNN
#define DISPLAY_UPDATE_INTERVAL_MS 16  // Interval update display
#define BATTERY_CHECK_INTERVAL_MS 1000 // Interval cek baterai (5 detik)
#define SCHEDULE_CHECK_INTERVAL_MS 50  // Interval cek jadwal (1 detik)

// --- Konfigurasi RTC dan EEPROM
// Ukuran alokasi EEPROM untuk data tertentu
#define EEPROM_SIZE_BYTES 4096    // Total ukuran EEPROM yang akan digunakan
#define EEPROM_ADDR_CALIBRATION 0 // Alamat awal untuk data kalibrasi HX711
#define EEPROM_ADDR_KNN_DATA 100  // Alamat awal untuk dataset KNN
#define EEPROM_ADDR_SCHEDULE 300  // Alamat awal untuk data jadwal obat

// --- Toleransi (contoh, akan disesuaikan) ---
#define WEIGHT_CALIBRATION 4119.755f
#define WEIGHT_TOLERANCE_MG 50 // Toleransi berat dalam miligram untuk deteksi obat

#endif // APP_CONFIG_H