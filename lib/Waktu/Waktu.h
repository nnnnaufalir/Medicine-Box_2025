#ifndef WAKTU_H
#define WAKTU_H

#include <RTClib.h>

/**
 * @brief Menginisialisasi komunikasi dengan modul RTC DS3231.
 * Fungsi ini harus dipanggil sekali di dalam setup().
 */
void initWaktu();

/**
 * @brief Mendapatkan waktu dan tanggal saat ini dari RTC.
 * @return Sebuah objek DateTime yang berisi informasi waktu saat ini.
 */
DateTime getCurrentTime();

#endif // WAKTU_H
