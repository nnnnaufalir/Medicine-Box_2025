#include <Arduino.h>
#include "HardwareConfig.h" // Untuk pin HX711 dan baud rate
#include "AppConfig.h"      // Untuk EMA_FILTER_ALPHA, KNN_K_VALUE, LOADCELL_READ_INTERVAL_MS
// #include "TFT_Display/TFT_Display.h" // Modul TFT Display (DIKOMENTARI UNTUK TEST INI)
#include "HX711_Module.h"  // Modul HX711 kita
#include "Utils.h"         // Modul Utils untuk EMAFilter
#include "KNN_Processor.h" // Modul KNN kita

// --- Global Instances ---
// TFT_Display myTFT; // DIKOMENTARI
HX711_Module myLoadCell;
EMAFilter weightFilter(EMA_FILTER_ALPHA); // Gunakan alpha dari AppConfig.h
KNN_Processor myKNN(KNN_K_VALUE);         // Gunakan K_VALUE dari AppConfig.h

// --- FreeRTOS Task Declarations ---
void taskLoadCellAndKNN(void *pvParameters); // Task gabungan untuk membaca dan mengklasifikasi
// void taskUpdateDisplay(void *pvParameters); // DIKOMENTARI

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("Starting Kotak Obat Pintar - HX711, EMA, & KNN Full Test...");

  // Inisialisasi Modul TFT Display (DIKOMENTARI DULU UNTUK TEST INI)
  // myTFT.begin();
  // myTFT.clearScreen(ILI9341_BLACK);
  // myTFT.printText("Loading...", 10, 10, ILI9341_WHITE, 2);

  // Inisialisasi Modul HX711
  // Pin DT dan SCK diambil dari HardwareConfig.h
  myLoadCell.begin(HX711_DT_PIN, HX711_SCK_PIN);
  delay(1000);       // Beri waktu sensor untuk stabil
  myLoadCell.tare(); // Lakukan tare awal

  // Set faktor kalibrasi awal. Ini HARUS disesuaikan setelah kalibrasi!
  // Gunakan nilai yang paling akurat dari kalibrasi Anda.
  myLoadCell.setCalibrationFactor(WEIGHT_CALIBRATION); // <<-- SESUAIKAN DENGAN FAKTOR KALIBRASI ANDA!

  // Reset EMA filter dengan nilai 0 setelah tare
  weightFilter.reset(EMA_FILTER_RESET);

  // Modul KNN sudah diinisialisasi otomatis saat 'myKNN' dibuat,
  // dan dataset default juga sudah dimuat di konstruktornya.

  // --- Buat FreeRTOS Task ---
  xTaskCreatePinnedToCore(
      taskLoadCellAndKNN,
      "LoadCellKNNTask",
      4096, // Ukuran stack (mungkin perlu lebih jika dataset KNN besar)
      NULL,
      5, // Prioritas (lebih tinggi karena ini inti)
      NULL,
      0 // Core 0
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

  Serial.println("Setup Complete. Load Cell & KNN Task is running.");
}

void loop()
{
  delay(10); // Biarkan RTOS scheduler bekerja
}

// --- Implementasi FreeRTOS Task ---

void taskLoadCellAndKNN(void *pvParameters)
{
  for (;;)
  {
    float currentWeight = myLoadCell.getCalibratedWeight();
    float filteredWeight = weightFilter.filter(currentWeight);

    // Lakukan klasifikasi KNN
    int predictedPillCount = myKNN.classify(filteredWeight);

    Serial.printf("Berat Terukur: %.2f g | Berat Filtered (EMA): %.2f g | Prediksi Jumlah Obat (KNN): %d\n",
                  currentWeight, filteredWeight, predictedPillCount);

    // Di sini nanti kita akan mengirim 'predictedPillCount' ke task display
    // atau task jadwal melalui FreeRTOS Queue.

    vTaskDelay(pdMS_TO_TICKS(LOADCELL_READ_INTERVAL_MS)); // Interval dari AppConfig.h
  }
}

// Implementasi taskUpdateDisplay dan fungsi terkait TFT DIKOMENTARI
// void taskUpdateDisplay(void *pvParameters) { /* ... */ }