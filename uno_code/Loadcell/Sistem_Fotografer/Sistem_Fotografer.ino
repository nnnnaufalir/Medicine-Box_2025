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

// --- Parameter & Variabel Sistem ---
EMAFilter raw_filter(0.16);
EMAFilter weight_filter(0.08);

// Tabel kalibrasi Anda
const int JUMLAH_TITIK_KALIBRASI = 8;
float calibration_weights[JUMLAH_TITIK_KALIBRASI] = { 0.0, 1.55, 3.10, 7.75, 15.50, 23.25, 31.00, 38.75 };
long calibration_raw_values[JUMLAH_TITIK_KALIBRASI] = { 297285, 301713, 307905, 324348, 346881, 376932, 407826, 437476 };

// --- Logika Penjaga Stabilitas ---
float anchor_filtered_raw;
const unsigned long DRIFT_CHECK_INTERVAL_MS = 10000;
const long STABILITY_THRESHOLD_RAW = 3000;
unsigned long last_drift_check_time = 0;

// [LOGIKA FOTOGRAFER] Variabel Status
bool isSystemStable = true;
unsigned long lastInteractionTime = 0;
float lastFilteredRawForStabilityCheck = 0.0;
const unsigned long SETTLING_TIME_MS = 4000;  // Masa tenang 4 detik
const float INTERACTION_THRESHOLD_RAW = 500;  // Perubahan mentah yang dianggap interaksi

// --- Fungsi Interpolasi ---
float interpolate(long raw_value) {
  if (raw_value <= calibration_raw_values[0]) {
    long x1 = calibration_raw_values[0];
    float y1 = calibration_weights[0];
    long x2 = calibration_raw_values[1];
    float y2 = calibration_weights[1];
    return y1 + ((float)(raw_value - x1) * (y2 - y1)) / (float)(x2 - x1);
  }
  if (raw_value >= calibration_raw_values[JUMLAH_TITIK_KALIBRASI - 1]) {
    long x1 = calibration_raw_values[JUMLAH_TITIK_KALIBRASI - 2];
    float y1 = calibration_weights[JUMLAH_TITIK_KALIBRASI - 2];
    long x2 = calibration_raw_values[JUMLAH_TITIK_KALIBRASI - 1];
    float y2 = calibration_weights[JUMLAH_TITIK_KALIBRASI - 1];
    return y2 + ((float)(raw_value - x2) * (y2 - y1)) / (float)(x2 - x1);
  }
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
  Serial.println("Inisialisasi Sistem Fotografer (Final)...");
  scale.begin(13, 14);

  // Koreksi drift awal
  long original_zero_raw = calibration_raw_values[0];
  long current_zero_raw = scale.read_average(30);
  long drift_offset = current_zero_raw - original_zero_raw;
  for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
    calibration_raw_values[i] += drift_offset;
  }

  // Inisialisasi filter dan titik jangkar
  anchor_filtered_raw = raw_filter.filter(scale.read_average(20));
  lastFilteredRawForStabilityCheck = anchor_filtered_raw;
  last_drift_check_time = millis();
  Serial.println("Sistem siap.");
}

void loop() {
  long raw_reading = scale.read();
  float filtered_raw = raw_filter.filter(raw_reading);
  float interpolate_weight = interpolate(filtered_raw);
  float accurate_weight = weight_filter.filter(interpolate_weight);

  // [LOGIKA FOTOGRAFER] Deteksi Stabilitas Sistem
  if (abs(filtered_raw - lastFilteredRawForStabilityCheck) > INTERACTION_THRESHOLD_RAW) {
    isSystemStable = false;
    lastInteractionTime = millis();
  }

  if (!isSystemStable && (millis() - lastInteractionTime > SETTLING_TIME_MS)) {
    isSystemStable = true;
  }
  lastFilteredRawForStabilityCheck = filtered_raw;

  // Output utama
  Serial.print("Berat: ");
  Serial.print(accurate_weight, 2);
  Serial.print(" g | Status: ");
  Serial.println(isSystemStable ? "STABIL" : "SIBUK ");

  // --- Logika Penjaga Stabilitas ---
  if (millis() - last_drift_check_time > DRIFT_CHECK_INTERVAL_MS) {
    if (isSystemStable) {
      float current_stable_raw = raw_filter.filter(scale.read_average(10));
      long drift_diff = current_stable_raw - anchor_filtered_raw;

      if (abs(drift_diff) < STABILITY_THRESHOLD_RAW) {
        for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
          calibration_raw_values[i] += drift_diff;
        }
      }
      anchor_filtered_raw = current_stable_raw;
    }
    last_drift_check_time = millis();
  }
  delay(1);
}
