#ifndef HX711_MODULE_H
#define HX711_MODULE_H

// --- Includes ---
#include <Arduino.h>     // Untuk tipe data dasar seperti float, int
#include <HX711.h>       // Pustaka eksternal untuk HX711
#include "HardwareConfig.h" // Untuk definisi pin HX711

// --- Kelas HX711_Module ---
class HX711_Module {
public:
    /**
     * @brief Menginisialisasi modul HX711.
     * Memulai komunikasi dengan sensor HX711.
     * @param doutPin Pin data (DOUT) dari HX711.
     * @param sckPin Pin clock (SCK) dari HX711.
     */
    void begin(int doutPin = HX711_DT_PIN, int sckPin = HX711_SCK_PIN);

    /**
     * @brief Menetapkan faktor skala kalibrasi untuk HX711.
     * @param scale Faktor skala yang didapat dari kalibrasi.
     */
    void setCalibrationFactor(float scale);

    /**
     * @brief Mengatur nilai offset (tare) untuk pembacaan nol.
     * Call setelah tidak ada beban di load cell.
     */
    void tare();

    /**
     * @brief Mengambil nilai berat yang sudah dikalibrasi.
     * @param times Jumlah pembacaan rata-rata untuk kestabilan.
     * @return Berat dalam unit yang dikalibrasi (misal: gram).
     */
    float getCalibratedWeight(byte times = 10);

    /**
     * @brief Mengambil nilai mentah dari HX711.
     * @param times Jumlah pembacaan rata-rata.
     * @return Nilai mentah dari sensor.
     */
    long getRawValue(byte times = 5);

    /**
     * @brief Memeriksa apakah sensor HX711 siap untuk dibaca.
     * @return true jika siap, false jika tidak.
     */
    bool isReady();

    // --- Metode Kalibrasi (Opsional, bisa dilakukan terpisah) ---
    // Metode ini bisa digunakan dalam mode kalibrasi terpisah jika diperlukan
    // atau untuk menyimpan dan memuat faktor kalibrasi dari EEPROM.

    /**
     * @brief Mendapatkan faktor skala saat ini.
     * @return Faktor skala yang digunakan.
     */
    float getScale();

    /**
     * @brief Mendapatkan offset saat ini.
     * @return Nilai offset yang digunakan.
     */
    long getOffset();


private:
    HX711 _scale; // Instance dari pustaka eksternal HX711
    float _calibrationFactor; // Faktor kalibrasi yang akan digunakan
};

#endif // HX711_MODULE_H