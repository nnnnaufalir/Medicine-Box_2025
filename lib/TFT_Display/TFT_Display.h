#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

// --- Includes ---
#include <Adafruit_ILI9341.h> // Pustaka driver ILI9341
#include <Adafruit_GFX.h>     // Pustaka grafis dasar
#include "HardwareConfig.h"   // Include file konfigurasi pin

// Deklarasikan instance Adafruit_ILI9341 di sini
extern Adafruit_ILI9341 tft; // Gunakan 'extern' agar tft bisa diakses di .cpp

// --- Warna Kustom ---
// Anda bisa mendefinisikan warna kustom di sini atau di AppConfig.h
// Ini adalah contoh dari kode Anda, diubah ke format define
#define HITAM 0x0000
#define BIRU_GELAP 0x0003
#define HIJAU_GELAP 0x0200
#define KUNING_CERAH 0xFFE0
#define MERAH 0xF800
#define HIJAU 0x07E0
#define BIRU 0x001F
#define PUTIH 0xFFFF
#define ABU_ABU_TERANG 0xC618
#define UNGU_GELAP 0x301F
#define BIRU_LANGIT 0x8EDD
#define JINGGA 0xFB20

class TFT_Display
{
public:
    // Metode untuk menginisialisasi display
    void begin();

    // Metode untuk menampilkan teks (standar)
    void printText(const char *text, int x, int y, uint16_t color, int size = 1);

    // Metode untuk membersihkan layar
    void clearScreen(uint16_t color = HITAM); // Menggunakan warna kustom HITAM

    // Metode untuk mengatur warna teks default
    void setTextColor(uint16_t color);

    // Metode untuk mengatur ukuran teks default
    void setTextSize(uint8_t size);

    // --- Metode Halaman (Diadaptasi dari kode Anda) ---
    void drawHomeScreen(int jumlahObat = 0);    // Menambahkan parameter jumlahObat
    void drawWarningScreen(int jumlahObat = 0); // Menambahkan parameter jumlahObat
    void drawInformationScreen();
    void drawSettingsScreen(); // Contoh tambahan jika ada setting

    // --- Utilitas Gambar dari kode Anda (Diadaptasi sebagai metode) ---
    void drawButton(int x, int y, int width, int height, uint16_t color, uint16_t borderColor, const char *text, uint16_t textColor);
    void drawProgressBar(int x, int y, int width, int height, int progress, uint16_t barColor, uint16_t bgColor);
    void drawLargeNumber(int number, int x, int y, uint16_t color);

private:
    // Tidak ada variabel private yang spesifik untuk halaman, karena mereka stateless.
    // Variabel internal tft_obj sudah diurus oleh pustaka Adafruit_ILI9341.
};

#endif // TFT_DISPLAY_H