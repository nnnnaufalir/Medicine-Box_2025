#include "HX711.h"

HX711 scale;

// --- [KONFIGURASI] ---
const int K_VALUE = 17;

// --- Kelas & Struktur Data ---
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

const int SAMPEL_PER_KELAS = 11;
const int JUMLAH_KELAS = 26;
struct KNNDataPoint {
  int jumlah_obat;
  long nilai_mentah[SAMPEL_PER_KELAS];
};

// --- Parameter & Variabel Sistem ---
EMAFilter raw_filter(0.16);
const KNNDataPoint knn_dataset_original[JUMLAH_KELAS] = {
  { 0, { 294061, 294139, 294218, 294298, 294377, 295457, 295460, 295474, 295483, 295492, 295500 } },
  { 1, { 299808, 299853, 299899, 299948, 299995, 300043, 300088, 300137, 300185, 300232, 300280 } },
  { 2, { 306652, 306704, 306757, 306809, 306862, 306915, 306968, 307020, 307073, 307125, 307179 } },
  { 3, { 310399, 310455, 310512, 310568, 310625, 310682, 310738, 310795, 310851, 310908, 310965 } },
  { 4, { 315934, 315983, 316031, 316080, 316129, 316177, 316226, 316274, 316282, 316310, 316378 } },
  { 5, { 322100, 322197, 322267, 322337, 322408, 322478, 322548, 322619, 322689, 322759, 322830 } },
  { 6, { 327889, 327966, 328043, 328119, 328196, 328274, 328350, 328427, 328504, 328581, 328658 } },
  { 7, { 337284, 337339, 337394, 337449, 337504, 337559, 337614, 337669, 337724, 337779, 337835 } },
  { 8, { 337898, 337955, 338012, 338070, 338127, 338185, 338241, 338299, 338356, 338414, 338471 } },
  { 9, { 346171, 346230, 346289, 346348, 346408, 346465, 346525, 346585, 346645, 346704, 346764 } },
  { 10, { 354327, 354386, 354446, 354515, 354579, 354643, 354706, 354770, 354833, 354897, 354962 } },
  { 11, { 358070, 358122, 358175, 358227, 358280, 358333, 358385, 358438, 358490, 358543, 358596 } },
  { 12, { 365697, 365771, 365846, 365924, 365997, 366070, 366142, 366214, 366287, 366359, 366433 } },
  { 13, { 371240, 371280, 371321, 371363, 371405, 371446, 371488, 371529, 371571, 371612, 371654 } },
  { 14, { 376915, 376943, 376972, 377001, 377030, 377059, 377088, 377117, 377146, 377175, 377204 } },
  { 15, { 381553, 381617, 381682, 381747, 381812, 381877, 381941, 382006, 382070, 382135, 382200 } },
  { 16, { 387845, 387880, 387915, 387954, 387993, 388032, 388070, 388108, 388147, 388186, 388225 } },
  { 17, { 395278, 395304, 395330, 395357, 395385, 395412, 395439, 395466, 395493, 395520, 395547 } },
  { 18, { 399855, 399903, 399951, 399999, 400047, 400096, 400144, 400192, 400241, 400289, 400338 } },
  { 19, { 407690, 407740, 407791, 407842, 407892, 407941, 407991, 408040, 408089, 408138, 408188 } },
  { 20, { 414066, 414124, 414182, 414240, 414299, 414357, 414415, 414474, 414532, 414590, 414649 } },
  { 21, { 419250, 419319, 419388, 419458, 419527, 419597, 419666, 419736, 419805, 419875, 419944 } },
  { 22, { 423730, 423785, 423841, 423897, 423952, 424009, 424065, 424121, 424177, 424233, 424289 } },
  { 23, { 427506, 427544, 427582, 427620, 427658, 427697, 427735, 427773, 427811, 427849, 427887 } },
  { 24, { 431299, 431337, 431375, 431413, 431451, 431490, 431528, 431566, 431604, 431642, 431681 } },
  { 25, { 441118, 441159, 441202, 441240, 441277, 441314, 441351, 441388, 441425, 441462, 441500 } }
};
KNNDataPoint knn_dataset_adjusted[JUMLAH_KELAS];

