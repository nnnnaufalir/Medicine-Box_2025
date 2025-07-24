// 1. Include semua library yang dibutuhkan
#include "HX711.h"           // Library asli untuk membaca sensor
#include "KalmanFilter.h"    // Library filter kustom kita
#include "ProcessedScale.h"  // Library kalibrasi kustom kita


// 2. Buat objek untuk setiap komponen
HX711 hardware_scale;
KalmanFilter kalman_filter(0.1, 15.0);  // Gunakan parameter Q & R terbaik Anda
ProcessedScale logic_scale;

long bulatkanKeRatusan(long angka) {
  // 1. Bagi dengan 100.0 untuk mendapatkan nilai desimal
  float nilaiSementara = angka / 100.0;

  // 2. Gunakan round() untuk membulatkan ke bilangan bulat terdekat
  long hasilBulat = round(nilaiSementara);

  // 3. Kembalikan ke skala ratusan
  return hasilBulat * 100;
}

// Fungsi helper untuk melakukan tare manual
void performManualTare() {
  Serial.println("\n>>> Melakukan Tare Manual...");
  float sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += bulatkanKeRatusan(kalman_filter.update(hardware_scale.read()));
    delay(20);
  }
  logic_scale.tare(round(sum / 20.0));
  Serial.print(">>> Tare selesai. Offset baru: ");
  Serial.println(logic_scale.getOffset());
}

void printInstructions() {
  Serial.println("\n--- Alat Kalibrasi Sistem Kustom ---");
  Serial.println("Tujuan: Menemukan Faktor Skala yang tepat untuk logika kustom kita.");
  Serial.println("Perintah (kirim lalu Enter):");
  Serial.println("  t = Tare (Mengatur Offset ke nilai terfilter saat ini)");
  Serial.println("  a/s/d/f/g/h/j = Tambah Faktor Skala");
  Serial.println("  z/x/c/v/b/n/m = Kurangi Faktor Skala");
  Serial.println("-----------------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  hardware_scale.begin(13, 14);

  printInstructions();
  performManualTare();           // Lakukan tare awal saat startup
  logic_scale.setScale(1000.0);  // Atur faktor skala awal yang masuk akal
}

void loop() {
  // Handle user input untuk penyesuaian
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    float current_scale = logic_scale.getScale();
    switch (cmd) {
      case 't': performManualTare(); break;
      case 'a': logic_scale.setScale(current_scale + 1000); break;
      case 's': logic_scale.setScale(current_scale + 100); break;
      case 'd': logic_scale.setScale(current_scale + 10); break;
      case 'f': logic_scale.setScale(current_scale + 1); break;
      case 'g': logic_scale.setScale(current_scale + 0.1f); break;
      case 'h': logic_scale.setScale(current_scale + 0.01f); break;
      case 'j': logic_scale.setScale(current_scale + 0.001f); break;
      case 'z': logic_scale.setScale(current_scale - 1000); break;
      case 'x': logic_scale.setScale(current_scale - 100); break;
      case 'c': logic_scale.setScale(current_scale - 10); break;
      case 'v': logic_scale.setScale(current_scale - 1); break;
      case 'b': logic_scale.setScale(current_scale - 0.1f); break;
      case 'n': logic_scale.setScale(current_scale - 0.01f); break;
      case 'm': logic_scale.setScale(current_scale - 0.001f); break;
    }
  }

  // Alur Kerja Pengukuran
  long raw_value = hardware_scale.read();
  float filtered_value = bulatkanKeRatusan(kalman_filter.update(raw_value));
  float final_weight = logic_scale.getWeight(filtered_value);

  // Tampilkan Informasi Diagnostik
  Serial.print("Terfilter: ");
  Serial.print(long(filtered_value));
  Serial.print(" | Offset: ");
  Serial.print(logic_scale.getOffset());
  Serial.print(" | Faktor Skala: ");
  Serial.print(logic_scale.getScale());
  Serial.print(" | Berat (g): ");
  Serial.println(final_weight);
  delay(20);
}
