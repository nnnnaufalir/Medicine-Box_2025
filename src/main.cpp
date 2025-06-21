#include <Arduino.h>
#include "HardwareConfig.h"          // Untuk pin HX711 dan baud rate
#include "AppConfig.h"               // Untuk EMA_FILTER_ALPHA
// #include "TFT_Display.h"             // <--- KOMENTARI BARIS INI DULU
#include "HX711_Module.h" // Modul HX711 kita
#include "Utils.h"             // Modul Utils untuk EMAFilter

// --- Global Instances ---
// TFT_Display myTFT; // <--- KOMENTARI BARIS INI DULU
HX711_Module myLoadCell;
EMAFilter weightFilter(EMA_FILTER_ALPHA); // Gunakan alpha dari AppConfig.h

// --- FreeRTOS Task Declarations ---
void taskLoadCellKNN(void *pvParameters); // Akan kita gunakan untuk membaca load cell
// void taskUpdateDisplay(void *pvParameters); // <--- KOMENTARI BARIS INI DULU

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("Starting Kotak Obat Pintar - HX711 & EMA Test (Serial Only)...");

  // Inisialisasi Modul TFT Display (DIKOMENTARI DULU UNTUK TEST INI)
  // myTFT.begin();
  // myTFT.clearScreen(ILI9341_BLACK);
  // myTFT.printText("Loading...", 10, 10, ILI9341_WHITE, 2);

  // Inisialisasi Modul HX711
  // Pin DT dan SCK diambil dari HardwareConfig.h
  myLoadCell.begin(HX711_DT_PIN, HX711_SCK_PIN);
  delay(1000); // Beri waktu sensor untuk stabil
  myLoadCell.tare(); // Lakukan tare awal

  // Set faktor kalibrasi awal. Ini HARUS disesuaikan setelah kalibrasi!
  // Anda bisa mulai dengan 1.0f dulu, nanti angkanya tidak realistis tapi bisa lihat perubahan.
  // Atau masukkan nilai perkiraan Anda dari kalibrasi sebelumnya.
  myLoadCell.setCalibrationFactor(400.0f); // <<-- SESUAIKAN DENGAN PERKIRAAN FAKTOR KALIBRASI ANDA!

  // Reset EMA filter dengan nilai 0 setelah tare
  weightFilter.reset(0.0f);

  // --- Buat FreeRTOS Task ---
  xTaskCreatePinnedToCore(
    taskLoadCellKNN,
    "LoadCellKNN",
    4096, // Ukuran stack
    NULL,
    5,    // Prioritas
    NULL,
    0     // Core 0
  );

  // Task Display DIKOMENTARI DULU
  // xTaskCreatePinnedToCore(
  //   taskUpdateDisplay,
  //   "DisplayTask",
  //   4096,
  //   NULL,
  //   3,
  //   NULL,
  //   1
  // );

  Serial.println("Setup Complete. Load Cell Task is running.");
}

void loop() {
  delay(10); // Biarkan RTOS scheduler bekerja
}

// --- Implementasi FreeRTOS Task ---

void taskLoadCellKNN(void *pvParameters) {
  for (;;) {
    float currentWeight = myLoadCell.getCalibratedWeight();
    float filteredWeight = weightFilter.filter(currentWeight);

    Serial.printf("Raw Weight: %.2f g, Filtered Weight (EMA): %.2f g\n", currentWeight, filteredWeight);

    // Memberikan waktu untuk task ini dan membiarkan task lain berjalan
    vTaskDelay(pdMS_TO_TICKS(LOADCELL_READ_INTERVAL_MS)); // Interval dari AppConfig.h
  }
}

// Task Update Display DIKOMENTARI DULU
// void taskUpdateDisplay(void *pvParameters) {
//   // ... (kode display yang dikomentari)
// }