#include "TFT_Display.h" // Include header kita sendiri
#include <Arduino.h>     // Untuk Serial.println dan delay
#include "AppConfig.h"   // Untuk konstanta umum jika diperlukan

// Inisialisasi objek Adafruit_ILI9341 di luar kelas
// Menggunakan pin dari HardwareConfig.h
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN, TFT_SCLK_PIN, TFT_RST_PIN);

void TFT_Display::begin()
{
    Serial.println("TFT_Display: Menginisialisasi display...");
    tft.begin();
    tft.setRotation(1);    // Sesuaikan rotasi display (0, 1, 2, 3)
    tft.fillScreen(HITAM); // Layar awal hitam
    // Inisialisasi pin LED TFT jika digunakan
    pinMode(TFT_LED_PIN, OUTPUT);
    digitalWrite(TFT_LED_PIN, HIGH); // Nyalakan backlight (HIGH untuk aktif, sesuaikan jika LOW)
    Serial.println("TFT_Display: Inisialisasi selesai.");
}

void TFT_Display::printText(const char *text, int x, int y, uint16_t color, int size)
{
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.print(text);
}

void TFT_Display::clearScreen(uint16_t color)
{
    tft.fillScreen(color);
}

void TFT_Display::setTextColor(uint16_t color)
{
    tft.setTextColor(color);
}

void TFT_Display::setTextSize(uint8_t size)
{
    tft.setTextSize(size);
}

// --- Implementasi Metode Halaman (Diadaptasi dari kode Anda) ---

void TFT_Display::drawHomeScreen(int jumlahObat)
{
    clearScreen(BIRU_GELAP); // Background

    // Header
    tft.fillRect(0, 0, tft.width(), 40, BIRU); // Header bar
    printText("Kotak Obat Pintar", 10, 10, PUTIH, 2);

    // Box Jumlah Obat
    tft.drawRoundRect(10, 50, 220, 100, 8, PUTIH);
    printText("Jumlah Obat:", 20, 60, PUTIH, 2);
    drawLargeNumber(jumlahObat, 70, 90, KUNING_CERAH); // Posisi disesuaikan

    // Box Jadwal Berikutnya (placeholder)
    tft.drawRoundRect(10, 160, 220, 70, 8, PUTIH);
    printText("Jadwal Berikutnya:", 20, 170, PUTIH, 1);
    printText("Obat A - 08:00", 20, 190, HIJAU_GELAP, 2);

    // Footer
    tft.fillRect(0, tft.height() - 30, tft.width(), 30, ABU_ABU_TERANG);
    printText("Status: OK", 10, tft.height() - 25, HITAM, 1);
}

void TFT_Display::drawWarningScreen(int jumlahObat)
{
    clearScreen(MERAH); // Background peringatan merah!

    // Header
    tft.fillRect(0, 0, tft.width(), 40, HITAM);
    printText("Peringatan Obat!", 10, 10, PUTIH, 2);

    // Pesan Peringatan
    printText("Waktunya minum obat!", 20, 60, PUTIH, 2);
    printText("Segera Ambil Obat Anda.", 20, 90, PUTIH, 1);

    // Tampilkan jumlah obat saat ini (jika relevan)
    printText("Sisa Obat:", 20, 130, PUTIH, 2);
    drawLargeNumber(jumlahObat, 70, 160, KUNING_CERAH);

    // Tombol OK (contoh)
    drawButton(tft.width() / 2 - 50, tft.height() - 60, 100, 40, HIJAU, PUTIH, "OK", PUTIH);
}

void TFT_Display::drawInformationScreen()
{
    clearScreen(BIRU_LANGIT);
    tft.fillRect(0, 0, tft.width(), 40, BIRU);
    printText("Informasi Sistem", 10, 10, PUTIH, 2);
    printText("Versi: 1.0", 20, 60, HITAM, 1);
    printText("Serial No: ABC123DEF", 20, 80, HITAM, 1);
    printText("Baterai: XX%", 20, 100, HITAM, 1);
    // ... Tambahkan info lainnya
}

void TFT_Display::drawSettingsScreen()
{
    clearScreen(ABU_ABU_TERANG);
    tft.fillRect(0, 0, tft.width(), 40, UNGU_GELAP);
    printText("Pengaturan", 10, 10, PUTIH, 2);
    printText("1. Kalibrasi Loadcell", 20, 60, HITAM, 1);
    printText("2. Atur Jadwal", 20, 80, HITAM, 1);
    printText("3. Konfigurasi WiFi", 20, 100, HITAM, 1);
    // ... dst
}

// --- Implementasi Utilitas Gambar (Diadaptasi dari kode Anda) ---

void TFT_Display::drawButton(int x, int y, int width, int height, uint16_t color, uint16_t borderColor, const char *text, uint16_t textColor)
{
    tft.fillRoundRect(x, y, width, height, 5, color);
    tft.drawRoundRect(x, y, width, height, 5, borderColor);
    // Hitung posisi teks agar di tengah tombol
    tft.setTextSize(1); // Set ukuran default untuk perhitungan
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    printText(text, x + (width - w) / 2, y + (height - h) / 2 - y1, textColor, 1);
}

void TFT_Display::drawProgressBar(int x, int y, int width, int height, int progress, uint16_t barColor, uint16_t bgColor)
{
    tft.drawRoundRect(x, y, width, height, 3, PUTIH);   // Border
    tft.fillRoundRect(x, y, width, height, 3, bgColor); // Background

    int barWidth = map(progress, 0, 100, 0, width); // Map progress 0-100 to bar width
    if (barWidth > 0)
    {                                                           // Hanya gambar jika ada progress
        tft.fillRoundRect(x, y, barWidth, height, 3, barColor); // Progress bar
    }
}

void TFT_Display::drawLargeNumber(int number, int x, int y, uint16_t color)
{
    char numStr[10];
    sprintf(numStr, "%d", number);
    printText(numStr, x, y, color, 5); // Ukuran teks besar
}