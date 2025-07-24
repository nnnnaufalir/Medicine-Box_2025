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

// --- Konfigurasi ---
EMAFilter raw_filter(0.16);               // Alpha yang telah Anda tuning
const int JUMLAH_LANGKAH_KALIBRASI = 26;  // Untuk 0 hingga 25 obat

// --- Variabel State untuk Asisten ---
enum CalState { STATE_INSTRUCT,
                STATE_STABILIZING,
                STATE_CONFIRM };
CalState currentState = STATE_INSTRUCT;
int current_step = 0;
long detected_stable_value = 0;

// --- Parameter untuk Deteksi Stabilitas ---
const int STABILITY_SAMPLES = 20;
const long STABILITY_TOLERANCE = 100;
long stability_buffer[STABILITY_SAMPLES];
int stability_counter = 0;

// --- Variabel Hasil Akhir ---
long final_calibration_raws[JUMLAH_LANGKAH_KALIBRASI];

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);
  Serial.println("--- Asisten Kalibrasi KNN (Metode Titik Tunggal) ---");
}

void loop() {
  switch (currentState) {
    case STATE_INSTRUCT:
      Serial.print("\nLangkah ");
      Serial.print(current_step + 1);
      Serial.print("/");
      Serial.print(JUMLAH_LANGKAH_KALIBRASI);
      Serial.print(": Letakkan total ");
      Serial.print(current_step);
      Serial.println(" buah obat.");
      Serial.println("Memulai pemantauan langsung...");
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
          detected_stable_value = filtered_raw;
          Serial.print("\n==> Nilai stabil terdeteksi: ");
          Serial.println(detected_stable_value);
          Serial.print("    Simpan nilai ini? (y/n): ");
          currentState = STATE_CONFIRM;
        }
      }
      delay(200);
      break;

    case STATE_CONFIRM:
      if (Serial.available() > 0) {
        char input = Serial.read();
        while (Serial.available() > 0) Serial.read();

        if (input == 'y' || input == 'Y') {
          final_calibration_raws[current_step] = detected_stable_value;
          current_step++;
          if (current_step >= JUMLAH_LANGKAH_KALIBRASI) {
            // Semua langkah selesai
            Serial.println("\n--- PENGUMPULAN DATA SELESAI ---");
            Serial.println("Salin dan tempel array berikut ke program pengukuran KNN Anda:");
            Serial.println("long knn_dataset_raw_values[26] = {");
            for (int i = 0; i < JUMLAH_LANGKAH_KALIBRASI; i++) {
              Serial.print("  ");
              Serial.print(final_calibration_raws[i]);
              if (i < JUMLAH_LANGKAH_KALIBRASI - 1) {
                Serial.println(",");
              } else {
                Serial.println();
              }
            }
            Serial.println("};");
            while (1)
              ;  // Hentikan program
          } else {
            currentState = STATE_INSTRUCT;
          }
        } else if (input == 'n' || input == 'N') {
          Serial.println("\nNilai dibuang. Mengulang pemantauan...");
          currentState = STATE_STABILIZING;
        }
      }
      break;
  }
}