// --- Variabel Logika ---
bool isSystemStable = true;
bool isFrozen = false;  // [BARU] Status untuk "Pembekuan Cerdas"
int last_stable_pill_count = 0;
unsigned long lastInteractionTime = 0;
float lastFilteredRawForStabilityCheck = 0.0;
float anchor_filtered_raw;
unsigned long last_drift_check_time = 0;

const unsigned long SETTLING_TIME_MS = 1000;
const float INTERACTION_THRESHOLD_RAW = 500;
const long HAND_INTERACTION_THRESHOLD_RAW = 110000;  // [BARU] Ambang batas untuk deteksi tangan
const unsigned long DRIFT_CHECK_INTERVAL_MS = 10000;
const long STABILITY_THRESHOLD_RAW = 3000;

// --- FUNGSI-FUNGSI LOGIKA ---

void adjustDatasetByOffset(long offset) {
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      knn_dataset_adjusted[i].nilai_mentah[j] += offset;
    }
  }
}

void adjustDatasetForDrift() {
  long original_zero_raw = knn_dataset_original[0].nilai_mentah[5];
  long current_zero_raw = scale.read_average(30);
  long drift_offset = current_zero_raw - original_zero_raw;

  Serial.print("Drift terdeteksi: ");
  Serial.println(drift_offset);

  for (int i = 0; i < JUMLAH_KELAS; i++) {
    knn_dataset_adjusted[i].jumlah_obat = knn_dataset_original[i].jumlah_obat;
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      knn_dataset_adjusted[i].nilai_mentah[j] = knn_dataset_original[i].nilai_mentah[j] + drift_offset;
    }
  }
  anchor_filtered_raw = raw_filter.filter(current_zero_raw);
}

void updateSystemStability(float current_filtered_raw) {
  float change = abs(scale.read() - lastFilteredRawForStabilityCheck);

  if (change > HAND_INTERACTION_THRESHOLD_RAW) {  // [BARU] Deteksi tangan masuk
    Serial.println("ada Tangan");
    isSystemStable = false;
    isFrozen = true;  // Aktifkan mode beku
    lastInteractionTime = millis();
  } else if (change > INTERACTION_THRESHOLD_RAW) {  // Interaksi normal
    isSystemStable = false;
    lastInteractionTime = millis();
  }

  if (!isSystemStable && (millis() - lastInteractionTime > SETTLING_TIME_MS)) {
    isSystemStable = true;
    isFrozen = false;  // Nonaktifkan mode beku setelah masa tenang
  }
  lastFilteredRawForStabilityCheck = current_filtered_raw;
}

void runStabilityGuardian(long current_stable_raw, int current_pill_count) {
  if (millis() - last_drift_check_time < DRIFT_CHECK_INTERVAL_MS) return;

  Serial.println("\n--- PENJAGA STABILITAS AKTIF ---");

  if (isSystemStable) {
    Serial.println("   --> Status: STABIL. Memeriksa drift...");

    long drift_diff = current_stable_raw - anchor_filtered_raw;

    Serial.print("   Titik Jangkar Lama : ");
    Serial.println(long(anchor_filtered_raw));
    Serial.print("   Titik Stabil Saat Ini: ");
    Serial.println(long(current_stable_raw));
    Serial.print("   Selisih Absolut    : ");
    Serial.println(abs(drift_diff));
    Serial.print("   Ambang Batas       : ");
    Serial.println(STABILITY_THRESHOLD_RAW);

    if (current_pill_count == 0) {  // [BARU] Logika Taring Aktif
      Serial.println(" Taring Aktif!");
      long old_zero_center = knn_dataset_adjusted[0].nilai_mentah[5];
      long refinement_offset = current_stable_raw - old_zero_center;
      if (abs(refinement_offset) > 0) {  // Hanya koreksi jika ada perubahan
        adjustDatasetByOffset(refinement_offset);
      }
    } else if (abs(drift_diff) < STABILITY_THRESHOLD_RAW) {  // Logika drift normal
      Serial.print("   --> KEPUTUSAN: Drift terdeteksi: ");
      Serial.println(drift_diff);
      adjustDatasetByOffset(drift_diff);
    } else {
      Serial.println("   --> KEPUTUSAN: Perubahan berat signifikan. Tidak ada koreksi.");
    }
    anchor_filtered_raw = current_stable_raw;
  } else {
    Serial.println("   --> Status: SIBUK. Melewatkan pengecekan drift.");
  }
  last_drift_check_time = millis();
  Serial.println("--------------------------------\n");
}

