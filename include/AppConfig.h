#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// --- Konfigurasi Aplikasi Umum ---

// Konstanta untuk EMA Filter
#define EMA_FILTER_ALPHA 0.6f // Nilai alpha untuk EMA filter (0.0f - 1.0f) // Semakin kecil, semakin halus tapi lambat respon
#define EMA_FILTER_RESET_VALUE 0.0f

// Konstanta untuk KNN
#define KNN_K_VALUE 21 // Nilai 'K' untuk algoritma K-Nearest Neighbors

// Konstanta untuk Debounce Tombol (dalam milidetik)
#define BUTTON_DEBOUNCE_MS 10 // Waktu tunda untuk debounce tombol

// Konstanta untuk Interval Pembacaan Sensor/Task (dalam milidetik)
#define LOADCELL_READ_INTERVAL_MS 15   // Interval pembacaan load cell dan proses KNN
#define DISPLAY_UPDATE_INTERVAL_MS 50  // Interval update display
#define BATTERY_CHECK_INTERVAL_MS 1000 // Interval cek baterai (5 detik)
#define SCHEDULE_CHECK_INTERVAL_MS 100 // Interval cek jadwal (1 detik)

// --- Konfigurasi RTC dan EEPROM
// Ukuran alokasi EEPROM untuk data tertentu
#define EEPROM_SIZE_BYTES 4096    // Total ukuran EEPROM yang akan digunakan
#define EEPROM_ADDR_CALIBRATION 0 // Alamat awal untuk data kalibrasi HX711
#define EEPROM_ADDR_KNN_DATA 100  // Alamat awal untuk dataset KNN
#define EEPROM_ADDR_SCHEDULE 300  // Alamat awal untuk data jadwal obat

// --- Toleransi (contoh, akan disesuaikan) ---
#define WEIGHT_CALIBRATION 4119.755f
#define WEIGHT_SAMPLING 10      // Jumlah sampel untuk rata-rata pembacaan load cell
#define WEIGHT_TOLERANCE_G 1.2f // Toleransi berat dalam gram untuk dianggap "nol" atau "kosong" (misal: +/- 0.1g)
#define AUTO_TARE_DELAY_MS 5000 // Waktu minimum (ms) load cell harus stabil di bawah nol threshold sebelum auto-tare

#endif // APP_CONFIG_H