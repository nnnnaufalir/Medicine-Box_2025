#ifndef KNN_PROCESSOR_H
#define KNN_PROCESSOR_H

// --- Includes ---
#include <Arduino.h>   // Untuk tipe data dasar seperti float, int
#include <vector>      // Untuk menyimpan dataset
#include "AppConfig.h" // Untuk KNN_K_VALUE

// --- Struktur Data untuk Dataset KNN ---
// Mewakili satu titik data dalam dataset (berat dan jumlah obat terkait)
// Menggunakan struct yang sama dengan Anda
struct KNN_DataPoint
{
    float weight;  // Berat terkalibrasi (misal: dalam gram)
    int pillCount; // Jumlah obat yang sesuai dengan berat tersebut
};

// --- Kelas KNN_Processor ---
class KNN_Processor
{
public:
    /**
     * @brief Konstruktor untuk KNN_Processor.
     * @param k Nilai 'K' untuk algoritma K-Nearest Neighbors.
     */
    KNN_Processor(int k = KNN_K_VALUE);

    /**
     * @brief Menambahkan satu titik data ke dataset KNN.
     * @param weight Berat terkalibrasi dari titik data.
     * @param pillCount Jumlah obat yang sesuai.
     */
    void addDataPoint(float weight, int pillCount);

    /**
     * @brief Menambahkan banyak titik data sekaligus dari sebuah array.
     * @param data Array dari KNN_DataPoint.
     * @param size Ukuran array.
     */
    void addDataPoints(const KNN_DataPoint data[], size_t size);

    /**
     * @brief Mengklasifikasikan jumlah obat berdasarkan berat yang diberikan.
     * Menggunakan algoritma KNN untuk mencari tetangga terdekat.
     * @param measuredWeight Berat yang diukur (sudah difilter).
     * @return Estimasi jumlah obat. Mengembalikan -1 jika tidak dapat mengklasifikasi.
     */
    int classify(float measuredWeight);

    /**
     * @brief Mengosongkan dataset KNN.
     */
    void clearDataset();

    /**
     * @brief Mendapatkan jumlah titik data dalam dataset.
     * @return Jumlah titik data.
     */
    size_t getDatasetSize() const;

    /**
     * @brief Menetapkan nilai K.
     * @param k Nilai K baru.
     */
    void setKValue(int k);

private:
    std::vector<KNN_DataPoint> _dataset; // Dataset KNN (berat vs jumlah obat)
    int _k;                              // Nilai K untuk KNN

    // Metode helper untuk menghitung jarak Euclidean
    // Untuk 1D, Euclidean distance adalah selisih absolut
    float calculateEuclideanDistance(float weight1, float weight2) const;
};

#endif // KNN_PROCESSOR_H