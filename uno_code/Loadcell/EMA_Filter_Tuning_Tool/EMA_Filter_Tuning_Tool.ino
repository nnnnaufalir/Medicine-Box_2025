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

// --- Variabel Global untuk Parameter Filter ---
float current_alpha_filter = 0.9;  // Nilai awal alpha
float current_alpha_weight = 0.9;

EMAFilter raw_filter(current_alpha_filter);
EMAFilter weight_filter(current_alpha_weight);

// Tabel kalibrasi baru Anda
const int JUMLAH_TITIK_KALIBRASI = 8;
float calibration_weights[JUMLAH_TITIK_KALIBRASI] = { 0.0, 1.55, 3.10, 7.75, 15.50, 23.25, 31.00, 38.75 };
long calibration_raw_values[JUMLAH_TITIK_KALIBRASI] = { 297285, 301713, 307905, 324348, 346881, 376932, 407826, 437476 };

// --- Fungsi Interpolasi (tidak berubah) ---
float interpolate(long raw_value) {
  if (raw_value <= calibration_raw_values[0]) {
    long x1 = calibration_raw_values[0];
    float y1 = calibration_weights[0];  // y1 akan 0.0
    long x2 = calibration_raw_values[1];
    float y2 = calibration_weights[1];
    // Lakukan ekstrapolasi linear berdasarkan dua titik pertama
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


void printInstructions() {
  Serial.println("\n--- Alat Tuning EMA Filter Interaktif ---");
  Serial.println("Tujuan: Menemukan nilai Alpha yang paling stabil dan responsif.");
  Serial.println("Perintah (kirim lalu Enter):");
  Serial.println("  A=[nilai]  -> Atur Alpha langsung. Contoh: A=0.05");
  Serial.println("  --- Penyesuaian Bertahap ---");
  Serial.println("  a = Tambah 0.1 | s = Tambah 0.01 | d = Tambah 0.001");
  Serial.println("  z = Kurangi 0.1| x = Kurangi 0.01| c = Kurangi 0.001");
  Serial.println("-------------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);

  // printInstructions();
  // Serial.println("Sistem siap. Biarkan timbangan kosong dan amati.");
}

void loop() {
  // --- Handle User Input ---
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.startsWith("A=")) {
      String alpha_str = command.substring(2);
      current_alpha_filter = alpha_str.toFloat();
      current_alpha_weight = alpha_str.toFloat();
    } else {
      char cmd_char = command.charAt(0);
      switch (cmd_char) {
        case 'a': current_alpha_filter += 0.1; break;
        case 's': current_alpha_filter += 0.01; break;
        case 'd': current_alpha_filter -= 0.1; break;
        case 'f': current_alpha_filter -= 0.01; break;

        case 'z': current_alpha_weight += 0.1; break;
        case 'x': current_alpha_weight += 0.01; break;
        case 'c': current_alpha_weight -= 0.1; break;
        case 'v': current_alpha_weight -= 0.01; break;
      }
    }
    // Batasi nilai alpha antara 0.0 dan 1.0
    if (current_alpha_filter > 1.0) current_alpha_filter = 1.0;
    if (current_alpha_filter < 0.0) current_alpha_filter = 0.0;

    if (current_alpha_weight > 1.0) current_alpha_weight = 1.0;
    if (current_alpha_weight < 0.0) current_alpha_weight = 0.0;

    raw_filter = EMAFilter(current_alpha_filter);  // Buat ulang filter dengan alpha baru
    weight_filter = EMAFilter(current_alpha_weight);
    // Serial.print("\n>>> Parameter Alpha diubah menjadi: "); Serial.println(current_alpha, 4);
  }

  // --- Lakukan Pengukuran dan Pemfilteran ---

  long raw_reading = scale.read();
  float filtered_raw = raw_filter.filter(raw_reading);
  float inter_calibrate = interpolate(filtered_raw);
  float accurate_weight = weight_filter.filter(inter_calibrate);

  // --- Tampilkan Informasi Diagnostik ---
  // Serial.print("Alpha: ");
  // Serial.print(current_alpha, 4);
  Serial.print("Mentah:");
  Serial.print(float(raw_reading), 2);
  Serial.print(",");
  Serial.print("Terfilter:");
  Serial.print(filtered_raw, 2);
  Serial.print(",");
  Serial.print("interpolate:");
  Serial.print(inter_calibrate, 2);
  Serial.print(",");
  Serial.print("weight:");
  Serial.print(accurate_weight, 2);
  Serial.print(",");
  Serial.print("alphaFilter:");
  Serial.print(current_alpha_filter);
  Serial.print(",");
  Serial.print("alphaWeight:");
  Serial.println(current_alpha_weight);

  delay(1);
}
