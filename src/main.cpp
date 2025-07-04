#include <Arduino.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <RTClib.h>

// --- Sertakan semua modul dan konfigurasi kita ---
#include "HardwareConfig.h"
#include "AppConfig.h"
#include "ConfigData.h" // (BARU) Struktur data konfigurasi
#include "TFT_Display.h"
#include "RTC_Module.h"
#include "HX711_Module.h"
#include "KNN_Processor.h"
#include "Schedule_Manager.h"
#include "Utils.h"          // (BARU) Mengandung WiFi_Manager & Buzzer
#include "EEPROM_Manager.h" // (BARU) Pengelola penyimpanan

// --- Deklarasi Instance Global ---
ConfigData config; // (BARU) Variabel global untuk menampung semua konfigurasi
TFT_Display tft;
RTC_Module rtc;
HX711_Module hx711(PIN_HX711_DOUT, PIN_HX711_SCK);
KNN_Processor knn(KNN_K);
Schedule_Manager scheduleManager;
BuzzerModule buzzer(PIN_BUZZER);
EEPROM_Manager eepromManager;                      // (BARU) Instance untuk EEPROM
WiFi_Manager wifiManager("KotakObatPintar_Setup"); // (BARU) Buat WiFi dgn SSID ini
AsyncWebServer server(80);                         // (BARU) Buat server di port 80

// --- Variabel Global untuk Status Alarm ---
volatile bool isAlarmActive = false; // `volatile` agar aman diakses antar task

// --- Deklarasi Tugas & Queue FreeRTOS ---
TaskHandle_t h_taskSensorProcessor;
TaskHandle_t h_taskUIManager;
TaskHandle_t h_taskNotification;
QueueHandle_t q_displayData;

// Struktur data untuk komunikasi antar tugas
struct DisplayData
{
  int pillCount;
  char formattedDate[30];
};

// Dataset KNN Anda
const KNNDataPoint myDataset[] = {
    {{0.2f}, 0}, {{0.5f}, 0}, {{0.8f}, 0}, {{1.1f}, 0}, {{1.3f}, 0}, {{1.6f}, 1}, {{1.55f}, 1}, {{1.65f}, 1}, {{1.7f}, 1}, {{1.5f}, 1}, {{3.15f}, 2}, {{3.2f}, 2}, {{3.3f}, 2}, {{3.1f}, 2}, {{3.25f}, 2}, {{4.8f}, 3}, {{4.9f}, 3}, {{4.85f}, 3}, {{4.65f}, 3}, {{4.95f}, 3}, {{6.45f}, 4}, {{6.35f}, 4}, {{6.3f}, 4}, {{6.5f}, 4}, {{6.4f}, 4}, {{8.0f}, 5}, {{7.75f}, 5}, {{8.5f}, 5}, {{7.5f}, 5}, {{8.25f}, 5}, {{9.1f}, 6}, {{9.4f}, 6}, {{9.65f}, 6}, {{9.9f}, 6}, {{10.15f}, 6}, {{10.6f}, 7}, {{10.9f}, 7}, {{11.2f}, 7}, {{11.5f}, 7}, {{11.8f}, 7}, {{12.1f}, 8}, {{12.5f}, 8}, {{12.8f}, 8}, {{13.2f}, 8}, {{13.5f}, 8}, {{13.6f}, 9}, {{14.0f}, 9}, {{14.4f}, 9}, {{14.8f}, 9}, {{15.2f}, 9}, {{15.2f}, 10}, {{15.6f}, 10}, {{16.0f}, 10}, {{16.4f}, 10}, {{16.8f}, 10}, {{16.7f}, 11}, {{17.2f}, 11}, {{17.7f}, 11}, {{18.2f}, 11}, {{18.6f}, 11}, {{18.2f}, 12}, {{18.7f}, 12}, {{19.2f}, 12}, {{19.7f}, 12}, {{20.2f}, 12}, {{19.7f}, 13}, {{20.3f}, 13}, {{20.9f}, 13}, {{21.5f}, 13}, {{22.0f}, 13}, {{21.2f}, 14}, {{21.8f}, 14}, {{22.4f}, 14}, {{23.0f}, 14}, {{23.6f}, 14}, {{22.7f}, 15}, {{23.5f}, 15}, {{24.0f}, 15}, {{24.8f}, 15}, {{25.3f}, 15}};
