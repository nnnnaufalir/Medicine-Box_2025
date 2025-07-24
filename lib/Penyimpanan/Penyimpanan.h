#ifndef PENYIMPANAN_H
#define PENYIMPANAN_H

#include <Arduino.h>

// Struktur untuk menyimpan satu jadwal alarm
struct TimeSchedule
{
    uint8_t hour;
    uint8_t minute;
    bool enabled; // Untuk mengaktifkan/menonaktifkan jadwal ini
};

// Struktur utama untuk menampung semua pengaturan pengguna
struct UserSettings
{
    char patientName[32];      // Nama pasien, maks 31 karakter
    int dose;                  // Dosis obat per minum
    TimeSchedule schedules[3]; // Mendukung hingga 3 jadwal alarm
    char wifi_ssid[32];        // Nama jaringan WiFi untuk mode AP
    char wifi_password[64];    // Kata sandi jaringan WiFi
};

/**
 * @brief Menginisialisasi komunikasi dengan modul EEPROM AT24C32.
 */
void initPenyimpanan();

/**
 * @brief Menyimpan struct UserSettings ke EEPROM.
 * @param settings Struct yang berisi data untuk disimpan.
 */
void saveSettings(const UserSettings &settings);

/**
 * @brief Memuat struct UserSettings dari EEPROM.
 * @return Sebuah struct UserSettings yang berisi data yang telah dimuat.
 */
UserSettings loadSettings();

#endif // PENYIMPANAN_H
