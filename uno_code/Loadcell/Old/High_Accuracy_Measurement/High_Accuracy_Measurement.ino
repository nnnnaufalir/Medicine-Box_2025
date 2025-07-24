#include "HX711.h"

HX711 scale;

// --- Kelas Filter EMA Sederhana ---
class EMAFilter {
private:
  float alpha;
  float last_ema;
  bool has_run;
public:
  EMAFilter(float alpha_val)
    : alpha(alpha_val), last_ema(0.0f), has_run(false) {}

  float filter(float new_value) {
    if (!has_run) {
      last_ema = new_value;
      has_run = true;
    }
    last_ema = (alpha * new_value) + ((1.0 - alpha) * last_ema);
    return last_ema;
  }
};

// Inisialisasi filter EMA dengan faktor pemulusan 0.1
EMAFilter raw_filter(0.4);

// --- Tabel Kalibrasi (Berdasarkan Hasil Anda) ---
const int JUMLAH_TITIK_KALIBRASI = 6;

// Berat referensi yang kita gunakan (dalam gram)
float calibration_weights[JUMLAH_TITIK_KALIBRASI] = { 0.0, 1.6, 3.2, 4.8, 6.4, 50.0 };

// Nilai mentah yang Anda dapatkan dari alat kalibrasi
long calibration_raw_values[JUMLAH_TITIK_KALIBRASI] = { 286576, 290906, 297209, 303523, 308628, 424872 };


// --- Fungsi Interpolasi Linear ---
// Fungsi ini mengubah nilai mentah menjadi gram menggunakan tabel kalibrasi.
float interpolate(long raw_value) {
  // Jika di bawah titik kalibrasi terendah, kembalikan berat terendah.
  if (raw_value <= calibration_raw_values[0]) {
    return calibration_weights[0];
  }
  // Jika di atas titik kalibrasi tertinggi, kembalikan berat tertinggi.
  if (raw_value >= calibration_raw_values[JUMLAH_TITIK_KALIBRASI - 1]) {
    return calibration_weights[JUMLAH_TITIK_KALIBRASI - 1];
  }

  // Cari segmen yang mengapit nilai mentah saat ini
  int i = 0;
  while (raw_value > calibration_raw_values[i + 1]) {
    i++;
  }

  // Ekstrak titik x1, y1 (titik bawah) dan x2, y2 (titik atas)
  long x1 = calibration_raw_values[i];
  float y1 = calibration_weights[i];
  long x2 = calibration_raw_values[i + 1];
  float y2 = calibration_weights[i + 1];

  // Lakukan interpolasi linear
  // Rumus: y = y1 + (x - x1) * (y2 - y1) / (x2 - x1)
  return y1 + ((float)(raw_value - x1) * (y2 - y1)) / (float)(x2 - x1);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Timbangan Akurasi Tinggi...");
  scale.begin(13, 14);

  // if (!scale.is_ready()) {
  //     Serial.println("HX711 tidak ditemukan. Periksa pengkabelan.");
  //     while (1); // Hentikan eksekusi
  // }
  Serial.println("Sistem siap. Pengukuran dimulai.");
}

void loop() {
  // if (scale.is_ready()) {
  // 1. Baca nilai mentah dari ADC
  long raw_reading = scale.read();

  // 2. Terapkan filter EMA pada nilai mentah
  float filtered_raw = raw_filter.filter(raw_reading);

  // 3. Konversi nilai mentah yang sudah difilter ke gram menggunakan interpolasi
  float accurate_weight = interpolate(filtered_raw);

  // 4. Tampilkan hasil akhir yang akurat
  Serial.print("Berat Akurat: ");
  Serial.print(accurate_weight, 2);  // Tampilkan dengan 2 angka desimal
  Serial.println(" g");
  // } else {
  //     Serial.println("HX711 tidak merespons.");
  // }
  delay(20);
}