const int DATASET_SIZE = sizeof(myDataset) / sizeof(myDataset[0]);

// --- (BARU) Fungsi untuk membuat halaman HTML ---
String buildHtmlPage()
{
  String html = "<!DOCTYPE html><html><head><title>Konfigurasi Kotak Obat</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family: Arial, sans-serif; background-color: #f0f8ff; margin: 20px;}";
  html += "h2{color: #005a8d;} .card{background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 20px;}";
  html += "input[type='text'], input[type='number']{width: calc(100% - 22px); padding: 10px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 4px;}";
  html += "input[type='submit'], button{background-color: #008CBA; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px;}";
  html += ".button-off{background-color: #f44336;} .schedule-item{display: flex; justify-content: space-between; align-items: center; padding: 8px; border-bottom: 1px solid #eee;}";
  html += "</style></head><body>";
  html += "<h2>Konfigurasi Kotak Obat Pintar</h2>";

  // --- Card untuk mematikan alarm ---
  if (isAlarmActive)
  {
    html += "<div class='card'><a href='/matikan-alarm'><button class='button-off'>MATIKAN ALARM SEKARANG</button></a></div>";
  }

  // --- Card untuk pengaturan utama ---
  html += "<div class='card'><form action='/simpan' method='POST'>";
  html += "<b>Nama Pasien:</b><br><input type='text' name='nama' value='" + String(config.patient_name) + "'><br>";
  html += "<b>Faktor Kalibrasi:</b><br><input type='text' name='kalibrasi' value='" + String(config.calibration_factor) + "'><br><br>";
  html += "<hr><b>Jadwal Minum Obat (Jam:Menit):</b><br>";
  for (int i = 0; i < 10; ++i)
  {
    html += "Jadwal " + String(i + 1) + ": ";
    html += "<input type='number' name='jam" + String(i) + "' min='0' max='23' placeholder='Jam' value='" + (i < config.schedule_count ? String(config.schedules[i].hour) : "") + "'>";
    html += "<input type='number' name='menit" + String(i) + "' min='0' max='59' placeholder='Menit' value='" + (i < config.schedule_count ? String(config.schedules[i].minute) : "") + "'>";
  }
  html += "<br><input type='submit' value='Simpan Semua Perubahan'>";
  html += "</form></div></body></html>";

  return html;
}

// --- (BARU) Definisi handler untuk Web Server ---
void setupWebServer()
{
  // Halaman utama
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", buildHtmlPage()); });

  // Halaman untuk menyimpan data
  server.on("/simpan", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              ConfigData newConfig = config; // Salin konfigurasi lama sebagai dasar
              newConfig.schedule_count = 0;

              // Ambil nama pasien
              if (request->hasParam("nama", true))
              {
                strncpy(newConfig.patient_name, request->getParam("nama", true)->value().c_str(), 50);
              }

              // Ambil faktor kalibrasi
              if (request->hasParam("kalibrasi", true))
              {
                newConfig.calibration_factor = request->getParam("kalibrasi", true)->value().toFloat();
              }

              // Ambil semua jadwal
              for (int i = 0; i < 10; ++i)
              {
                if (request->hasParam("jam" + String(i), true) && request->hasParam("menit" + String(i), true))
                {
                  String jamStr = request->getParam("jam" + String(i), true)->value();
                  String menitStr = request->getParam("menit" + String(i), true)->value();
                  if (jamStr != "" && menitStr != "") {
                    newConfig.schedules[newConfig.schedule_count].hour = jamStr.toInt();
                    newConfig.schedules[newConfig.schedule_count].minute = menitStr.toInt();
                    newConfig.schedule_count++;
                  }
                }
              }

              // Simpan konfigurasi baru ke struct global dan EEPROM
              config = newConfig;
              eepromManager.saveConfig(config);

              // Terapkan konfigurasi baru
              hx711.setCalibrationFactor(config.calibration_factor);
              
              // Refresh halaman
              request->send(200, "text/html", "<h1>Perubahan Disimpan!</h1><p>Memuat ulang halaman...</p><script>setTimeout(() => { window.location.href = '/'; }, 2000);</script>"); });

  // Halaman untuk mematikan alarm
  server.on("/matikan-alarm", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              isAlarmActive = false;
              request->send(200, "text/html", "<h1>Alarm Dimatikan!</h1><p>Memuat ulang halaman...</p><script>setTimeout(() => { window.location.href = '/'; }, 2000);</script>"); });

  server.begin();
}

