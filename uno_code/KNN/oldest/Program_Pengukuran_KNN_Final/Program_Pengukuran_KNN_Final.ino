#include "HX711.h"

HX711 scale;

// --- Kelas Filter EMA ---
class EMAFilter {
private:
  float alpha, last_ema;
  bool has_run;
public:
  EMAFilter(float a)
    : alpha(a), last_ema(0.0f), has_run(false) {}
  float filter(float val) {
    if (!has_run) {
      last_ema = val;
      has_run = true;
    }
    last_ema = (alpha * val) + ((1.0 - alpha) * last_ema);
    return last_ema;
  }
};

// --- Parameter Sistem ---
EMAFilter raw_filter(0.16);  // Alpha yang telah Anda tuning

// --- Dataset Kalibrasi KNN Anda ---
// Saya berasumsi data ini untuk 0 hingga 20 obat (total 21 titik)
const int JUMLAH_DATASET_KNN = 26;
const long knn_dataset_original[JUMLAH_DATASET_KNN] = {
  294165,  297965,  302155,  307955,
  314605,  320403,  328027,  334577,
  338975,  345882,  350608,  355397,
  359228,  364620,  374860,  377343,
  384496,  388387,  394963,  401632,
  407596,  411109,  416649,  421598,
  427395,  434011
};

// Array untuk menyimpan dataset yang telah disesuaikan dengan drift
long knn_dataset_adjusted[JUMLAH_DATASET_KNN];

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Sistem Pengukuran KNN (Final)...");
  scale.begin(13, 14);

  // --- Koreksi Drift Dinamis saat Startup ---
  Serial.println("Menyesuaikan dataset berdasarkan drift saat ini...");
  long original_zero_raw = knn_dataset_original[0];
  long current_zero_raw = scale.read_average(30);
  long drift_offset = current_zero_raw - original_zero_raw;

  Serial.print("Drift terdeteksi: ");
  Serial.println(drift_offset);

  // Buat dataset baru yang telah disesuaikan di dalam memori
  for (int i = 0; i < JUMLAH_DATASET_KNN; i++) {
    knn_dataset_adjusted[i] = knn_dataset_original[i] + drift_offset;
  }
  Serial.println("Dataset KNN telah disesuaikan.");

  // Inisialisasi filter
  raw_filter.filter(current_zero_raw);
  Serial.println("Sistem siap.");
}

void loop() {
  // 1. Baca dan filter nilai mentah
  float filtered_raw = raw_filter.filter(scale.read());

  // 2. Implementasi Algoritma K-Nearest Neighbors (KNN, dengan K=1)
  long min_distance = -1;
  int best_match_index = 0;

  for (int i = 0; i < JUMLAH_DATASET_KNN; i++) {
    long distance = abs(long(filtered_raw) - knn_dataset_adjusted[i]);
    if (min_distance == -1 || distance < min_distance) {
      min_distance = distance;
      best_match_index = i;
    }
  }

  // 3. Hasilnya adalah indeks dari tetangga terdekat
  int pill_count = best_match_index;

  // Tampilkan hasil akhir
  Serial.print("Jumlah Obat: ");
  Serial.print(pill_count);
  Serial.print("  |  Mentah Terfilter: ");
  Serial.println(long(filtered_raw));

  delay(200);
}
