#ifndef KNN_PROCESSOR_H
#define KNN_PROCESSOR_H

// --- Includes ---
#include <Arduino.h>
#include <vector>      // Untuk menyimpan dataset (std::vector)
#include "AppConfig.h" // Untuk KNN_K_VALUE (nilai K default)

// --- Struktur Data untuk Dataset KNN ---
// Mewakili satu titik data dalam dataset
struct KNN_DataPoint
{
    float weight;  // Fitur input: Berat terkalibrasi (misal: dalam gram)
    int pillCount; // Label/kelas output: Jumlah obat yang sesuai dengan berat tersebut
};

// --- Kelas KNN_Processor ---
class KNN_Processor
{
public:
    /**
     * @brief Konstruktor untuk KNN_Processor.
     * Fungsi ini otomatis dipanggil saat objek KNN_Processor dibuat
     * Menginisialisasi nilai K dan memuat dataset default.
     * @param k Nilai 'K' yang akan digunakan dalam algoritma K-Nearest Neighbors. Defaultnya diambil dari APP_CONFIG.
     */
    KNN_Processor(int k = KNN_K_VALUE);

    /**
     * @brief Menambahkan satu titik data (sampel) ke dataset KNN.
     * Dataset ini digunakan untuk melatih/mempelajari hubungan antara berat dan jumlah obat.
     * @param weight Berat terkalibrasi dari titik data yang akan ditambahkan.
     * @param pillCount Jumlah obat (label) yang sesuai dengan berat tersebut.
     */
    void addDataPoint(float weight, int pillCount);

    /**
     * @brief Menambahkan banyak titik data sekaligus dari sebuah array ke dataset KNN.
     * Berguna untuk memuat dataset awal secara efisien.
     * @param data Pointer ke array berisi struktur KNN_DataPoint.
     * @param size Ukuran (jumlah elemen) dari array 'data'.
     */
    void addDataPoints(const KNN_DataPoint data[], size_t size);

    /**
     * @brief Mengklasifikasikan jumlah obat berdasarkan berat yang diberikan.
     * Menggunakan algoritma KNN dengan Weighted Voting untuk mencari tetangga terdekat.
     * @param measuredWeight Berat yang diukur (dari load cell, sudah difilter EMA).
     * @return Estimasi jumlah obat. Mengembalikan -1 jika dataset kosong atau tidak dapat mengklasifikasi.
     */
    int classify(float measuredWeight);

    /**
     * @brief Mengosongkan (menghapus semua) titik data dari dataset KNN.
     * Berguna jika ingin melatih ulang atau memuat dataset baru.
     */
    void clearDataset();

    /**
     * @brief Mendapatkan jumlah titik data yang saat ini ada dalam dataset.
     * @return Jumlah elemen dalam '_dataset'.
     */
    size_t getDatasetSize() const;

    /**
     * @brief Menetapkan nilai K baru untuk algoritma KNN.
     * @param k Nilai K yang baru. Harus lebih besar dari 0.
     */
    void setKValue(int k);

private:                                 // Bagian 'private' berarti variabel dan metode ini hanya bisa diakses dari dalam kelas ini.
    std::vector<KNN_DataPoint> _dataset; // Variabel private untuk menyimpan dataset pelatihan KNN kita. Menggunakan 'std::vector' untuk fleksibilitas.
    int _k;                              // Variabel private untuk menyimpan nilai K yang saat ini digunakan.

    /**
     * @brief Metode helper untuk menghitung jarak Euclidean antara dua berat.
     * Karena kita hanya punya satu fitur (berat), jarak Euclidean adalah selisih absolut.
     * @param weight1 Berat pertama.
     * @param weight2 Berat kedua.
     * @return Nilai jarak Euclidean.
     */
    float calculateEuclideanDistance(float weight1, float weight2) const;
};

#endif // KNN_PROCESSOR_H