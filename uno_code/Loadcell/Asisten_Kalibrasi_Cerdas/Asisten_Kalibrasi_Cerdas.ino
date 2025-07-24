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
EMAFilter raw_filter(0.16);  // Alpha yang telah Anda tuning
const int JUMLAH_TITIK_KALIBRASI = 8;
int jumlah_obat_kalibrasi[JUMLAH_TITIK_KALIBRASI] = { 0, 1, 2, 5, 10, 15, 20, 25 };
const float BERAT_SATU_OBAT = 1.55f;

// --- Variabel State untuk Asisten ---
enum CalState { STATE_INSTRUCT,
                STATE_STABILIZING,
                STATE_CONFIRM };
CalState currentState = STATE_INSTRUCT;
int current_step = 0;
long detected_stable_value = 0;

// --- Parameter untuk Deteksi Stabilitas ---
const int STABILITY_SAMPLES = 20;      // Butuh 20 sampel stabil berturut-turut
const long STABILITY_TOLERANCE = 100;  // Toleransi getaran maksimum
long stability_buffer[STABILITY_SAMPLES];
int stability_counter = 0;

// --- Variabel Hasil Akhir ---
float final_calibration_weights[JUMLAH_TITIK_KALIBRASI];
long final_calibration_raws[JUMLAH_TITIK_KALIBRASI];

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);
  Serial.println("--- Asisten Kalibrasi Cerdas ---");
}

void loop() {
  switch (currentState) {
    case STATE_INSTRUCT:
      // Memberi instruksi kepada pengguna
      final_calibration_weights[current_step] = jumlah_obat_kalibrasi[current_step] * BERAT_SATU_OBAT;
      Serial.print("\nLangkah ");
      Serial.print(current_step + 1);
      Serial.print("/");
      Serial.print(JUMLAH_TITIK_KALIBRASI);
      Serial.print(": Letakkan total ");
      Serial.print(jumlah_obat_kalibrasi[current_step]);
      Serial.print(" buah obat (target: ");
      Serial.print(final_calibration_weights[current_step], 2);
      Serial.println("g).");
      Serial.println("Memulai pemantauan langsung...");
      stability_counter = 0;  // Reset penghitung stabilitas
      currentState = STATE_STABILIZING;
      break;

    case STATE_STABILIZING:
      // Menampilkan data langsung dan mendeteksi stabilitas
      if (scale.is_ready()) {
        long raw_value = scale.read();
        float filtered_raw = raw_filter.filter(raw_value);

        Serial.print("Nilai Terfilter Saat Ini: ");
        Serial.println(long(filtered_raw));

        // Cek stabilitas
        long min_val = filtered_raw;
        long max_val = filtered_raw;
        for (int i = 0; i < STABILITY_SAMPLES - 1; i++) {
          stability_buffer[i] = stability_buffer[i + 1];
          if (stability_buffer[i] < min_val) min_val = stability_buffer[i];
          if (stability_buffer[i] > max_val) max_val = stability_buffer[i];
        }
        stability_buffer[STABILITY_SAMPLES - 1] = filtered_raw;

        if ((max_val - min_val) < STABILITY_TOLERANCE) {
          stability_counter++;
        } else {
          stability_counter = 0;  // Reset jika ada gejolak
        }

        if (stability_counter >= STABILITY_SAMPLES) {
          detected_stable_value = filtered_raw;
          Serial.print("\n==> Nilai stabil terdeteksi: ");
          Serial.println(detected_stable_value);
          Serial.print("    Simpan nilai ini untuk ");
          Serial.print(final_calibration_weights[current_step], 2);
          Serial.print("g? (y/n): ");
          currentState = STATE_CONFIRM;
        }
      }
      delay(200);
      break;

    case STATE_CONFIRM:
      // Menunggu konfirmasi dari pengguna
      if (Serial.available() > 0) {
        char input = Serial.read();
        while (Serial.available() > 0) Serial.read();  // Bersihkan buffer

        if (input == 'y' || input == 'Y') {
          Serial.println("\nNilai tersimpan.");
          final_calibration_raws[current_step] = detected_stable_value;
          current_step++;
          if (current_step >= JUMLAH_TITIK_KALIBRASI) {
            // Semua langkah selesai
            Serial.println("\n--- KALIBRASI SELESAI ---");
            Serial.println("Tabel kalibrasi akhir Anda:");
            for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
              Serial.print(final_calibration_weights[i], 2);
              Serial.print("g\t\t->\t");
              Serial.println(final_calibration_raws[i]);
            }
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
