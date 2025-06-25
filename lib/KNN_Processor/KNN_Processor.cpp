#include "KNN_Processor.h"
#include <Arduino.h> // Memasukkan pustaka Arduino
#include <algorithm> // Memasukkan pustaka STL C++ 'algorithm', terutama untuk std::sort.
#include <map>       // Memasukkan pustaka STL C++ 'map', untuk implementasi voting mayoritas.
#include <cmath>     // Memasukkan pustaka C++ 'cmath', terutama untuk std::abs.

// --- Definisi Dataset KNN Default ---
const KNN_DataPoint DEFAULT_KNN_DATASET[] = {
    // Jumlah Obat = 0 (Berat 0 - 1.3g)
    {0.2f, 0},
    {0.5f, 0},
    {0.8f, 0},
    {1.1f, 0},
    {1.2f, 0},
    // Jumlah Obat = 1 (Berat ~1.5g - 1.7g)
    {1.5f, 1},
    {1.55f, 1},
    {1.6f, 1},
    {1.65f, 1},
    {1.7f, 1},
    // Jumlah Obat = 2 (Berat ~3.0g - 3.4g)
    {3.0f, 2},
    {3.1f, 2},
    {3.2f, 2},
    {3.3f, 2},
    {3.4f, 2},
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
// Menghitung ukuran dataset secara otomatis.
const size_t DEFAULT_DATASET_SIZE = sizeof(DEFAULT_KNN_DATASET) / sizeof(DEFAULT_KNN_DATASET[0]);

// --- Struktur Helper dan Comparator untuk Sorting ---
struct NeighborInfo
{
    float distance; // Jarak dari titik data yang diukur
    int label;      // Label (jumlah obat) dari titik data ini.
};

// Fungsi comparator ini digunakan oleh std::sort.
// Ia mendefinisikan bagaimana dua objek NeighborInfo harus dibandingkan: berdasarkan jarak (asc).
bool compareNeighbors(const NeighborInfo &a, const NeighborInfo &b)
{
    return a.distance < b.distance; // Mengurutkan dari jarak terkecil ke terbesar.
}

// --- Implementasi Kelas KNN_Processor ---

// Konstruktor kelas. Dipanggil saat objek KNN_Processor dibuat.
KNN_Processor::KNN_Processor(int k) : _k(k)
{ // Inisialisasi member _k dengan nilai 'k' yang diberikan.
    Serial.println("KNN_Processor: Modul KNN diinisialisasi.");
    // Memuat dataset default ke dalam '_dataset' (variabel member kelas).
    addDataPoints(DEFAULT_KNN_DATASET, DEFAULT_DATASET_SIZE);
    Serial.printf("KNN_Processor: Dataset default dimuat, ukuran: %d\n", _dataset.size());
}

// Menambahkan satu titik data ke _dataset.
void KNN_Processor::addDataPoint(float weight, int pillCount)
{
    _dataset.push_back({weight, pillCount}); // Menambahkan KNN_DataPoint baru ke akhir vector _dataset.
    Serial.printf("KNN_Processor: Menambah data point - Berat: %.2f g, Jumlah: %d\n", weight, pillCount);
}

// Menambahkan banyak titik data dari array ke _dataset.
void KNN_Processor::addDataPoints(const KNN_DataPoint data[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {                                // Loop melalui setiap elemen di array 'data'.
        _dataset.push_back(data[i]); // Menambahkan setiap elemen ke _dataset.
    }
}

// Metode mengklasifikasikan berat yang diukur.
int KNN_Processor::classify(float measuredWeight)
{
    // 1. Cek Dataset Kosong
    if (_dataset.empty())
    {
        Serial.println("KNN_Processor: Dataset kosong, tidak dapat mengklasifikasi.");
        return -1;
    }

    // 2. Sesuaikan Nilai K
    int actualK = _k; // Menggunakan variabel lokal untuk K yang efektif.
    if (_k > _dataset.size())
    {                              // Jika K yang diminta lebih besar dari jumlah data dalam dataset.
        actualK = _dataset.size(); // Set K efektif sama dengan ukuran dataset.
        Serial.printf("KNN_Processor: K (%d) lebih besar dari dataset size (%d). Menggunakan K = dataset size.\n", _k, _dataset.size());
    }

    // 3. Hitung Jarak ke Semua Titik Data
    std::vector<NeighborInfo> distances; // Vector untuk menyimpan jarak dan label dari semua titik dataset.
    distances.reserve(_dataset.size());  // Optimasi: alokasikan memori di awal
    for (const auto &point : _dataset)
    {                                                                          // Loop melalui setiap 'point' dalam '_dataset'.
        float dist = calculateEuclideanDistance(measuredWeight, point.weight); // Hitung jarak.
        distances.push_back({dist, point.pillCount});                          // Tambahkan {jarak, label} ke vector 'distances'.
    }

    // 4. Urutkan Tetangga Berdasarkan Jarak
    // Mengurutkan vector 'distances' menggunakan std::sort dan comparator kita.
    std::sort(distances.begin(), distances.end(), compareNeighbors);

    // 5. Ambil K Tetangga Terdekat & Lakukan Voting Terbobot
    std::map<int, float> weightedVotes; // Map untuk menghitung suara terbobot: {label_obat : total_bobot_suara}.

    for (int i = 0; i < actualK; ++i)
    { // Loop melalui 'actualK' tetangga terdekat.
        float dist = distances[i].distance;
        int label = distances[i].label;

        float weight = 0.0f;
        if (dist == 0.0f)
        {                    // Jika jaraknya 0, berikan bobot yang sangat besar (kecocokan sempurna).
            weight = 1.0e9f; // Bobot sangat besar untuk kecocokan sempurna. 'f' untuk float literal.
        }
        else
        {
            weight = 1.0f / (dist * dist); // Bobot = 1 / jarak^2.
        }
        weightedVotes[label] += weight; // Tambahkan bobot ke total suara untuk label ini.
    }

    // 6. Temukan Label (Jumlah Obat) dengan Bobot Suara Terbanyak
    int bestPillCount = -1;         // Inisialisasi hasil prediksi dengan -1 (nilai error/tidak ditemukan).
    float maxWeightedVotes = -1.0f; // Inisialisasi jumlah bobot suara terbanyak dengan -1.0f.

    // Iterasi melalui map 'weightedVotes' untuk menemukan label dengan bobot suara terbanyak.
    for (const auto &pair : weightedVotes)
    { // 'pair' adalah pasangan {label, total_bobot_suara}.
        if (pair.second > maxWeightedVotes)
        {                                   // Jika total bobot suara saat ini lebih besar dari 'maxWeightedVotes' sebelumnya.
            maxWeightedVotes = pair.second; // Update 'maxWeightedVotes'.
            bestPillCount = pair.first;     // Update 'bestPillCount' ke label ini.
            // Tidak perlu bendera 'tie' di sini; yang pertama ditemukan dengan bobot tertinggi akan menjadi pemenang.
        }
    }

    // Output ke Serial Monitor untuk debugging/informasi.
    Serial.printf("KNN_Processor: Berat %.2f g diklasifikasikan sebagai %d obat (K=%d) dengan Weighted Voting. Max Weight: %.2f\n", measuredWeight, bestPillCount, actualK, maxWeightedVotes);
    return bestPillCount; // Mengembalikan label (jumlah obat) yang diprediksi.
}

// Mengosongkan dataset.
void KNN_Processor::clearDataset()
{
    _dataset.clear(); // Menghapus semua elemen dari vector '_dataset'.
    Serial.println("KNN_Processor: Dataset KNN dikosongkan.");
}

// Mendapatkan ukuran dataset.
size_t KNN_Processor::getDatasetSize() const
{
    return _dataset.size(); // Mengembalikan jumlah elemen dalam '_dataset'.
}

// Mengatur nilai K.
void KNN_Processor::setKValue(int k)
{
    if (k > 0)
    {           // Pastikan K adalah nilai positif.
        _k = k; // Mengatur nilai K ke variabel member '_k'.
        Serial.printf("KNN_Processor: Nilai K diatur ke %d.\n", _k);
    }
    else
    {
        Serial.println("KNN_Processor: Nilai K harus lebih besar dari 0.");
    }
}

// Implementasi metode helper untuk menghitung jarak Euclidean (1D).
float KNN_Processor::calculateEuclideanDistance(float weight1, float weight2) const
{
    return std::abs(weight1 - weight2); // Menggunakan std::abs dari cmath untuk nilai absolut.
}