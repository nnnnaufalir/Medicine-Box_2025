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
                STATE_CONFIRM,
                STATE_CALCULATE };
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
  Serial.println("--- Asisten Kalibrasi Regresi ---");
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
      // [PERUBAHAN] Langsung membaca tanpa is_ready()
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
            currentState = STATE_CALCULATE;
          } else {
            currentState = STATE_INSTRUCT;
          }
        } else if (input == 'n' || input == 'N') {
          currentState = STATE_STABILIZING;
        }
      }
      break;

    case STATE_CALCULATE:
      Serial.println("\n--- MENGHITUNG REGRESI LINEAR ---");
      double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
      for (int i = 0; i < JUMLAH_LANGKAH_KALIBRASI; i++) {
        sum_x += i;                          // x adalah jumlah obat
        sum_y += final_calibration_raws[i];  // y adalah nilai mentah
        sum_xy += (double)i * final_calibration_raws[i];
        sum_x2 += (double)i * i;
      }

      double n = JUMLAH_LANGKAH_KALIBRASI;
      double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
      double intercept = (sum_y - slope * sum_x) / n;

      Serial.println("\n--- HASIL KALIBRASI REGRESI ---");
      Serial.println("Gunakan dua nilai ini di program pengukuran akhir Anda:");
      Serial.print("const long OFFSET_FINAL = ");
      Serial.print(long(intercept));
      Serial.println(";");
      Serial.print("const float RAW_PER_OBAT_FINAL = ");
      Serial.print(slope, 4);
      Serial.println("f;");

      while (1)
        ;  // Hentikan program
      break;
  }
}
