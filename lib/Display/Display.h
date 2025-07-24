#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// Enum untuk mengelola layar mana yang sedang aktif
enum ScreenState
{
    SCREEN_BOOT,
    SCREEN_HOME,
    SCREEN_POPUP
};

/**
 * @brief Menginisialisasi driver TFT dan menampilkan layar booting.
 * Harus dipanggil sekali di dalam setup().
 */
void initDisplay();

/**
 * @brief Fungsi loop untuk modul display.
 * Mengelola pembaruan layar dan transisi antar layar.
 * Harus dipanggil berulang kali oleh displayTask.
 */
void loopDisplay();

#endif // DISPLAY_H
