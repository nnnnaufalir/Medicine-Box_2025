#include "HX711.h"

HX711 scale;

// --- Kelas Filter EMA (tidak berubah) ---
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

// --- Parameter & Variabel Sistem ---
EMAFilter raw_filter(0.16);
EMAFilter weight_filter(0.08);

// Tabel kalibrasi asli Anda
const int JUMLAH_TITIK_KALIBRASI = 8;
float calibration_weights[JUMLAH_TITIK_KALIBRASI] = { 0.0, 1.55, 3.10, 7.75, 15.50, 23.25, 31.00, 38.75 };
long calibration_raw_values[JUMLAH_TITIK_KALIBRASI] = { 297285, 301713, 307905, 324348, 346881, 376932, 407826, 437476 };

// --- Logika Penjaga Stabilitas ---
float anchor_filtered_raw;
const unsigned long DRIFT_CHECK_INTERVAL_MS = 10000;
const long STABILITY_THRESHOLD_RAW = 3000;
unsigned long last_drift_check_time = 0;


// --- Fungsi Interpolasi (Telah Diperbaiki) ---
float interpolate(long raw_value) {
  // [PERBAIKAN] Izinkan ekstrapolasi ke bawah (negatif)
  if (raw_value <= calibration_raw_values[0]) {
    long x1 = calibration_raw_values[0];
    float y1 = calibration_weights[0];  // y1 akan 0.0
    long x2 = calibration_raw_values[1];
    float y2 = calibration_weights[1];
    // Lakukan ekstrapolasi linear berdasarkan dua titik pertama
    return y1 + ((float)(raw_value - x1) * (y2 - y1)) / (float)(x2 - x1);
  }

  // Ekstrapolasi ke atas (tidak berubah)
  if (raw_value >= calibration_raw_values[JUMLAH_TITIK_KALIBRASI - 1]) {
    long x1 = calibration_raw_values[JUMLAH_TITIK_KALIBRASI - 2];
    float y1 = calibration_weights[JUMLAH_TITIK_KALIBRASI - 2];
    long x2 = calibration_raw_values[JUMLAH_TITIK_KALIBRASI - 1];
    float y2 = calibration_weights[JUMLAH_TITIK_KALIBRASI - 1];
    return y2 + ((float)(raw_value - x2) * (y2 - y1)) / (float)(x2 - x1);
  }

  // Interpolasi di antara titik (tidak berubah)
  int i = 0;
  while (raw_value > calibration_raw_values[i + 1]) i++;
  long x1 = calibration_raw_values[i];
  float y1 = calibration_weights[i];
  long x2 = calibration_raw_values[i + 1];
  float y2 = calibration_weights[i + 1];
  return y1 + ((float)(raw_value - x1) * (y2 - y1)) / (float)(x2 - x1);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Sistem Penjaga Stabilitas (Final)...");
  scale.begin(13, 14);

  // Koreksi drift awal saat startup
  long original_zero_raw = calibration_raw_values[0];
  long current_zero_raw = scale.read_average(30);
  long drift_offset = current_zero_raw - original_zero_raw;
  for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
    calibration_raw_values[i] += drift_offset;
  }
  Serial.println("Koreksi drift awal selesai.");

  // Inisialisasi filter dan titik jangkar
  anchor_filtered_raw = raw_filter.filter(scale.read_average(20));
  last_drift_check_time = millis();
  Serial.println("Sistem siap. Pengukuran dimulai.");
}

void loop() {
  long raw_reading = scale.read();
  float filtered_raw = raw_filter.filter(raw_reading);
  float interpolate_weight = interpolate(filtered_raw);
  float accurate_weight = weight_filter.filter(interpolate_weight);

  Serial.print("Titik 0: ");
  Serial.print(calibration_raw_values[0]);
  Serial.print(" | Berat: ");
  Serial.print(accurate_weight, 2);
  Serial.print(" g  |  Mentah Terfilter: ");
  Serial.println(long(filtered_raw));


  // --- Logika Penjaga Stabilitas ---
  if (millis() - last_drift_check_time > DRIFT_CHECK_INTERVAL_MS) {
    Serial.println("\n--- PENJAGA STABILITAS AKTIF ---");
    float current_stable_raw = raw_filter.filter(scale.read_average(20));
    long drift_diff = current_stable_raw - anchor_filtered_raw;

    if (abs(drift_diff) < STABILITY_THRESHOLD_RAW) {
      Serial.print("   --> KEPUTUSAN: Drift terdeteksi: ");
      Serial.println(drift_diff);
      for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
        calibration_raw_values[i] += drift_diff;
      }
      Serial.println("   Koreksi berhasil. Tabel kalibrasi diperbarui.");
    } else {
      Serial.println("   --> KEPUTUSAN: Perubahan berat signifikan. Tidak ada koreksi drift.");
    }

    // [DIAGNOSTIK] Tampilkan semua nilai yang digunakan untuk keputusan
    Serial.print("   Titik 0 baru : ");
    Serial.println(calibration_raw_values[0]);
    Serial.print("   Titik Jangkar Lama : ");
    Serial.println(long(anchor_filtered_raw));
    Serial.print("   Titik Stabil Saat Ini: ");
    Serial.println(long(current_stable_raw));
    Serial.print("   Selisih (Drift)    : ");
    Serial.println(abs(drift_diff));
    Serial.print("   Ambang Batas       : ");
    Serial.println(STABILITY_THRESHOLD_RAW);


    // Perbarui titik jangkar dan waktu pengecekan
    anchor_filtered_raw = current_stable_raw;
    last_drift_check_time = millis();
    Serial.println("--------------------------------\n");
  }

  delay(1);  // Delay yang lebih wajar untuk pembacaan serial
}
