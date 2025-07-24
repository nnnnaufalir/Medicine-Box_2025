#include "HX711.h"

HX711 scale;

// --- [KONFIGURASI] ---
int k_value = 17;  // Nilai K awal, bisa diubah saat runtime

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
EMAFilter raw_filter(0.24); //0.16//0.08/0.32//0.24

const int JUMLAH_KELAS = 26;
const KNNDataPoint knn_dataset_original[JUMLAH_KELAS] = {
  { 0, { 298523, 298564, 298603, 298644, 298684, 298725, 298764, 298805, 298845, 298886, 298900 } },
  { 1, { 303850, 304890, 304930, 304969, 305009, 305049, 305089, 305129, 305168, 305208, 305248 } },
  { 2, { 311057, 311087, 311118, 311148, 311179, 311209, 311240, 311270, 311301, 311331, 311362 } },
  { 3, { 317072, 317112, 317152, 317192, 317233, 317273, 317313, 317354, 317394, 317434, 317475 } },
  { 4, { 321342, 321380, 321416, 321452, 321487, 321525, 321562, 321598, 321635, 321651, 321669 } },
  { 5, { 325431, 328468, 328505, 328543, 328580, 328618, 328655, 328692, 328729, 328766, 328804 } },
  { 6, { 335254, 335295, 335336, 335378, 335420, 335462, 335503, 335544, 335586, 335628, 335670 } },
  { 7, { 342926, 342964, 343002, 343040, 343078, 343116, 343153, 343191, 343229, 343267, 343305 } },
  { 8, { 347033, 347075, 347117, 347159, 347201, 347243, 347285, 347327, 347369, 347411, 347454 } },
  { 9, { 350694, 352738, 352782, 352826, 352871, 352915, 352959, 353003, 353047, 353091, 353100 } },
  { 10, { 357072, 357107, 358143, 358179, 358215, 358251, 358286, 358322, 358357, 358392, 358429 } },
  { 11, { 362194, 362228, 362263, 362298, 362333, 362368, 362402, 362437, 362472, 362507, 362542 } },
  { 12, { 366614, 366661, 366708, 366755, 366802, 366850, 366896, 366944, 366990, 370764, 370804 } },
  { 13, { 372254, 372295, 372337, 372379, 372421, 372463, 372505, 372546, 372589, 372630, 372673 } },
  { 14, { 379763, 379812, 379862, 379911, 379961, 380010, 380059, 380109, 380158, 380208, 380257 } },
  { 15, { 386589, 386630, 386672, 386713, 386755, 386797, 386839, 386880, 386922, 386963, 387006 } },
  { 16, { 393133, 393178, 393224, 393270, 393316, 393362, 393408, 393454, 393500, 393545, 393592 } },
  { 17, { 400155, 400190, 400224, 400259, 400293, 400328, 400363, 400397, 400432, 400466, 400501 } },
  { 18, { 405313, 405358, 405404, 405450, 405496, 405542, 405587, 405633, 405679, 405725, 405771 } },
  { 19, { 411297, 411365, 411432, 411500, 411568, 411636, 411703, 411771, 411839, 411906, 411974 } },
  { 20, { 416668, 416698, 416727, 416757, 416787, 416817, 416847, 416876, 416906, 416936, 416966 } },
  { 21, { 421284, 421323, 421362, 421401, 421441, 421480, 421519, 421559, 421598, 421637, 421677 } },
  { 22, { 424789, 424836, 424882, 424929, 424975, 425022, 425068, 425115, 425161, 425208, 425254 } },
  { 23, { 428969, 429002, 429036, 429069, 429102, 429136, 429169, 429202, 429235, 429269, 429302 } },
  { 24, { 434912, 434948, 434984, 435019, 435055, 435091, 435127, 435163, 435198, 435234, 435270 } },
  { 25, { 439767, 439800, 439833, 439865, 439898, 439932, 439964, 439997, 440030, 440063, 440096 } }
};

KNNDataPoint knn_dataset_adjusted[JUMLAH_KELAS];

// Variabel Logika Fotografer & Penjaga Stabilitas
bool isSystemStable = true;
float current_stable_raw = 0;
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
  runStabilityGuardian();
  delay(200);
}
