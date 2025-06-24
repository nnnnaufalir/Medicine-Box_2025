#include <Arduino.h>
#include "HardwareConfig.h" // Untuk pin dan baud rate
#include "AppConfig.h"      // Untuk konstanta aplikasi
#include "TFT_Display.h"    // Modul TFT Display kita
// #include "HX711_Module/HX711_Module.h" // Modul HX711 kita (DIKOMENTARI)
// #include "Utils/Utils.h"             // Modul Utils untuk EMAFilter (DIKOMENTARI)
// #include "KNN_Processor/KNN_Processor.h" // Modul KNN kita (DIKOMENTARI)

// --- Global Instances ---
TFT_Display myTFT;
// HX711_Module myLoadCell; // DIKOMENTARI
// EMAFilter weightFilter(EMA_FILTER_ALPHA); // DIKOMENTARI
// KNN_Processor myKNN(KNN_K_VALUE);         // DIKOMENTARI

// --- FreeRTOS Task Declarations ---
// void taskLoadCellAndKNN(void *pvParameters); // DIKOMENTARI
void taskUpdateDisplay(void *pvParameters); // Task display yang akan kita pakai

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("Starting Kotak Obat Pintar - TFT Full Display Test...");

  // Inisialisasi Modul TFT Display
  myTFT.begin();
  delay(100); // Beri waktu display untuk stabil

  // --- Buat FreeRTOS Task ---
  xTaskCreatePinnedToCore(
      taskUpdateDisplay,
      "DisplayTask",
      4096, // Ukuran stack
      NULL,
      3, // Prioritas
      NULL,
      1 // Core 1 (agar tidak bertabrakan dengan serial debug di Core 0 jika ada)
  );

  Serial.println("Setup Complete. Display Task is running.");
}

void loop()
{
  delay(10); // Biarkan RTOS scheduler bekerja
}

// --- Implementasi FreeRTOS Task ---

void taskUpdateDisplay(void *pvParameters)
{
  int currentPage = 0;
  int dummyObatCount = 5; // Untuk simulasi jumlah obat
  for (;;)
  {
    switch (currentPage)
    {
    case 0:
      myTFT.drawHomeScreen(dummyObatCount);
      break;
    case 1:
      myTFT.drawWarningScreen(dummyObatCount - 1); // Contoh kurangi satu untuk peringatan
      break;
    case 2:
      myTFT.drawInformationScreen();
      break;
    case 3:
      myTFT.drawSettingsScreen();
      break;
    default:
      currentPage = 0; // Kembali ke home
      myTFT.drawHomeScreen(dummyObatCount);
      break;
    }

    currentPage = (currentPage + 1) % 4;        // Ganti halaman setiap kali (0, 1, 2, 3 -> 0, 1, 2, 3...)
    dummyObatCount = (dummyObatCount % 10) + 1; // Ubah jumlah obat dummy

    Serial.printf("Display Task: Drawing Page %d with %d dummy pills\n", currentPage, dummyObatCount);
    vTaskDelay(pdMS_TO_TICKS(3000)); // Ganti halaman setiap 3 detik
  }
}