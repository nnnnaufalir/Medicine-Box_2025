#include <Arduino.h>
#include "HardwareConfig.h"
#include "AppConfig.h"

// Sertakan semua modul yang kita butuhkan
#include "TFT_Display.h"
#include "RTC_Module.h"
#include "HX711_Module.h"
#include "KNN_Processor.h"

// --- Deklarasi Global Instance Modul ---
TFT_Display tft;
RTC_Module rtc;
HX711_Module hx711(PIN_HX711_DOUT, PIN_HX711_SCK);
KNN_Processor knn(KNN_K);

// --- Konfigurasi Anda ---
const float YOUR_CALIBRATION_FACTOR = 4119.755f;
const char *PATIENT_NAME = "Bapak Budi";

// --- Deklarasi Tugas & Queue FreeRTOS ---
TaskHandle_t h_taskSensorProcessor;
TaskHandle_t h_taskUIManager;
QueueHandle_t q_displayData;

// Struktur data untuk komunikasi antar tugas
struct DisplayData
{
  int pillCount;
  char formattedDate[30]; // Menggunakan char array lebih aman untuk RTOS
};

// Dataset KNN Anda
const KNNDataPoint myDataset[] = {
    {{0.2f}, 0}, {{0.5f}, 0}, {{0.8f}, 0}, {{1.1f}, 0}, {{1.3f}, 0}, {{1.6f}, 1}, {{1.55f}, 1}, {{1.65f}, 1}, {{1.7f}, 1}, {{1.5f}, 1}, {{3.15f}, 2}, {{3.2f}, 2}, {{3.3f}, 2}, {{3.1f}, 2}, {{3.25f}, 2}, {{4.8f}, 3}, {{4.9f}, 3}, {{4.85f}, 3}, {{4.65f}, 3}, {{4.95f}, 3}, {{6.45f}, 4}, {{6.35f}, 4}, {{6.3f}, 4}, {{6.5f}, 4}, {{6.4f}, 4}, {{8.0f}, 5}, {{7.75f}, 5}, {{8.5f}, 5}, {{7.5f}, 5}, {{8.25f}, 5}, {{9.1f}, 6}, {{9.4f}, 6}, {{9.65f}, 6}, {{9.9f}, 6}, {{10.15f}, 6}, {{10.6f}, 7}, {{10.9f}, 7}, {{11.2f}, 7}, {{11.5f}, 7}, {{11.8f}, 7}, {{12.1f}, 8}, {{12.5f}, 8}, {{12.8f}, 8}, {{13.2f}, 8}, {{13.5f}, 8}, {{13.6f}, 9}, {{14.0f}, 9}, {{14.4f}, 9}, {{14.8f}, 9}, {{15.2f}, 9}, {{15.2f}, 10}, {{15.6f}, 10}, {{16.0f}, 10}, {{16.4f}, 10}, {{16.8f}, 10}, {{16.7f}, 11}, {{17.2f}, 11}, {{17.7f}, 11}, {{18.2f}, 11}, {{18.6f}, 11}, {{18.2f}, 12}, {{18.7f}, 12}, {{19.2f}, 12}, {{19.7f}, 12}, {{20.2f}, 12}, {{19.7f}, 13}, {{20.3f}, 13}, {{20.9f}, 13}, {{21.5f}, 13}, {{22.0f}, 13}, {{21.2f}, 14}, {{21.8f}, 14}, {{22.4f}, 14}, {{23.0f}, 14}, {{23.6f}, 14}, {{22.7f}, 15}, {{23.5f}, 15}, {{24.0f}, 15}, {{24.8f}, 15}, {{25.3f}, 15}};
const int DATASET_SIZE = sizeof(myDataset) / sizeof(myDataset[0]);

// --- Implementasi Tugas FreeRTOS ---

/**
 * @brief Tugas untuk memproses semua sensor (HX711, RTC) dan KNN.
 * Dijalankan di Core 0 sebagai tugas latar belakang.
 */
void taskSensorAndDataProcessor(void *pvParameters)
{
  Serial.println("Tugas Sensor & Data Processor berjalan di Core 0.");
  DisplayData dataToSend;

  const char *days[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "Mei", "Jun", "Jul", "Ags", "Sep", "Okt", "Nov", "Des"};

  for (;;)
  {
    // 1. Dapatkan data
    float currentWeight = hx711.getWeight();
    DateTime now = rtc.now();

    // 2. Proses data
    dataToSend.pillCount = knn.classify(currentWeight);
    sprintf(dataToSend.formattedDate, "%s, %d %s %d", days[now.dayOfTheWeek()], now.day(), months[now.month() - 1], now.year());

    // 3. Kirim data ke tugas UI
    // xQueueOverwrite akan menimpa data lama, cocok untuk UI yang hanya butuh data terbaru.
    xQueueOverwrite(q_displayData, &dataToSend);

    vTaskDelay(pdMS_TO_TICKS(TASK_UI_UPDATE_INTERVAL_MS)); // Jeda sesuai konfigurasi
  }
}

/**
 * @brief Tugas yang HANYA bertanggung jawab untuk memperbarui layar TFT.
 * Dijalankan di Core 1 untuk memastikan UI selalu responsif.
 */
void taskUIManager(void *pvParameters)
{
  Serial.println("Tugas UI Manager berjalan di Core 1.");
  DisplayData receivedData;

  for (;;)
  {
    // Menunggu data baru dari queue tanpa batas waktu.
    // Tugas ini akan "tidur" sampai data tersedia, sangat efisien.
    if (xQueueReceive(q_displayData, &receivedData, portMAX_DELAY) == pdPASS)
    {
      // Data baru diterima, gambar ulang layar.
      tft.drawHomePage(PATIENT_NAME, receivedData.pillCount, receivedData.formattedDate);
    }
  }
}

// --- SETUP & LOOP ---

void setup()
{
  Serial.begin(115200);
  Serial.println("\n--- Sistem Kotak Obat Pintar dengan FreeRTOS ---");

  // Inisialisasi semua modul hardware
  Wire.begin(I2C_SDA, I2C_SCL);
  tft.begin();
  rtc.begin();
  hx711.begin();
  hx711.setCalibrationFactor(YOUR_CALIBRATION_FACTOR);
  hx711.tare();

  // Inisialisasi modul software
  knn.loadDataset(myDataset, DATASET_SIZE);

  // Buat queue untuk komunikasi antar tugas
  q_displayData = xQueueCreate(1, sizeof(DisplayData));
  if (q_displayData == NULL)
  {
    Serial.println("FATAL: Gagal membuat queue!");
    while (1)
      ;
  }

  Serial.println("Memulai tugas-tugas...");

  // Buat tugas pemroses data di Core 0
  xTaskCreatePinnedToCore(
      taskSensorAndDataProcessor,
      "SensorTask",
      4096, // Ukuran stack
      NULL,
      1, // Prioritas
      &h_taskSensorProcessor,
      0);

  // Buat tugas UI di Core 1
  xTaskCreatePinnedToCore(
      taskUIManager,
      "UITask",
      4096, // Ukuran stack
      NULL,
      2, // Prioritas lebih tinggi untuk UI
      &h_taskUIManager,
      1);

  Serial.println("Sistem siap.");
}

void loop()
{
  // Kosong. Semua pekerjaan dilakukan oleh tugas-tugas RTOS.
  vTaskSuspend(NULL);
}
