#include "HX711.h"

HX711 scale;

long drift_offset;

// --- Kelas Filter EMA Sederhana (tidak berubah) ---
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

EMAFilter raw_filter(0.16);

// --- Tabel Kalibrasi Asli (dari alat kalibrasi) ---
const int JUMLAH_TITIK_KALIBRASI = 8;
float calibration_weights[JUMLAH_TITIK_KALIBRASI] = { 0.0, 1.55, 3.10, 7.75, 15.50, 23.25, 31.00, 38.75 };
long calibration_raw_values[JUMLAH_TITIK_KALIBRASI] = { 297285, 301713, 307905, 324348, 346881, 376932, 407826, 437476 };

// --- Fungsi Interpolasi Linear (tidak berubah) ---
float interpolate(long raw_value) {
  if (raw_value <= calibration_raw_values[0]) return calibration_weights[0];
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
  Serial.println("Inisialisasi Timbangan Akurasi Tinggi (v3)...");
  scale.begin(13, 14);

  // --- [LOGIKA BARU] Kalibrasi Ulang Titik Nol Otomatis ---
  Serial.println("Menyesuaikan titik nol berdasarkan kondisi saat ini...");
  long original_zero_raw = calibration_raw_values[0];
  long current_zero_raw = scale.read_average(99);  // Baca titik nol saat ini
  drift_offset = current_zero_raw - original_zero_raw;

  Serial.print("Drift terdeteksi: ");
  Serial.println(drift_offset);

  // Geser seluruh tabel kalibrasi berdasarkan drift
  for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
    calibration_raw_values[i] += drift_offset;
  }
  Serial.println("Tabel kalibrasi telah disesuaikan.");
  Serial.println("------------------------------------");
  Serial.println("Sistem siap. Pengukuran dimulai.");
}

void loop() {
  long raw_reading = scale.read();
  float filtered_raw = raw_filter.filter(raw_reading);
  float accurate_weight = interpolate(filtered_raw);

  Serial.print("offset:");
  Serial.print(float(drift_offset),2);
  Serial.print(" , ");
  Serial.print("Mentah:");
  Serial.print(float(raw_reading), 2);
  Serial.print(" , ");
  Serial.print("Filter:");
  Serial.print(filtered_raw, 2);
  Serial.print(" | ");
  Serial.print("Berat Akurat:");
  Serial.print(accurate_weight, 2);
  Serial.print(" g  ||  ");
  Serial.print("obat1:");
  Serial.print(accurate_weight / 1.55);
  Serial.print(" , ");
  Serial.print("obat2:");
  Serial.print(accurate_weight / 1.6);
  Serial.println(" ");

  delay(1);
}
