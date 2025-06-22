#include "KNN_Processor.h" // Include header kita sendiri
#include <Arduino.h>       // Untuk Serial.println
#include <algorithm>       // Untuk std::sort
#include <map>             // Untuk std::map (untuk voting mayoritas)
#include <cmath>           // Untuk std::abs (untuk jarak)

// --- Definisi Dataset KNN Anda (Pindahkan dari kode lama Anda ke sini) ---
// Ini akan menjadi dataset default yang dimuat saat inisialisasi KNN_Processor.
// Nanti bisa dimodifikasi untuk dimuat dari EEPROM.
const KNN_DataPoint DEFAULT_KNN_DATASET[] = {
    // Jumlah Obat = 0 (Berat 0 - 1.3g)
    {0.2f, 0},
    {0.5f, 0},
    {0.8f, 0},
    {1.1f, 0},
    {1.3f, 0},
    // Jumlah Obat = 1 (Berat ~1.5g - 1.7g)
    {1.6f, 1},
    {1.55f, 1},
    {1.65f, 1},
    {1.7f, 1},
    {1.5f, 1},
    // Jumlah Obat = 2 (Berat ~3.0g - 3.4g)
    {3.15f, 2},
    {3.2f, 2},
    {3.3f, 2},
    {3.1f, 2},
    {3.25f, 2},
    // Jumlah Obat = 3 (Berat ~4.5g - 5.1g)
    {4.8f, 3},
    {4.9f, 3},
    {4.85f, 3},
    {4.65f, 3},
    {4.95f, 3},
    // Jumlah Obat = 4 (Berat ~6.0g - 6.8g)
    {6.45f, 4},
    {6.35f, 4},
    {6.3f, 4},
    {6.5f, 4},
    {6.4f, 4},
    // Jumlah Obat = 5 (Berat ~7.5g - 8.5g)
    {8.0f, 5},
    {7.75f, 5},
    {8.5f, 5},
    {7.5f, 5},
    {8.25f, 5},
    // Jumlah Obat = 6 (Berat ~9.0g - 10.2g)
    {9.1f, 6},
    {9.4f, 6},
    {9.65f, 6},
    {9.9f, 6},
    {10.15f, 6},
    // Jumlah Obat = 7 (Berat ~10.5g - 11.9g)
    {10.6f, 7},
    {10.9f, 7},
    {11.2f, 7},
    {11.5f, 7},
    {11.8f, 7},
    // Jumlah Obat = 8 (Berat ~12.0g - 13.6g)
    {12.1f, 8},
    {12.5f, 8},
    {12.8f, 8},
    {13.2f, 8},
    {13.5f, 8},
    // Jumlah Obat = 9 (Berat ~13.5g - 15.3g)
    {13.6f, 9},
    {14.0f, 9},
    {14.4f, 9},
    {14.8f, 9},
    {15.2f, 9},
    // Jumlah Obat = 10 (Berat ~15.0g - 17.0g)
    {15.2f, 10},
    {15.6f, 10},
    {16.0f, 10},
    {16.4f, 10},
    {16.8f, 10},
    // Jumlah Obat = 11 (Berat ~16.5g - 18.7g)
    {16.7f, 11},
    {17.2f, 11},
    {17.7f, 11},
    {18.2f, 11},
    {18.6f, 11},
    // Jumlah Obat = 12 (Berat ~18.0g - 20.4g)
    {18.2f, 12},
    {18.7f, 12},
    {19.2f, 12},
    {19.7f, 12},
    {20.2f, 12},
    // Jumlah Obat = 13 (Berat ~19.5g - 22.1g)
    {19.7f, 13},
    {20.3f, 13},
    {20.9f, 13},
    {21.5f, 13},
    {22.0f, 13},
    // Jumlah Obat = 14 (Berat ~21.0g - 23.8g)
    {21.2f, 14},
    {21.8f, 14},
    {22.4f, 14},
    {23.0f, 14},
    {23.6f, 14},
    // Jumlah Obat = 15 (Berat ~22.5g - 25.5g)
    {22.7f, 15},
    {23.5f, 15},
    {24.0f, 15},
    {24.8f, 15},
    {25.3f, 15}};
const size_t DEFAULT_DATASET_SIZE = sizeof(DEFAULT_KNN_DATASET) / sizeof(DEFAULT_KNN_DATASET[0]);

// --- Implementasi Kelas KNN_Processor ---

// Helper struct untuk sorting
struct NeighborInfo
{
    float distance;
    int label;
};

