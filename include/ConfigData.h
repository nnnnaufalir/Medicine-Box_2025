#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <Arduino.h>
#include "Schedule_Manager/Schedule_Manager.h" // Diperlukan untuk struct MedicationSchedule
#include "KNN_Processor.h"                     // Diperlukan untuk struct KNNDataPoint

// Struktur data utama untuk menyimpan semua konfigurasi perangkat.
// Ini akan menjadi "cetakan" untuk data yang disimpan di EEPROM.
struct ConfigData
{
    // Penanda untuk verifikasi data di EEPROM.
    long magic_number = 123456789;

    // --- Data Konfigurasi ---
    float calibration_factor = 4119.755f; // Nilai default kalibrasi HX711
    char patient_name[50] = "Pasien";     // Nama pasien default

    // Jadwal minum obat
    int schedule_count = 0;           // Jumlah jadwal yang tersimpan
    MedicationSchedule schedules[10]; // Maksimal 10 jadwal

    // (Opsional) Dataset KNN untuk fleksibilitas di masa depan
    // int knn_dataset_size = 85;
    // KNNDataPoint knn_dataset[85];
};

#endif // CONFIG_DATA_H