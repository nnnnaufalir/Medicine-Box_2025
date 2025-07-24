#include "HX711.h"

HX711 scale;

// --- Konfigurasi Kalibrasi Kumulatif ---
const int JUMLAH_TITIK_KALIBRASI = 6; 

// Kita buat sendiri titik berat kita berdasarkan apa yang kita punya.
// 0g, 1 obat, 2 obat, 3 obat, 4 obat, lalu 1 baterai.
float berat_kalibrasi[JUMLAH_TITIK_KALIBRASI] = {0.0, 1.6, 3.2, 4.8, 6.4, 50.0};

// Array untuk menyimpan hasil pembacaan nilai mentah.
long nilai_mentah_kalibrasi[JUMLAH_TITIK_KALIBRASI];

void run_calibration_routine() {
  Serial.println("\n--- Memulai Rutinitas Kalibrasi Kumulatif ---");
  Serial.println("Kita akan membuat titik kalibrasi dengan menumpuk beban.");
  Serial.println("-------------------------------------------------");

  for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
    Serial.print("Langkah ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(JUMLAH_TITIK_KALIBRASI);
    Serial.print(": Siapkan beban untuk target ");
    Serial.print(berat_kalibrasi[i], 1);
    Serial.println("g.");
    
    // Memberikan instruksi yang lebih jelas
    if (i == 0) {
      Serial.println("   -> Pastikan timbangan KOSONG.");
    } else if (i < 5) {
      Serial.print("   -> Letakkan total "); Serial.print(i); Serial.println(" buah obat di timbangan.");
    } else {
      Serial.println("   -> SINGKIRKAN semua obat, lalu letakkan BATERAI di timbangan.");
    }

    Serial.println("--> Ketik 'ok' dan tekan Enter jika sudah siap.");

    while (Serial.available() == 0) { delay(100); }
    while(Serial.available() > 0) { Serial.read(); }
    
    Serial.println("    Mengambil pembacaan stabil...");
    nilai_mentah_kalibrasi[i] = scale.read_average(30);
    
    Serial.print("    Nilai mentah tercatat: ");
    Serial.println(nilai_mentah_kalibrasi[i]);
    Serial.println("-------------------------------------------------");
  }

  Serial.println("\n--- KALIBRASI SELESAI ---");
  Serial.println("Salin dan simpan tabel kalibrasi ini!");
  Serial.println("Berat (g)\t->\tNilai Mentah");
  for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
    Serial.print(berat_kalibrasi[i], 1);
    Serial.print("g\t\t->\t");
    Serial.println(nilai_mentah_kalibrasi[i]);
  }
  Serial.println("\nSilakan reset perangkat untuk memulai pengukuran.");
}

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);

  run_calibration_routine();

  // if (scale.is_ready()) {
  //   run_calibration_routine();
  // } else {
  //   Serial.println("HX711 tidak ditemukan. Periksa pengkabelan.");
  //   run_calibration_routine();
  //   while (1);
  // }
}

void loop() {
  // Loop sengaja dikosongkan.
}
