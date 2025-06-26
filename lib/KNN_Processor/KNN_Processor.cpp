#include "KNN_Processor.h"
#include <math.h>    // Untuk sqrt
#include <algorithm> // Diperlukan untuk std::sort

// Struktur helper untuk sorting, sama seperti NeighborInfo Anda
struct Neighbor
{
    float distance;
    int label;
};

// --- Implementasi Metode Kelas ---

KNN_Processor::KNN_Processor(int k) : _k(k), _dataset(nullptr), _dataset_size(0) {}

void KNN_Processor::loadDataset(const KNNDataPoint *dataset, int size)
{
    _dataset = dataset;
    _dataset_size = size;
}

// Ini adalah fungsi publik utama yang akan kita panggil
int KNN_Processor::classify(float weight)
{
    if (!_dataset || _dataset_size == 0)
    {
        return -1; // Error: dataset belum dimuat
    }

    float sample_feature[1] = {weight};
    int nearest_labels[_k];

    getKNearestNeighbors(sample_feature, nearest_labels);
    return predictClass(nearest_labels);
}

// --- Implementasi Metode Privat (Logika dari Kode Anda) ---

float KNN_Processor::euclideanDistance(const float features1[], const float features2[])
{
    // Untuk 1D, ini sama dengan `fabs(features1[0] - features2[0])`
    // Tapi kita pertahankan kode Anda untuk konsistensi
    float diff = features1[0] - features2[0];
    return sqrt(diff * diff);
}

void KNN_Processor::getKNearestNeighbors(const float new_sample_features[], int k_neighbors_labels[])
{
    Neighbor all_distances[_dataset_size];
    for (int i = 0; i < _dataset_size; i++)
    {
        all_distances[i].distance = euclideanDistance(new_sample_features, _dataset[i].features);
        all_distances[i].label = _dataset[i].label;
    }

    // PERBAIKAN: Menggunakan std::sort yang jauh lebih cepat daripada Bubble Sort.
    // Ini sangat penting untuk performa di sistem embedded.
    std::sort(all_distances, all_distances + _dataset_size, [](const Neighbor &a, const Neighbor &b)
              { return a.distance < b.distance; });

    for (int i = 0; i < _k; i++)
    {
        k_neighbors_labels[i] = all_distances[i].label;
    }
}

int KNN_Processor::predictClass(const int k_neighbors_labels[])
{
    const int MAX_LABEL = 15; // Sesuai dengan data Anda
    int vote_counts[MAX_LABEL + 1] = {0};

    for (int i = 0; i < _k; i++)
    {
        if (k_neighbors_labels[i] >= 0 && k_neighbors_labels[i] <= MAX_LABEL)
        {
            vote_counts[k_neighbors_labels[i]]++;
        }
    }

    int max_votes = -1;
    int predicted_label = 0;

    for (int i = 0; i <= MAX_LABEL; i++)
    {
        if (vote_counts[i] > max_votes)
        {
            max_votes = vote_counts[i];
            predicted_label = i;
        }
    }
    return predicted_label;
}
