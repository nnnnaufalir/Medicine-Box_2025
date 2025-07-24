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

// --- [STRUKTUR DATA BARU] ---
const int SAMPEL_PER_KELAS = 5;
struct KNNDataPoint {
  int jumlah_obat;
  long nilai_mentah[SAMPEL_PER_KELAS];
};

// --- Konfigurasi ---
EMAFilter raw_filter(0.16);
const int JUMLAH_KELAS = 26;

// --- Variabel State untuk Asisten ---
enum CalState { STATE_INSTRUCT,
                STATE_STABILIZING,
                STATE_SAMPLING,
                STATE_PROCESS,
                STATE_CONFIRM,
                STATE_DONE };
CalState currentState = STATE_INSTRUCT;
int current_class = 0;

// --- Parameter untuk Deteksi & Pengambilan Sampel ---
const int STABILITY_SAMPLES = 20;
const long STABILITY_TOLERANCE = 100;
long stability_buffer[STABILITY_SAMPLES];
int stability_counter = 0;

const int SAMPLING_COUNT = 100;
long sampling_buffer[SAMPLING_COUNT];
long temp_dataset_points[SAMPEL_PER_KELAS];

// --- Variabel Hasil Akhir ---
KNNDataPoint final_dataset[JUMLAH_KELAS];

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);
  Serial.println("--- Asisten Kalibrasi Jamak (Terstruktur) ---");
}

void loop() {
  switch (currentState) {
    case STATE_INSTRUCT:
      Serial.print("\nLangkah ");
      Serial.print(current_class + 1);
      Serial.print("/");
      Serial.print(JUMLAH_KELAS);
      Serial.print(": Letakkan total ");
      Serial.print(current_class);
      Serial.println(" buah obat.");
      Serial.println("Memulai pemantauan untuk mendeteksi stabilitas...");
      stability_counter = 0;
      currentState = STATE_STABILIZING;
      break;

    case STATE_STABILIZING:
      {
        float filtered_raw = raw_filter.filter(scale.read());
        Serial.print("Nilai Terfilter Saat Ini: ");
        Serial.println(long(filtered_raw));

        // Cek stabilitas
        long min_val = filtered_raw, max_val = filtered_raw;
        for (int i = 0; i < STABILITY_SAMPLES - 1; i++) {
          stability_buffer[i] = stability_buffer[i + 1];
          if (stability_buffer[i] < min_val) min_val = stability_buffer[i];
          if (stability_buffer[i] > max_val) max_val = stability_buffer[i];
        }
        stability_buffer[STABILITY_SAMPLES - 1] = filtered_raw;

        if ((max_val - min_val) < STABILITY_TOLERANCE) {
          stability_counter++;
        } else {
          stability_counter = 0;
        }

        if (stability_counter >= STABILITY_SAMPLES) {
          Serial.println("\n==> Nilai stabil terdeteksi. Memulai pengambilan sampel otomatis...");
          currentState = STATE_SAMPLING;
        }
      }
      delay(200);
      break;

    case STATE_SAMPLING:
      for (int i = 0; i < SAMPLING_COUNT; i++) {
        sampling_buffer[i] = raw_filter.filter(scale.read());
        delay(50);
      }
      Serial.println("    Pengambilan sampel selesai. Memproses data...");
      currentState = STATE_PROCESS;
      break;

    case STATE_PROCESS:
      {
        long min_val = sampling_buffer[0], max_val = sampling_buffer[0];
        double sum = 0;
        for (int i = 0; i < SAMPLING_COUNT; i++) {
          sum += sampling_buffer[i];
          if (sampling_buffer[i] < min_val) min_val = sampling_buffer[i];
          if (sampling_buffer[i] > max_val) max_val = sampling_buffer[i];
        }
        long avg_val = sum / SAMPLING_COUNT;

        temp_dataset_points[0] = min_val;
        temp_dataset_points[1] = avg_val - (avg_val - min_val) / 2;
        temp_dataset_points[2] = avg_val;
        temp_dataset_points[3] = avg_val + (max_val - avg_val) / 2;
        temp_dataset_points[4] = max_val;

        Serial.println("    Data berikut telah dihasilkan:");
        Serial.print("    { ");
        for (int i = 0; i < SAMPEL_PER_KELAS; i++) {
          Serial.print(temp_dataset_points[i]);
          if (i < SAMPEL_PER_KELAS - 1) Serial.print(", ");
        }
        Serial.println(" }");
        Serial.print("    Simpan data ini? (y/n): ");
        currentState = STATE_CONFIRM;
      }
      break;

    case STATE_CONFIRM:
      if (Serial.available() > 0) {
        char input = Serial.read();
        while (Serial.available() > 0) Serial.read();

        if (input == 'y' || input == 'Y') {
          Serial.println("\nNilai tersimpan.");
          final_dataset[current_class].jumlah_obat = current_class;
          for (int i = 0; i < SAMPEL_PER_KELAS; i++) {
            final_dataset[current_class].nilai_mentah[i] = temp_dataset_points[i];
          }
          current_class++;
          if (current_class >= JUMLAH_KELAS) {
            currentState = STATE_DONE;
          } else {
            currentState = STATE_INSTRUCT;
          }
        } else if (input == 'n' || input == 'N') {
          Serial.println("\nNilai dibuang. Mengulang pemantauan...");
          currentState = STATE_STABILIZING;
        }
      }
      break;

    case STATE_DONE:
      Serial.println("\n--- PENGUMPULAN DATA SELESAI ---");
      Serial.println("Salin dan tempel array berikut ke program pengukuran KNN Anda:");
      Serial.println("const KNNDataPoint knn_dataset[26] = {");
      for (int i = 0; i < JUMLAH_KELAS; i++) {
        Serial.print("  { ");
        Serial.print(final_dataset[i].jumlah_obat);
        Serial.print(", { ");
        for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
          Serial.print(final_dataset[i].nilai_mentah[j]);
          if (j < SAMPEL_PER_KELAS - 1) Serial.print(", ");
        }
        Serial.print(" } }");
        if (i < JUMLAH_KELAS - 1) Serial.println(",");
        else Serial.println();
      }
      Serial.println("};");
      while (1)
        ;
      break;
  }
}
