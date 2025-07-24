#include "HX711.h"

HX711 scale;

// --- [KONFIGURASI] ---
int k_value = 17;  

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
struct KNNDataPoint {
  int jumlah_obat;
  long nilai_mentah[SAMPEL_PER_KELAS];
};

// --- Parameter & Variabel Sistem ---
EMAFilter raw_filter(0.16);  //0.16//0.08/0.32//0.24

const int JUMLAH_KELAS = 26;
const KNNDataPoint knn_dataset_original[JUMLAH_KELAS] = {
  { 0, { 295870, 295935, 295998, 296061, 296125, 296187, 296248, 296310, 296373, 296436, 296499 } },
  { 1, { 301167, 301213, 301259, 301305, 301351, 301398, 301444, 301490, 301536, 301582, 301629 } },
  { 2, { 308414, 308458, 308502, 308546, 308590, 308634, 308678, 308722, 308766, 308810, 308854 } },
  { 3, { 312686, 312734, 312782, 312830, 312878, 312926, 312974, 313022, 313070, 313118, 313166 } },
  { 4, { 317850, 317896, 317942, 317988, 318034, 318081, 318127, 318173, 318219, 318265, 318311 } },
  { 5, { 324819, 324865, 324911, 324957, 325004, 325050, 325096, 325142, 325188, 325234, 325280 } },
  { 6, { 329512, 329568, 329624, 329680, 329736, 329792, 329848, 329904, 329960, 330016, 330072 } },
  { 7, { 337018, 337056, 337094, 337132, 337170, 337208, 337246, 337284, 337322, 337360, 337398 } },
  { 8, { 340261, 340312, 340363, 340414, 340465, 340516, 340567, 340618, 340669, 340720, 340771 } },
  { 9, { 346176, 346223, 346270, 346317, 346364, 346411, 346458, 346505, 346552, 346599, 346646 } },
  { 10, { 352754, 352798, 352842, 352886, 352930, 352975, 353019, 353063, 353107, 353151, 353195 } },
  { 11, { 358228, 358267, 358306, 358345, 358384, 358423, 358462, 358501, 358540, 358579, 358618 } },
  { 12, { 365804, 365848, 365892, 365936, 365980, 366025, 366069, 366113, 366157, 366201, 366246 } },
  { 13, { 371815, 371850, 371885, 371920, 371955, 371990, 372025, 372060, 372095, 372130, 372165 } },
  { 14, { 377911, 377947, 377983, 378019, 378055, 378091, 378127, 378163, 378199, 378235, 378271 } },
  { 15, { 382898, 382928, 382958, 382988, 383018, 383048, 383078, 383108, 383138, 383168, 383198 } },
  { 16, { 388288, 388323, 388358, 388393, 388428, 388463, 388498, 388533, 388568, 388603, 388638 } },
  { 17, { 395093, 395127, 395161, 395195, 395229, 395263, 395297, 395331, 395365, 395399, 395433 } },
  { 18, { 400407, 400445, 400483, 400521, 400559, 400597, 400635, 400673, 400711, 400749, 400787 } },
  { 19, { 408019, 408061, 408103, 408145, 408187, 408229, 408271, 408313, 408355, 408397, 408439 } },
  { 20, { 414128, 414158, 414188, 414218, 414248, 414278, 414308, 414338, 414368, 414398, 414428 } },
  { 21, { 419822, 419862, 419902, 419942, 419982, 420022, 420062, 420102, 420142, 420182, 420222 } },
  { 22, { 424360, 424403, 424446, 424489, 424532, 424575, 424618, 424661, 424704, 424747, 424790 } },
  { 23, { 428080, 428113, 428146, 428179, 428212, 428245, 428278, 428311, 428344, 428377, 428410 } },
  { 24, { 433496, 433534, 433572, 433610, 433648, 433686, 433724, 433762, 433800, 433838, 433876 } },
  { 25, { 438752, 438784, 438816, 438848, 438880, 438912, 438944, 438976, 439008, 439040, 439072 } }
};

KNNDataPoint knn_dataset_adjusted[JUMLAH_KELAS];

// Variabel Logika Fotografer & Penjaga Stabilitas
bool isSystemStable = true;
float current_stable_raw = 0;
unsigned long lastInteractionTime = 0;
float lastFilteredRawForStabilityCheck = 0.0;
float anchor_filtered_raw;
unsigned long last_drift_check_time = 0;
int pill_count = 0;

const unsigned long SETTLING_TIME_MS = 4000;
const float INTERACTION_THRESHOLD_RAW = 500;
const unsigned long DRIFT_CHECK_INTERVAL_MS = 10000;
const long STABILITY_THRESHOLD_RAW = 3000;

const unsigned long REFINEMENT_PERIOD_MS = 60000;
unsigned long startup_time = 0;

// --- FUNGSI-FUNGSI LOGIKA ---