// Comparator untuk std::sort
bool compareNeighbors(const NeighborInfo &a, const NeighborInfo &b)
{
    return a.distance < b.distance;
}

KNN_Processor::KNN_Processor(int k) : _k(k)
{
    Serial.println("KNN_Processor: Modul KNN diinisialisasi.");
    // Muat dataset default saat inisialisasi
    addDataPoints(DEFAULT_KNN_DATASET, DEFAULT_DATASET_SIZE);
    Serial.printf("KNN_Processor: Dataset default dimuat, ukuran: %d\n", _dataset.size());
}

void KNN_Processor::addDataPoint(float weight, int pillCount)
{
    _dataset.push_back({weight, pillCount});
    Serial.printf("KNN_Processor: Menambah data point - Berat: %.2f g, Jumlah: %d\n", weight, pillCount);
}

void KNN_Processor::addDataPoints(const KNN_DataPoint data[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        _dataset.push_back(data[i]);
    }
    // Tidak mencetak per item, cukup setelah semua ditambahkan
}

int KNN_Processor::classify(float measuredWeight)
{
    if (_dataset.empty())
    {
        Serial.println("KNN_Processor: Dataset kosong, tidak dapat mengklasifikasi.");
        return -1; // Menunjukkan error atau tidak dapat mengklasifikasi
    }

    // Sesuaikan K jika lebih besar dari dataset
    int actualK = _k;
    if (_k > _dataset.size())
    {
        actualK = _dataset.size();
        Serial.printf("KNN_Processor: K (%d) lebih besar dari dataset size (%d). Menggunakan K = dataset size.\n", _k, _dataset.size());
    }

    // 1. Hitung jarak Euclidean untuk setiap titik data dalam dataset
    std::vector<NeighborInfo> distances; // Pair: {jarak, pillCount}
    for (const auto &point : _dataset)
    {
        float dist = calculateEuclideanDistance(measuredWeight, point.weight);
        distances.push_back({dist, point.pillCount});
    }

    // 2. Urutkan berdasarkan jarak (dari yang terdekat)
    std::sort(distances.begin(), distances.end(), compareNeighbors);

    // 3. Ambil K tetangga terdekat dan lakukan voting mayoritas
    std::map<int, int> votes; // Map: {pillCount, countOfVotes}
    for (int i = 0; i < actualK; ++i)
    {
        votes[distances[i].label]++; // Tambah vote untuk pillCount ini
    }

    // 4. Temukan pillCount dengan vote terbanyak
    int bestPillCount = -1;
    int maxVotes = -1;
    bool tie = false; // Flag untuk mendeteksi seri (tie-breaker logic bisa ditambahkan)

    for (const auto &pair : votes)
    {
        if (pair.second > maxVotes)
        {
            maxVotes = pair.second;
            bestPillCount = pair.first;
            tie = false; // Reset tie flag
        }
        else if (pair.second == maxVotes)
        {
            // Jika ada nilai yang sama, ini adalah seri (tie)
            // Anda bisa tambahkan logika tie-breaker di sini,
            // contoh: pilih label dengan nilai terkecil/terbesar, atau rata-rata label
            // Untuk saat ini, kita biarkan yang pertama ditemukan yang menang.
            tie = true;
        }
    }

    // Opsional: Jika ada seri dan K genap, kadang KNN bisa tidak stabil.
    // Jika K genap dan terjadi seri, bisa tambahkan logika seperti:
    // jika (tie && actualK % 2 == 0) {
    //   // Tambahkan logika untuk mencari tetangga ke K+1, atau cari rata-rata label, dll.
    // }

    Serial.printf("KNN_Processor: Berat %.2f g diklasifikasikan sebagai %d obat (K=%d).\n", measuredWeight, bestPillCount, actualK);
    return bestPillCount;
}

void KNN_Processor::clearDataset()
{
    _dataset.clear();
    Serial.println("KNN_Processor: Dataset KNN dikosongkan.");
}

size_t KNN_Processor::getDatasetSize() const
{
    return _dataset.size();
}

void KNN_Processor::setKValue(int k)
{
    if (k > 0)
    {
        _k = k;
        Serial.printf("KNN_Processor: Nilai K diatur ke %d.\n", _k);
    }
    else
    {
        Serial.println("KNN_Processor: Nilai K harus lebih besar dari 0.");
    }
}

// Metode helper untuk menghitung jarak Euclidean (dalam 1 dimensi)
float KNN_Processor::calculateEuclideanDistance(float weight1, float weight2) const
{
    return std::abs(weight1 - weight2); // Untuk 1D, Euclidean distance adalah selisih absolut
}