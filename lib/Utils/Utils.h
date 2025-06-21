#ifndef UTILS_H
#define UTILS_H

// --- Includes ---
#include <Arduino.h> // Untuk tipe data dasar seperti float

// --- Kelas EMAFilter ---
class EMAFilter {
public:
    /**
     * @brief Konstruktor untuk EMAFilter.
     * @param alpha Faktor penghalusan (0.0f - 1.0f). Semakin kecil, semakin halus.
     */
    EMAFilter(float alpha = 0.1f);

    /**
     * @brief Memfilter nilai baru menggunakan EMA.
     * @param newValue Nilai input yang akan difilter.
     * @return Nilai yang sudah difilter.
     */
    float filter(float newValue);

    /**
     * @brief Mengatur ulang filter dengan nilai awal tertentu.
     * @param initialValue Nilai awal untuk _filteredValue.
     */
    void reset(float initialValue);

    /**
     * @brief Mengatur ulang filter dan faktor alpha.
     * @param initialValue Nilai awal untuk _filteredValue.
     * @param alpha Faktor penghalusan baru.
     */
    void reset(float initialValue, float alpha);

private:
    float _alpha;          // Faktor penghalusan EMA
    float _filteredValue;  // Nilai yang sudah difilter terakhir
};

// --- Anda bisa menambahkan deklarasi kelas atau fungsi utilitas lainnya di sini nanti ---

#endif // UTILS_H