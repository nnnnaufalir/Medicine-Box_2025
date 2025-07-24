#include <HX711.h>
#include "AppConfig.h"

// --- State Machine Definitions ---
enum SystemState {
  STATE_IDLE,
  STATE_ARMED
};

// --- Tunable Parameters ---
const float WEIGHT_CHANGE_THRESHOLD_G = 2.0;
const unsigned long COOLDOWN_PERIOD_MS = 3000; // 3 seconds to ignore readings after a take.

// --- State Variables ---
SystemState currentState = STATE_IDLE;
float baseWeight = 0.0;
float peakWeight = 0.0;
unsigned long cooldownUntil = 0;
bool recalibrationNeeded = false;

HX711 scale;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("--- Cooldown Method Test (v2 with Net Weight) ---");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(LOADCELL_CALIBRATION_FACTOR);
  
  scale.tare(20);
  baseWeight = scale.get_units(10);
  Serial.print("Initial Base Weight set to: "); Serial.println(baseWeight, 2);
}

void loop() {
  if (recalibrationNeeded && millis() > cooldownUntil) {
    Serial.println(">> Cooldown Selesai. Melakukan re-kalibrasi referensi dasar.");
    baseWeight = scale.get_units(10);
    Serial.print("   Referensi Dasar Baru: "); Serial.println(baseWeight, 2);
    recalibrationNeeded = false;
  }

  if (millis() < cooldownUntil) {
    return;
  }

  if (!scale.is_ready()) return;
  float currentWeight = scale.get_units(10);

  // --- [KODE BARU] Menghitung Berat Bersih ---
  float netWeight = currentWeight - baseWeight;

  switch (currentState) {
    case STATE_IDLE:
      if (currentWeight > baseWeight + WEIGHT_CHANGE_THRESHOLD_G) {
        Serial.println("\n>> Kenaikan terdeteksi. Masuk Mode SIAGA.");
        currentState = STATE_ARMED;
        peakWeight = currentWeight;
      }
      break;

    case STATE_ARMED:
      if (currentWeight > peakWeight) {
        peakWeight = currentWeight;
      }

      if (currentWeight < peakWeight - WEIGHT_CHANGE_THRESHOLD_G) {
        float weightTaken = peakWeight - currentWeight;
        Serial.println("\n>> Pengambilan terdeteksi!");
        Serial.print("   Berat Diambil (Kalkulasi Awal): "); Serial.println(weightTaken, 2);
        
        Serial.println("   Memulai Cooldown 3 detik...");
        cooldownUntil = millis() + COOLDOWN_PERIOD_MS;
        recalibrationNeeded = true;
        
        currentState = STATE_IDLE;
      }
      break;
  }

  // --- [KODE DIPERBARUI] Menampilkan Berat Bersih di Serial Monitor ---
  Serial.print("Status: ");
  Serial.print((currentState == STATE_IDLE) ? "MENUNGGU" : "SIAGA   ");
  Serial.print(" | Dasar: "); Serial.print(baseWeight, 2);
  Serial.print(" | Saat Ini: "); Serial.print(currentWeight, 2);
  Serial.print(" | Bersih: "); Serial.println(netWeight, 2); // Ditambahkan di sini

  delay(200);
}