int classifyPillCount(float current_filtered_raw) {
  long nearest_distances[K_VALUE];
  int nearest_classes[K_VALUE];

  for (int i = 0; i < K_VALUE; i++) nearest_distances[i] = LONG_MAX;

  for (int i = 0; i < JUMLAH_KELAS; i++) {
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      long distance = abs(long(current_filtered_raw) - knn_dataset_adjusted[i].nilai_mentah[j]);
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

  float weighted_votes[JUMLAH_KELAS] = { 0.0f };
  for (int i = 0; i < K_VALUE; i++) {
    float weight = 1.0 / (nearest_distances[i] + 1e-6);
    weighted_votes[nearest_classes[i]] += weight;
  }

  float max_weight = -1.0f;
  int result = 0;
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    if (weighted_votes[i] > max_weight) {
      max_weight = weighted_votes[i];
      result = i;
    }
  }

  // [DIAGNOSTIK] Tampilkan detail pemungutan suara
  Serial.println("\n--- Analisis KNN ---");
  Serial.print("   Tetangga Terdekat (Obat): { ");
  for (int i = 0; i < K_VALUE; i++) {
    Serial.print(nearest_classes[i]);
    if (i < K_VALUE - 1) Serial.print(", ");
  }
  Serial.println(" }");
  Serial.print("   Total Bobot Suara Tertinggi: ");
  Serial.print(max_weight, 4);
  Serial.print(" untuk kelas ");
  Serial.print(result);
  Serial.println(" obat.");
  Serial.println("--------------------");

  return result;
}

void printDiagnostics(float current_filtered_raw, int pill_count) {
  Serial.print("Titik 0: ");
  Serial.print(knn_dataset_adjusted[0].nilai_mentah[10]);
  Serial.print(" | Nilai Terfilter: ");
  Serial.print(long(current_filtered_raw));

  // [BARU] Tampilan status yang lebih deskriptif
  Serial.print(" | Status: ");
  if (isFrozen) {
    Serial.print("BEKU (Tangan Terdeteksi)");
  } else {
    Serial.print(isSystemStable ? "STABIL" : "SIBUK");
  }

  Serial.print(" | Prediksi Obat: ");
  Serial.println(pill_count);
}

// --- FUNGSI UTAMA ---

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Sistem KNN Robust Final...");
  scale.begin(13, 14);
  adjustDatasetForDrift();
  lastFilteredRawForStabilityCheck = anchor_filtered_raw;
  last_drift_check_time = millis();
  Serial.println("Sistem siap.");
}

void loop() {

  float filtered_raw = raw_filter.filter(scale.read());  // Konsisten menggunakan read_average
  updateSystemStability(filtered_raw);

  int current_pill_prediction;


  if (isFrozen) {
    current_pill_prediction = last_stable_pill_count;  // [BARU] Gunakan nilai beku
  } else {
    current_pill_prediction = classifyPillCount(filtered_raw);
    last_stable_pill_count = current_pill_prediction;  // Perbarui nilai stabil terakhir
  }

  runStabilityGuardian(filtered_raw, current_pill_prediction);

  printDiagnostics(filtered_raw, current_pill_prediction);


  delay(1);
}