void adjustDatasetForDrift() {
  Serial.println("Menyesuaikan dataset berdasarkan drift saat ini...");
  long original_zero_raw = knn_dataset_original[0].nilai_mentah[5];
  long current_zero_raw = scale.read_average(80);
  // [PERBAIKAN BUG] Menggunakan selisih penuh, bukan dibagi dua
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

void adjustDatasetByOffset(long offset) {
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      knn_dataset_adjusted[i].nilai_mentah[j] += offset;
    }
  }
}

void updateSystemStability(float current_filtered_raw) {
  if (abs(current_filtered_raw - lastFilteredRawForStabilityCheck) > INTERACTION_THRESHOLD_RAW) {
    isSystemStable = false;
    lastInteractionTime = millis();
  }

  if (!isSystemStable && (millis() - lastInteractionTime > SETTLING_TIME_MS)) {
    isSystemStable = true;
  }
  lastFilteredRawForStabilityCheck = current_filtered_raw;
}

void runStabilityGuardian(int current_pill_count) {
  if (millis() - last_drift_check_time < DRIFT_CHECK_INTERVAL_MS) return;

  Serial.println("\n--- PENJAGA STABILITAS AKTIF ---");
  if (isSystemStable) {
    Serial.println("   --> Status: STABIL. Memeriksa drift...");
    // float current_stable_raw = raw_filter.filter(scale.read_average(20));
    long drift_diff = current_stable_raw - anchor_filtered_raw;

    Serial.print("   Titik Jangkar Lama : ");
    Serial.println(long(anchor_filtered_raw));
    Serial.print("   Titik Stabil Saat Ini: ");
    Serial.println(long(current_stable_raw));
    Serial.print("   Selisih Absolut    : ");
    Serial.println(abs(drift_diff));
    Serial.print("   Ambang Batas       : ");
    Serial.println(STABILITY_THRESHOLD_RAW);

    // [LOGIKA BARU] Fase Penyempurnaan (Refinement)
    if (millis() - startup_time < REFINEMENT_PERIOD_MS && current_pill_count == 0) {
      Serial.println("Refinment aktif!");
      // Selama 2 menit pertama, jika sistem stabil dan kosong, sempurnakan titik nol.
      // Gunakan EMA yang sangat lambat untuk menyempurnakan.
      long refined_zero_point = (0.95 * knn_dataset_adjusted[0].nilai_mentah[5]) + (0.05 * current_stable_raw);
      long refinement_offset = refined_zero_point - knn_dataset_adjusted[0].nilai_mentah[5];

      adjustDatasetByOffset(refinement_offset);  // Geser seluruh dataset

    }

    else if (abs(drift_diff) < STABILITY_THRESHOLD_RAW) {
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
  const int MAX_K_SUPPORTED = 99;
  if (k_value > MAX_K_SUPPORTED) k_value = MAX_K_SUPPORTED;

  long nearest_distances[MAX_K_SUPPORTED];
  int nearest_classes[MAX_K_SUPPORTED];

  for (int i = 0; i < k_value; i++) {
    nearest_distances[i] = LONG_MAX;
  }

  for (int i = 0; i < JUMLAH_KELAS; i++) {
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      long distance = abs(long(current_filtered_raw) - knn_dataset_adjusted[i].nilai_mentah[j]);
      for (int k = 0; k < k_value; k++) {
        if (distance < nearest_distances[k]) {
          for (int m = k_value - 1; m > k; m--) {
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
  for (int i = 0; i < k_value; i++) {
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
  for (int i = 0; i < k_value; i++) {
    Serial.print(nearest_classes[i]);
    if (i < k_value - 1) Serial.print(", ");
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

// --- FUNGSI UTAMA ---

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Laboratorium KNN Interaktif (v3)...");
  Serial.println("Perintah: K=[nilai] (contoh: K=7)");
  scale.begin(13, 14);
  startup_time = millis();

  adjustDatasetForDrift();

  lastFilteredRawForStabilityCheck = anchor_filtered_raw;
  last_drift_check_time = millis();
  Serial.println("Sistem siap.");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.startsWith("K=")) {
      int new_k = command.substring(2).toInt();
      if (new_k > 0 && new_k % 2 != 0) {
        k_value = new_k;
        Serial.print("\n>>> Nilai K diubah menjadi: ");
        Serial.println(k_value);
      } else {
        Serial.println("\n>>> Input K tidak valid. Harus ganjil dan > 0.");
      }
    }
  }

  float filtered_raw = raw_filter.filter(scale.read());
  updateSystemStability(filtered_raw);
  int pill_count = classifyPillCount(filtered_raw);

  Serial.println("========================================================================");
  Serial.print("K: ");
  Serial.print(k_value);
  Serial.print(" | Nilai Terfilter: ");
  Serial.print(long(filtered_raw));
  Serial.print(" | Titik 0: ");
  Serial.print(knn_dataset_adjusted[0].nilai_mentah[5]);
  Serial.print(" | Status: ");
  Serial.print(isSystemStable ? "STABIL" : "SIBUK ");
  Serial.print(" | Prediksi Obat: ");
  Serial.println(pill_count);

  current_stable_raw = filtered_raw;
  runStabilityGuardian(pill_count);
  delay(1);
}