// --- Implementasi Tugas FreeRTOS ---

void taskSensorAndDataProcessor(void *pvParameters)
{
  Serial.println("Tugas Sensor & Data Processor berjalan di Core 0.");
  DisplayData dataToSend;
  const char *days[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "Mei", "Jun", "Jul", "Ags", "Sep", "Okt", "Nov", "Des"};

  for (;;)
  {
    float currentWeight = hx711.getWeight();
    DateTime now = rtc.now();

    dataToSend.pillCount = knn.classify(currentWeight);
    sprintf(dataToSend.formattedDate, "%s, %d %s %d", days[now.dayOfTheWeek()], now.day(), months[now.month() - 1], now.year());

    xQueueOverwrite(q_displayData, &dataToSend);
    vTaskDelay(pdMS_TO_TICKS(TASK_SENSOR_INTERVAL_MS));
  }
}

void taskUIManager(void *pvParameters)
{
  Serial.println("Tugas UI Manager berjalan di Core 1.");
  DisplayData receivedData;

  for (;;)
  {
    if (xQueueReceive(q_displayData, &receivedData, portMAX_DELAY) == pdPASS)
    {
      // (DIMODIFIKASI) Ambil nama pasien dari config global
      tft.drawHomePage(config.patient_name, receivedData.pillCount, receivedData.formattedDate);
    }
  }
}

void taskNotificationHandler(void *pvParameters)
{
  Serial.println("Tugas Notifikasi berjalan di Core 0.");
  uint8_t lastDay = 0;

  for (;;)
  {
    DateTime now = rtc.now();

    if (now.day() != lastDay)
    {
      scheduleManager.resetAllTriggers();
      lastDay = now.day();
    }

    // Jika tidak ada alarm aktif, periksa jadwal
    if (!isAlarmActive)
    {
      if (scheduleManager.checkSchedules(now))
      {
        Serial.println("WAKTUNYA MINUM OBAT!");
        isAlarmActive = true;
      }
    }

    // (DIMODIFIKASI) Logika tombol dihapus, buzzer hanya dikontrol oleh flag
    if (isAlarmActive)
    {
      buzzer.playNotification();
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// --- SETUP & LOOP ---

void setup()
{
  Serial.begin(115200);
  Serial.println("\n--- Sistem Kotak Obat Pintar v3.0 (WiFi & EEPROM) ---");

  // Inisialisasi hardware dasar
  Wire.begin(I2C_SDA, I2C_SCL);
  tft.begin();
  rtc.begin();
  buzzer.begin();
  hx711.begin();
  knn.loadDataset(myDataset, DATASET_SIZE);

  // (BARU) Inisialisasi EEPROM dan muat konfigurasi
  if (eepromManager.begin())
  {
    eepromManager.loadConfig(config);
  }

  // (BARU) Terapkan konfigurasi yang sudah dimuat
  hx711.setCalibrationFactor(config.calibration_factor);
  scheduleManager.resetAllTriggers(); // Pastikan jadwal bersih
  for (int i = 0; i < config.schedule_count; i++)
  {
    scheduleManager.addSchedule(config.schedules[i].hour, config.schedules[i].minute);
  }
  Serial.printf("%d jadwal berhasil dimuat dari EEPROM.\n", config.schedule_count);

  // (BARU) Mulai WiFi AP dan Web Server
  wifiManager.startAPMode();
  setupWebServer();

  // Buat queue
  q_displayData = xQueueCreate(1, sizeof(DisplayData));

  // Buat tugas-tugas RTOS
  xTaskCreatePinnedToCore(taskSensorAndDataProcessor, "SensorTask", 4096, NULL, 1, &h_taskSensorProcessor, 0);
  xTaskCreatePinnedToCore(taskNotificationHandler, "NotificationTask", 4096, NULL, 1, &h_taskNotification, 0);
  xTaskCreatePinnedToCore(taskUIManager, "UITask", 4096, NULL, 2, &h_taskUIManager, 1);

  Serial.println("Sistem siap. Akses via WiFi 'KotakObatPintar_Setup'.");
}

void loop()
{
  vTaskSuspend(NULL);
}