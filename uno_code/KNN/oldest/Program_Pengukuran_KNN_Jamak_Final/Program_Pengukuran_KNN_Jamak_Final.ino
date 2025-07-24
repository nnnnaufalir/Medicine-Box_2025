#include "HX711.h"

HX711 scale;

// --- [KONFIGURASI] Tentukan nilai K di sini ---
// K harus berupa angka ganjil (misal: 3, 5, 7) untuk menghindari hasil seri.
#define K_VALUE 7

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

// --- Struktur Data KNN ---
const int SAMPEL_PER_KELAS = 11;
struct KNNDataPoint {
  int jumlah_obat;
  long nilai_mentah[SAMPEL_PER_KELAS];
};

// --- Parameter Sistem ---
EMAFilter raw_filter(0.16);

// --- Dataset Kalibrasi Jamak Anda ---
const int JUMLAH_KELAS = 11;
const KNNDataPoint knn_dataset_original[JUMLAH_KELAS] = {
  { 0, { 291972, 292115, 292259, 292402, 292546, 292690, 292833, 292977, 293120, 293264, 293408 } },
  { 1, { 296034, 296210, 296386, 296562, 296738, 296915, 297091, 297267, 297443, 297619, 297796 } },
  { 2, { 303138, 303248, 303359, 303470, 303581, 303692, 303802, 303913, 304024, 304135, 304246 } },
  { 3, { 306738, 306867, 306997, 307126, 307256, 307386, 307515, 307645, 307774, 307904, 308034 } },
  { 4, { 315180, 315317, 315455, 315593, 315731, 315869, 316006, 316144, 316282, 316420, 316558 } },
  { 5, { 321728, 321819, 321910, 322001, 322092, 322184, 322275, 322366, 322457, 322548, 322640 } },
  { 6, { 326017, 326114, 326212, 326310, 326408, 326506, 326603, 326701, 326799, 326897, 326995 } },
  { 7, { 332156, 332267, 332379, 332490, 332602, 332714, 332825, 332937, 333048, 333160, 333272 } },
  { 8, { 338589, 338760, 338931, 339102, 339273, 339445, 339616, 339787, 339958, 340129, 340301 } },
  { 9, { 346162, 346333, 346504, 346675, 346846, 347018, 347189, 347360, 347531, 347702, 347874 } },
  { 10, { 349361, 349488, 349615, 349742, 349869, 349997, 350124, 350251, 350378, 350505, 350633 } }
};

// Array untuk menyimpan dataset yang telah disesuaikan dengan drift
KNNDataPoint knn_dataset_adjusted[JUMLAH_KELAS];

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Sistem Pengukuran KNN Jamak (Final)...");
  scale.begin(13, 14);

  // Koreksi Drift Dinamis saat Startup
  long original_zero_raw = knn_dataset_original[0].nilai_mentah[2];  // Gunakan titik tengah sebagai referensi
  long current_zero_raw = scale.read_average(30);
  long drift_offset = (current_zero_raw - original_zero_raw) / 2;

  // Buat dataset baru yang telah disesuaikan di dalam memori
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    knn_dataset_adjusted[i].jumlah_obat = knn_dataset_original[i].jumlah_obat;
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      knn_dataset_adjusted[i].nilai_mentah[j] = knn_dataset_original[i].nilai_mentah[j] + drift_offset;
    }
  }

  raw_filter.filter(current_zero_raw);
  Serial.println("Sistem siap.");
}

void loop() {
  float filtered_raw = raw_filter.filter(scale.read());

  // --- Implementasi Algoritma K-Nearest Neighbors (KNN) ---

  // 1. Temukan K tetangga terdekat dari total 130 titik data
  long nearest_distances[K_VALUE];
  int nearest_classes[K_VALUE];

  for (int i = 0; i < K_VALUE; i++) {
    nearest_distances[i] = LONG_MAX;
  }

  for (int i = 0; i < JUMLAH_KELAS; i++) {
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      long distance = abs(long(filtered_raw) - knn_dataset_adjusted[i].nilai_mentah[j]);

      for (int k = 0; k < K_VALUE; k++) {
        if (distance < nearest_distances[k]) {
          for (int m = K_VALUE - 1; m > k; m--) {
            nearest_distances[m] = nearest_distances[m - 1];
            nearest_classes[m] = nearest_classes[m - 1];
          }
          nearest_distances[k] = distance;
          nearest_classes[k] = knn_dataset_adjusted[i].jumlah_obat;
          break;
        }
      }
    }
  }

  // 2. Lakukan Pemungutan Suara Mayoritas (Majority Vote)
  int votes[JUMLAH_KELAS] = { 0 };
  for (int i = 0; i < K_VALUE; i++) {
    votes[nearest_classes[i]]++;
  }

  int max_votes = 0;
  int pill_count = 0;
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    if (votes[i] > max_votes) {
      max_votes = votes[i];
      pill_count = i;
    }
  }

  // Tampilkan hasil akhir
  Serial.print("Jumlah Obat: ");
  Serial.print(pill_count);
  Serial.print("  |  Mentah Terfilter: ");
  Serial.println(long(filtered_raw));

  delay(1);
}
