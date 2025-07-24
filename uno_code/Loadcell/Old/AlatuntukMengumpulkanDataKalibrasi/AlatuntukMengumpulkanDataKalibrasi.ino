#include "HX711.h"

HX711 scale;

// --- Konfigurasi Kalibrasi dengan Obat ---
const float BERAT_SATU_OBAT = 1.55f;
const int JUMLAH_TITIK_KALIBRASI = 8;

// Jumlah obat di setiap langkah kalibrasi
int jumlah_obat_kalibrasi[JUMLAH_TITIK_KALIBRASI] = { 0, 1, 2, 5, 10, 15, 20, 25 };

// Array untuk menyimpan berat target (akan dihitung otomatis)
float berat_kalibrasi[JUMLAH_TITIK_KALIBRASI];

// Array untuk menyimpan hasil pembacaan nilai mentah
long nilai_mentah_kalibrasi[JUMLAH_TITIK_KALIBRASI];

void run_calibration_routine() {
  Serial.println("\n--- Memulai Rutinitas Kalibrasi dengan Obat ---");
  Serial.println("Gunakan obat dengan berat asumsi 1.55g per buah.");
  Serial.println("-------------------------------------------------");

  for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
    // Hitung berat target untuk langkah ini
    berat_kalibrasi[i] = jumlah_obat_kalibrasi[i] * BERAT_SATU_OBAT;

    Serial.print("Langkah ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(JUMLAH_TITIK_KALIBRASI);
    Serial.print(": Letakkan total ");
    Serial.print(jumlah_obat_kalibrasi[i]);
    Serial.print(" buah obat (target: ");
    Serial.print(berat_kalibrasi[i], 2);
    Serial.println("g).");

    Serial.println("--> Ketik 'ok' dan tekan Enter jika sudah siap.");

    while (Serial.available() == 0) { delay(100); }
    while (Serial.available() > 0) { Serial.read(); }

    Serial.println("    Mengambil pembacaan stabil...");
    nilai_mentah_kalibrasi[i] = scale.read_average(99);

    Serial.print("    Nilai mentah tercatat: ");
    Serial.println(nilai_mentah_kalibrasi[i]);
    Serial.println("-------------------------------------------------");
  }

  Serial.println("\n--- KALIBRASI SELESAI ---");
  Serial.println("Salin dan simpan tabel kalibrasi baru ini!");
  Serial.println("Berat (g)\t->\tNilai Mentah");
  for (int i = 0; i < JUMLAH_TITIK_KALIBRASI; i++) {
    Serial.print(berat_kalibrasi[i], 2);
    Serial.print("g\t\t->\t");
    Serial.println(nilai_mentah_kalibrasi[i]);
  }
}

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);

  run_calibration_routine();
}

void loop() {
  // Loop sengaja dikosongkan. Alat ini hanya berjalan sekali.
}
