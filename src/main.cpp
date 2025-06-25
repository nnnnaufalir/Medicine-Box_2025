#include <Arduino.h>
#include "HardwareConfig.h" // Untuk pin HX711 dan baud rate
#include "AppConfig.h"      // Untuk EMA_FILTER_ALPHA, KNN_K_VALUE, LOADCELL_READ_INTERVAL_MS
#include "HX711_Module.h"   // Modul HX711 kita
#include "Utils.h"          // Modul Utils untuk EMAFilter
#include "KNN_Processor.h"  // Modul KNN kita

// --- Global Instances ---
HX711_Module myLoadCell;
EMAFilter weightFilter(EMA_FILTER_ALPHA); // Gunakan alpha dari AppConfig.h
KNN_Processor myKNN(KNN_K_VALUE);         // Gunakan K_VALUE dari AppConfig.h

static unsigned long lastStableZeroTime = 0; // Waktu terakhir load cell stabil di kondisi nol
static bool isLoadCellEmpty = true;          // Status apakah load cell saat ini kosong

// --- FreeRTOS Task Declarations ---
void taskLoadCellAndKNN(void *pvParameters); // Task gabungan untuk membaca dan mengklasifikasi

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("Starting Kotak Obat Pintar - HX711, EMA, & KNN Full Test...");

  // Inisialisasi Modul HX711
  // Pin DT dan SCK diambil dari HardwareConfig.h
  myLoadCell.begin(HX711_DT_PIN, HX711_SCK_PIN);
  myLoadCell.setCalibrationFactor(WEIGHT_CALIBRATION); // <<-- SESUAIKAN DENGAN FAKTOR KALIBRASI ANDA!
  myLoadCell.tare();                                   // Lakukan tare awal

  // Reset EMA filter dengan nilai 0 setelah tare
  weightFilter.reset(EMA_FILTER_RESET_VALUE);

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

  Serial.println("Setup Complete. Load Cell & KNN Task is running.");
}

void loop()
{
  delay(1); // Biarkan RTOS scheduler bekerja
}

// --- Implementasi FreeRTOS Task ---

void taskLoadCellAndKNN(void *pvParameters)
{
  for (;;)
  {
    float currentWeight = myLoadCell.getCalibratedWeight(WEIGHT_SAMPLING);
    float filteredWeight = weightFilter.filter(currentWeight);

    // --- Logika Auto-Tare ---
    // Cek apakah berat berada dalam ambang batas "nol"
    if (filteredWeight >= -WEIGHT_TOLERANCE_G && filteredWeight <= WEIGHT_TOLERANCE_G)
    {
      if (!isLoadCellEmpty)
      {                                // Baru masuk kondisi kosong
        lastStableZeroTime = millis(); // Catat waktu
        isLoadCellEmpty = true;
        Serial.println("Load Cell: Masuk kondisi dianggap KOSONG.");
      }
      else
      { // Sudah dalam kondisi kosong, cek durasi
        if (millis() - lastStableZeroTime >= AUTO_TARE_DELAY_MS)
        {
          myLoadCell.tare();
          // Reset filter EMA setelah tare untuk memastikan nilai awal yang bersih
          weightFilter.reset(EMA_FILTER_RESET_VALUE);
          Serial.println("Load Cell: Auto-Tare berhasil dilakukan!");
          lastStableZeroTime = millis(); // Reset waktu setelah tare
        }
        else
        {
          Serial.println("Load Cell: Sensor tidak siap untuk Auto-Tare.");
        }
      }
    }
    else
    { // Ada beban terdeteksi
      isLoadCellEmpty = false;
    }

    // Lakukan klasifikasi KNN
    int predictedPillCount = myKNN.classify(filteredWeight);

    Serial.printf("Raw: %.2f g | Filtered: %.2f g | Pills: %d | Empty: %s\n",
                  currentWeight, filteredWeight, predictedPillCount, isLoadCellEmpty ? "YES" : "NO");

    // Kirim 'predictedPillCount' ke task display (akan diimplementasikan nanti dengan Queue)

    vTaskDelay(pdMS_TO_TICKS(LOADCELL_READ_INTERVAL_MS));
  }
}