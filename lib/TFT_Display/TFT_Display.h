#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

// --- Includes ---
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>   // Pustaka grafis dasar
#include "HardwareConfig.h" // Include file konfigurasi pin

// Deklarasikan instance Adafruit_ILI9341 di sini
extern Adafruit_ILI9341 tft; // Menggunakan 'extern' karena objek diinisialisasi di .cpp

// --- Warna Kustom (dari kode Anda, disesuaikan ke define) ---
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
    /**
     * @brief Menginisialisasi display TFT.
     * Harus dipanggil sekali di setup().
     */
    void begin();

    /**
     * @brief Menampilkan teks di posisi tertentu dengan warna dan ukuran.
     * @param text String teks yang akan ditampilkan.
     * @param x Koordinat X awal.
     * @param y Koordinat Y awal.
     * @param color Warna teks (format 16-bit).
     * @param size Ukuran font (default 1).
     */
    void printText(const char *text, int x, int y, uint16_t color, int size = 1);

    /**
     * @brief Membersihkan seluruh layar dengan warna tertentu.
     * @param color Warna latar belakang (default HITAM).
     */
    void clearScreen(uint16_t color = HITAM);

    /**
     * @brief Mengatur warna teks default untuk penulisan selanjutnya.
     * @param color Warna teks.
     */
    void setTextColor(uint16_t color);

    /**
     * @brief Mengatur ukuran teks default untuk penulisan selanjutnya.
     * @param size Ukuran font.
     */
    void setTextSize(uint8_t size);

    // --- Metode Halaman (Diadaptasi dari kode Anda) ---
    /**
     * @brief Menggambar tampilan utama (Home Screen).
     * @param jumlahObat Jumlah obat yang akan ditampilkan.
     */
    void drawHomeScreen(int jumlahObat = 0);

    /**
     * @brief Menggambar tampilan peringatan obat.
     * @param jumlahObat Jumlah obat saat ini untuk konteks peringatan.
     */
    void drawWarningScreen(int jumlahObat = 0);

    /**
     * @brief Menggambar tampilan informasi sistem.
     */
    void drawInformationScreen();

    /**
     * @brief Menggambar tampilan pengaturan.
     */
    void drawSettingsScreen();

    // --- Utilitas Gambar dari kode Anda
    /**
     * @brief Menggambar tombol dengan teks di tengah.
     * @param x Koordinat X kiri atas.
     * @param y Koordinat Y kiri atas.
     * @param width Lebar tombol.
     * @param height Tinggi tombol.
     * @param color Warna isi tombol.
     * @param borderColor Warna border tombol.
     * @param text Teks pada tombol.
     * @param textColor Warna teks pada tombol.
     */
    void drawButton(int x, int y, int width, int height, uint16_t color, uint16_t borderColor, const char *text, uint16_t textColor);

    /**
     * @brief Menggambar progress bar.
     * @param x Koordinat X kiri atas.
     * @param y Koordinat Y kiri atas.
     * @param width Lebar progress bar.
     * @param height Tinggi progress bar.
     * @param progress Nilai progres (0-100).
     * @param barColor Warna bar progres.
     * @param bgColor Warna latar belakang bar.
     */
    void drawProgressBar(int x, int y, int width, int height, int progress, uint16_t barColor, uint16_t bgColor);

    /**
     * @brief Menggambar angka besar di layar.
     * @param number Angka yang akan digambar.
     * @param x Koordinat X awal.
     * @param y Koordinat Y awal.
     * @param color Warna angka.
     */
    void drawLargeNumber(int number, int x, int y, uint16_t color);
};

#endif // TFT_DISPLAY_H