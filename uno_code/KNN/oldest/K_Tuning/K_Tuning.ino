#include "HX711.h"

HX711 scale;

// --- [KONFIGURASI] ---
int k_value = 9;  // Nilai K awal, bisa diubah saat runtime

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
EMAFilter raw_filter(0.16);

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
KNNDataPoint knn_dataset_adjusted[JUMLAH_KELAS];

// Variabel Logika Fotografer & Penjaga Stabilitas
bool isSystemStable = true;
unsigned long lastInteractionTime = 0;
float lastFilteredRawForStabilityCheck = 0.0;
float anchor_filtered_raw;
unsigned long last_drift_check_time = 0;

const unsigned long SETTLING_TIME_MS = 4000;
const float INTERACTION_THRESHOLD_RAW = 500;
const unsigned long DRIFT_CHECK_INTERVAL_MS = 10000;
const long STABILITY_THRESHOLD_RAW = 3000;

// --- FUNGSI-FUNGSI LOGIKA ---

void adjustDatasetForDrift() {
  Serial.println("Menyesuaikan dataset berdasarkan drift saat ini...");
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
  raw_filter.filter(current_zero_raw);
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

void runStabilityGuardian() {
  if (millis() - last_drift_check_time < DRIFT_CHECK_INTERVAL_MS) return;

  Serial.println("\n--- PENJAGA STABILITAS AKTIF ---");
  if (isSystemStable) {
    Serial.println("   --> Status: STABIL. Memeriksa drift...");
    float current_stable_raw = raw_filter.filter(scale.read_average(20));
    long drift_diff = current_stable_raw - anchor_filtered_raw;

    Serial.print("   Titik Jangkar Lama : ");
    Serial.println(long(anchor_filtered_raw));
    Serial.print("   Titik Stabil Saat Ini: ");
    Serial.println(long(current_stable_raw));
    Serial.print("   Selisih Absolut    : ");
    Serial.println(abs(drift_diff));
    Serial.print("   Ambang Batas       : ");
    Serial.println(STABILITY_THRESHOLD_RAW);

    if (abs(drift_diff) < STABILITY_THRESHOLD_RAW) {
      Serial.print("   --> KEPUTUSAN: Drift terdeteksi: ");
      Serial.println(drift_diff);
      for (int i = 0; i < JUMLAH_KELAS; i++) {
        for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
          knn_dataset_adjusted[i].nilai_mentah[j] += drift_diff;
        }
      }
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
  const int MAX_K_SUPPORTED = 15;  // Ukuran maksimum array untuk keamanan
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

  int votes[JUMLAH_KELAS] = { 0 };
  for (int i = 0; i < k_value; i++) {
    votes[nearest_classes[i]]++;
  }

  int max_votes = 0;
  int result = 0;
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    if (votes[i] > max_votes) {
      max_votes = votes[i];
      result = i;
    }
  }

  // [DIAGNOSTIK BARU] Tampilkan detail pemungutan suara
  Serial.println("\n--- Analisis KNN ---");
  Serial.print("   Tetangga Terdekat (Obat): { ");
  for (int i = 0; i < k_value; i++) {
    Serial.print(nearest_classes[i]);
    if (i < k_value - 1) Serial.print(", ");
  }
  Serial.println(" }");
  Serial.print("   Suara Mayoritas: ");
  Serial.print(max_votes);
  Serial.print(" untuk kelas ");
  Serial.print(result);
  Serial.println(" obat");
  Serial.println("--------------------");

  return result;
}

// --- FUNGSI UTAMA ---

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Laboratorium KNN Interaktif...");
  Serial.println("Perintah: K=[nilai] (contoh: K=7)");
  scale.begin(13, 14);

  adjustDatasetForDrift();

  anchor_filtered_raw = raw_filter.filter(scale.read_average(20));
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
      // [PERUBAHAN] Batas bawah 1, tanpa batas atas
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

  // [OUTPUT UTAMA YANG DIPERBARUI]
  Serial.println("========================================================================");
  Serial.print("K: ");
  Serial.print(k_value);
  Serial.print(" | Nilai Terfilter: ");
  Serial.print(long(filtered_raw));
  Serial.print(" | Status: ");
  Serial.print(isSystemStable ? "STABIL" : "SIBUK ");
  Serial.print(" | Prediksi Obat: ");
  Serial.println(pill_count);

  runStabilityGuardian();

  delay(100);
}
