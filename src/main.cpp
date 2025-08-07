#include <Arduino.h>

// --- Impor semua header modul kustom ---
#include "AppConfig.h"
#include "Display.h"
#include "Jaringan.h"
#include "Logic.h"
#include "Penyimpanan.h"
#include "Sensor.h"
#include "Waktu.h"

#include <RTClib.h>

// --- Definisi Variabel Global ---
// Variabel-variabel ini dideklarasikan sebagai 'extern' di modul lain
// dan didefinisikan secara konkret di sini sebagai pusat data sistem.
volatile int g_pill_count = 0;
volatile bool g_isSystemStable = true;
volatile bool g_isFrozen = false;
volatile float g_battery_voltage = 0.0f;
volatile bool g_alarm_triggered = false;
volatile int g_dose_to_take = 0;

// --- Sinkronisasi RTOS ---
// Mutex (Mutual Exclusion) untuk melindungi akses ke variabel global
// dari race condition antar task.
SemaphoreHandle_t g_dataMutex;

// --- Task Handles (Opsional, untuk debugging/kontrol lanjutan) ---
TaskHandle_t sensorTaskHandle;
TaskHandle_t displayTaskHandle;
TaskHandle_t logicTaskHandle;
TaskHandle_t networkTaskHandle;

// --- Deklarasi Fungsi Task ---
// Ini adalah fungsi utama untuk setiap "program mini" kita.
void sensorTask(void *pvParameters);
void displayTask(void *pvParameters);
void logicTask(void *pvParameters);
void networkTask(void *pvParameters);

// =================================================================
// SETUP - Inisialisasi Sistem
// =================================================================
void setup()
{
  // 1. Inisialisasi dasar
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n--- Booting Smart Pill Box System ---");

  // 2. Inisialisasi semua modul perangkat keras dan logika
  // Memanggil Wire.begin() sekali di sini untuk I2C (RTC & EEPROM)
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  initPenyimpanan();
  initWaktu();
  initDisplay();  // Menampilkan boot screen
  initSensor();   // Melakukan kalibrasi drift awal
  initJaringan(); // Memulai WiFi AP dan Web Server

  Serial.println("--- Inisialisasi Modul Selesai ---");

  // 3. Membuat Mutex untuk sinkronisasi data
  g_dataMutex = xSemaphoreCreateMutex();
  if (g_dataMutex == NULL)
  {
    Serial.println("Kesalahan: Gagal membuat Mutex!");
    // Sistem tidak boleh lanjut jika mutex gagal dibuat
    while (1)
      ;
  }

  // 4. Membuat semua task FreeRTOS dengan afinitas Core
  Serial.println("--- Membuat Tasks (dengan Core Affinity) ---");

  // Core 1 (Application Core): Didedikasikan untuk tugas paling kritis
  // Task Sensor: Prioritas tertinggi, komputasi intensif. Dijalankan di Core 1
  // untuk performa maksimal dan jitter minimal.
  xTaskCreatePinnedToCore(
      sensorTask,
      "SensorTask",
      4096, // Ukuran stack besar untuk komputasi sensor
      NULL,
      3, // Prioritas: Tinggi
      &sensorTaskHandle,
      1); // <-- Pin ke Core 1

  // Core 0 (Protocol Core): Untuk tugas jaringan dan I/O lainnya
  // Task Network: Prioritas rendah. Dijalankan di Core 0 bersama tumpukan WiFi.
  xTaskCreatePinnedToCore(
      networkTask,
      "NetworkTask",
      4096, // Stack besar untuk tumpukan TCP/IP & Web Server
      NULL,
      1, // Prioritas: Rendah
      &networkTaskHandle,
      0); // <-- Pin ke Core 0

  // Task Display: Prioritas sedang.
  xTaskCreatePinnedToCore(
      displayTask,
      "DisplayTask",
      2048, // Ukuran stack standar
      NULL,
      2, // Prioritas: Sedang
      &displayTaskHandle,
      0); // <-- Pin ke Core 0

  // Task Logic: Prioritas sedang, frekuensi rendah.
  xTaskCreatePinnedToCore(
      logicTask,
      "LogicTask",
      2048, // Ukuran stack standar
      NULL,
      2, // Prioritas: Sedang
      &logicTaskHandle,
      0); // <-- Pin ke Core 0

  Serial.println("--- Sistem Siap. Scheduler RTOS dimulai. ---");
  // Scheduler RTOS akan mengambil alih dari sini.
}

// =================================================================
// LOOP - Tidak Digunakan
// =================================================================
void loop()
{
  // Dibiarkan kosong. Semua pekerjaan dilakukan oleh task FreeRTOS.
  // Menghapus fungsi ini akan menghemat sedikit memori.
}

// =================================================================
// IMPLEMENTASI TASK
// =================================================================

/**
 * @brief Task untuk membaca sensor, memfilter, dan mengklasifikasi data.
 * Frekuensi: ~100 ms
 */
void sensorTask(void *pvParameters)
{
  for (;;) // Loop tak terbatas
  {
    // Ambil kunci mutex sebelum mengakses data global
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
    {
      loopSensor(); // Jalankan logika sensor
      // Lepas kunci mutex setelah selesai
      xSemaphoreGive(g_dataMutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Tunggu ~100 ms
  }
}

/**
 * @brief Task untuk memperbarui tampilan di layar TFT.
 * Frekuensi: ~250 ms
 */
void displayTask(void *pvParameters)
{
  for (;;)
  {
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
    {
      loopDisplay(); // Jalankan logika display
      xSemaphoreGive(g_dataMutex);
    }
    vTaskDelay(250 / portTICK_PERIOD_MS); // Tunggu ~250 ms
  }
}

/**
 * @brief Task untuk menjalankan logika utama (alarm, baterai).
 * Frekuensi: ~1 sekon
 */
void logicTask(void *pvParameters)
{
  for (;;)
  {
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
    {
      loopLogic(); // Jalankan logika aplikasi
      xSemaphoreGive(g_dataMutex);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Tunggu 1 detik
  }
}

/**
 * @brief Task untuk menjaga agar proses web server tetap berjalan.
 * Server bersifat asinkron, jadi task ini hanya perlu "tidur".
 */
void networkTask(void *pvParameters)
{
  for (;;)
  {
    // Tidak ada pekerjaan yang perlu dilakukan di sini secara aktif.
    // Library ESPAsyncWebServer menangani koneksi di latar belakang.
    // Task ini hanya perlu ada dan menunda dirinya sendiri.
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
