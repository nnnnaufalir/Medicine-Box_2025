#ifndef KNN_PROCESSOR_H
#define KNN_PROCESSOR_H

// Mengadopsi struktur data dari kode Anda
struct KNNDataPoint
{
    float features[1]; // Menggunakan array seperti di kode Anda
    int label;
};

class KNN_Processor
{
public:
    KNN_Processor(int k);

    // Metode untuk memuat dataset (C-style array) Anda
    void loadDataset(const KNNDataPoint *dataset, int size);

    // Fungsi publik utama untuk melakukan klasifikasi
    int classify(float weight);

private:
    // --- Variabel internal kelas ---
    int _k;
    const KNNDataPoint *_dataset; // Pointer ke dataset Anda
    int _dataset_size;

    // --- Fungsi helper dari kode Anda, sekarang menjadi metode privat ---
    float euclideanDistance(const float features1[], const float features2[]);
    void getKNearestNeighbors(const float new_sample_features[], int k_neighbors_labels[]);
    int predictClass(const int k_neighbors_labels[]);
};

#endif // KNN_PROCESSOR_H
