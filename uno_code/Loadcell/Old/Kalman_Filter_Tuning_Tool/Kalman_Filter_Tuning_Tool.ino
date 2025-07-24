#include "HX711.h"

HX711 scale;

// --- Kelas Kalman Filter Sederhana (1D) ---
class KalmanFilter {
private:
  float Q;  // Process noise covariance
  float R;  // Measurement noise covariance
  float x;  // Estimated value
  float p;  // Estimation error covariance
  float k;  // Kalman gain

public:
  KalmanFilter(float process_noise, float measurement_noise, float initial_value = 0) {
    Q = process_noise;
    R = measurement_noise;
    x = initial_value;
    p = 1.0;
  }

  float filter(float measurement) {
    p = p + Q;
    k = p / (p + R);
    x = x + k * (measurement - x);
    p = (1 - k) * p;
    return x;
  }
};

// --- Variabel Global untuk Parameter Filter ---
float current_q = 0.1;
float current_r = 15.0;

// Inisialisasi objek filter
KalmanFilter raw_filter(current_q, current_r);

void printInstructions() {
  Serial.println("\n--- Alat Tuning Kalman Filter Interaktif ---");
  Serial.println("Tujuan: Menemukan nilai Q dan R yang paling stabil untuk sensor Anda.");
  Serial.println("Perintah (kirim lalu Enter):");
  Serial.println("  Q=[nilai]  -> Contoh: Q=0.01");
  Serial.println("  R=[nilai]  -> Contoh: R=25.5");
  Serial.println("-------------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);

  printInstructions();
  Serial.println("Sistem siap. Biarkan timbangan dalam kondisi kosong dan amati.");
}

void loop() {
  // --- Handle User Input ---
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();  // Hapus spasi atau karakter tak terlihat

    if (command.startsWith("Q=")) {
      String q_value_str = command.substring(2);  // Ambil string setelah "Q="
      current_q = q_value_str.toFloat();
      raw_filter = KalmanFilter(current_q, current_r, raw_filter.filter(scale.read()));  // Re-create filter
      Serial.print("\n>>> Parameter Q diubah menjadi: ");
      Serial.println(current_q, 6);

    } else if (command.startsWith("R=")) {
      String r_value_str = command.substring(2);  // Ambil string setelah "R="
      current_r = r_value_str.toFloat();
      raw_filter = KalmanFilter(current_q, current_r, raw_filter.filter(scale.read()));  // Re-create filter
      Serial.print("\n>>> Parameter R diubah menjadi: ");
      Serial.println(current_r, 6);
    }
  }

  // --- Lakukan Pengukuran dan Pemfilteran ---
  long raw_reading = scale.read();
  float filtered_raw = raw_filter.filter(raw_reading);

  // --- Tampilkan Informasi Diagnostik ---
  Serial.print("Q: ");
  Serial.print(current_q, 6);
  Serial.print(" | R: ");
  Serial.print(current_r, 4);
  Serial.print(" | Mentah Asli: ");
  Serial.print(raw_reading);
  Serial.print(" | Mentah Terfilter: ");
  Serial.println(filtered_raw, 0);
  delay(20);
}
