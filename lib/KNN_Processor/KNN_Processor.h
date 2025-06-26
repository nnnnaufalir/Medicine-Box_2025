#ifndef KNN_PROCESSOR_H
#define KNN_PROCESSOR_H

#include <vector>

// Menggunakan struktur data dari logika Anda
struct KNNDataPoint
{
    float features[1];
    int label;
};

class KNN_Processor
{
public:
    KNN_Processor(int k);

    // Memuat dataset (C-style array) dari program utama
    void loadDataset(const KNNDataPoint *dataset, int size);

    // Fungsi utama untuk klasifikasi berat
    int classify(float weight);

private:
    int _k;
    const KNNDataPoint *_dataset;
    int _dataset_size;

    // Metode helper, sekarang menjadi bagian privat dari kelas
    float euclideanDistance(const float features1[], const float features2[]);
    void getKNearestNeighbors(const float new_sample_features[], int k_neighbors_labels[]);
    int predictClass(const int k_neighbors_labels[]);
};

#endif // KNN_PROCESSOR_H
