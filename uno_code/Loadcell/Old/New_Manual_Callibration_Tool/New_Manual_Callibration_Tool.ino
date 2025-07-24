// 1. Include semua library yang dibutuhkan
#include "HX711.h"           // Library asli untuk membaca sensor
#include "KalmanFilter.h"    // Library filter kustom kita
#include "ProcessedScale.h"  // Library kalibrasi kustom kita

// 2. Buat objek untuk setiap komponen
HX711 hardware_scale;
KalmanFilter kalman_filter(0.1, 15.0);  // Gunakan parameter Q & R terbaik Anda
ProcessedScale logic_scale;

void setup() {
  Serial.begin(115200);
  hardware_scale.begin(13, 14);

  // Inisialisasi awal
  Serial.println("Sistem siap. Melakukan Tare awal...");
  long initial_raw_reading = hardware_scale.read_average();
  float initial_filtered_reading = kalman_filter.update(initial_raw_reading);
  logic_scale.tare(initial_filtered_reading);
  logic_scale.setScale(8379.60f); // Atur faktor skala awal
}

void loop() {
  // a. Baca data mentah dari hardware
  long raw_value = hardware_scale.read();

  // b. Stabilkan data dengan Kalman Filter
  float filtered_value = kalman_filter.update(raw_value);

  // c. Hitung berat menggunakan logika kalibrasi kita
  float final_weight = logic_scale.getWeight(filtered_value);

  // Tampilkan hasilnya
  Serial.print(logic_scale.getOffset());
  Serial.print(", ");
  Serial.print(logic_scale.getScale());
  Serial.print(", ");
  Serial.print("Raw:");
  Serial.print(raw_value);
  Serial.print("; ");
  Serial.print("Kalman:");
  Serial.print(filtered_value);
  Serial.print("; ");
  Serial.print("Berat:");
  Serial.print(final_weight);
  Serial.println(" g");
}
