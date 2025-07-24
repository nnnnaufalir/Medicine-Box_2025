#include "HX711.h"

HX711 scale;

void setup() {
  Serial.begin(115200);
  Serial.println("--- Lapis 1: Uji Stabilitas Nilai Mentah ---");
  Serial.println("Tujuan: Mengamati kestabilan data mentah dari sensor.");

  scale.begin(13, 14);

  // Kita tidak melakukan tare atau set_scale.
  // Kita ingin melihat data paling murni.
  Serial.println("Sistem siap. Biarkan timbangan kosong dan amati nilainya.");
  Serial.println("----------------------------------------------------");
}

void loop() {
  // Ambil rata-rata dari 20 pembacaan untuk mendapatkan nilai yang cukup stabil
  long raw_value = scale.read_average(5);

  Serial.print("Nilai Mentah Stabil: ");
  Serial.println(raw_value);
  delay(1);  // Tampilkan data setiap setengah detik
}
