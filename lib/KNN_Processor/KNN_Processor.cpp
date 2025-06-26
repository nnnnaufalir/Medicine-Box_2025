#include "KNN_Processor.h"
#include <math.h>
#include <algorithm> // Untuk std::sort yang lebih efisien

// Struktur helper internal untuk sorting
struct Neighbor
{
    float distance;
    int label;
};

KNN_Processor::KNN_Processor(int k) : _k(k), _dataset(nullptr), _dataset_size(0) {}

void KNN_Processor::loadDataset(const KNNDataPoint *dataset, int size)
{
    _dataset = dataset;
    _dataset_size = size;
}

int KNN_Processor::classify(float weight)
{
    if (!_dataset || _dataset_size == 0)
    {
        return -1; // Kembalikan -1 jika dataset belum dimuat
    }

    float sample_feature[1] = {weight};
    int nearest_labels[_k];

    getKNearestNeighbors(sample_feature, nearest_labels);
    return predictClass(nearest_labels);
}

float KNN_Processor::euclideanDistance(const float features1[], const float features2[])
{
    float diff = features1[0] - features2[0];
    return sqrt(diff * diff);
}

void KNN_Processor::getKNearestNeighbors(const float new_sample_features[], int k_neighbors_labels[])
{
    // Buat vektor dinamis untuk jarak
    std::vector<Neighbor> all_distances;
    all_distances.reserve(_dataset_size);

    for (int i = 0; i < _dataset_size; i++)
    {
        float dist = euclideanDistance(new_sample_features, _dataset[i].features);
        all_distances.push_back({dist, _dataset[i].label});
    }

    // Menggunakan std::sort yang jauh lebih cepat dari Bubble Sort
    std::sort(all_distances.begin(), all_distances.end(), [](const Neighbor &a, const Neighbor &b)
              { return a.distance < b.distance; });

    // Ambil K tetangga terdekat
    for (int i = 0; i < _k; i++)
    {
        k_neighbors_labels[i] = all_distances[i].label;
    }
}

int KNN_Processor::predictClass(const int k_neighbors_labels[])
{
    const int MAX_LABEL = 15; // Sesuai data Anda
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
