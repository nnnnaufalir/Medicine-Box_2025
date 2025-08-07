// --- Library  ---
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <extEEPROM.h>
#include <RTClib.h>
#include "HX711.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// --- Konfigurasi Perangkat Keras & Sistem  ---
// Load Cell HX711
const int LOADCELL_DOUT_PIN = 13;
const int LOADCELL_SCK_PIN = 14;
// TFT ILI9341 (Hardware SPI - VSPI)
const int TFT_CS = 26;
const int TFT_DC = 25;
const int TFT_RST = 27;
const int TFT_MOSI = 23;
const int TFT_MISO = 19;
const int TFT_CLK = 18;
// Perangkat I/O
const int BUZZER_PIN = 32;
const int BATTERY_MONITOR_PIN = 34;
// I2C (RTC DS3231 & EEPROM AT24C32)
const int I2C_SDA_PIN = 21;
const int I2C_SCL_PIN = 22;
const uint8_t EEPROM_ADDRESS = 0x57;
// Konfigurasi Sistem
const long SERIAL_BAUD_RATE = 115200;
// Konfigurasi Jaringan (Web Server)
const char *WIFI_SSID = "KotakObat_ESP32";
const char *WIFI_PASSWORD = "";
// Konfigurasi Monitor Baterai
const float R1 = 1000.0;
const float R2 = 3300.0;
const float ADC_REF_VOLTAGE = 3.223;
const int ADC_RESOLUTION = 4095;

// --- 3. Halaman Web HTML (dari server_index.h) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Konfigurasi Kotak Obat</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 20px; color: #333; }
    .container { max-width: 500px; margin: 0 auto; background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h3 { color: #0056b3; text-align: center; }
    .input-group { margin-bottom: 15px; }
    label { display: block; margin-bottom: 5px; font-weight: bold; }
    input[type="text"], input[type="number"], input[type="time"] {
      width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box;
    }
    .btn {
      width: 100%; padding: 10px; border: none; border-radius: 4px; color: white; font-size: 16px; cursor: pointer;
    }
    .btn-save { background-color: #28a745; }
    .btn-reset { background-color: #dc3545; margin-top: 10px; }
    .schedule-group { display: flex; align-items: center; gap: 10px; }
    .schedule-group input[type="time"] { flex-grow: 1; }
  </style>
</head>
<body>
  <div class="container">
    <h3>Konfigurasi Kotak Obat</h3>
    <form id="settingsForm">
      <div class="input-group">
        <label for="patientName">Nama Pasien:</label>
        <input type="text" id="patientName" name="patientName" maxlength="31">
      </div>
      <div class="input-group">
        <label for="dose">Dosis (butir):</label>
        <input type="number" id="dose" name="dose">
      </div>
      
      <h4>Jadwal Minum Obat</h4>
      <div class="input-group schedule-group">
        <input type="checkbox" id="sched1_enabled" name="sched1_enabled">
        <input type="time" id="sched1_time" name="sched1_time">
      </div>
      <div class="input-group schedule-group">
        <input type="checkbox" id="sched2_enabled" name="sched2_enabled">
        <input type="time" id="sched2_time" name="sched2_time">
      </div>
      <div class="input-group schedule-group">
        <input type="checkbox" id="sched3_enabled" name="sched3_enabled">
        <input type="time" id="sched3_time" name="sched3_time">
      </div>
      
      <button type="submit" class="btn btn-save">Simpan Pengaturan</button>
    </form>
    <button onclick="resetDevice()" class="btn btn-reset">Reset Perangkat</button>
  </div>

  <script>
    window.onload = function() {
      fetch('/settings')
        .then(response => response.json())
        .then(data => {
          document.getElementById('patientName').value = data.patientName;
          document.getElementById('dose').value = data.dose;
          for (let i = 0; i < 3; i++) {
            let sched = data.schedules[i];
            document.getElementById(`sched${i+1}_enabled`).checked = sched.enabled;
            let timeStr = (sched.hour < 10 ? '0' : '') + sched.hour + ':' + (sched.minute < 10 ? '0' : '') + sched.minute;
            document.getElementById(`sched${i+1}_time`).value = timeStr;
          }
        });
    };

    document.getElementById('settingsForm').addEventListener('submit', function(event) {
      event.preventDefault();
      const formData = new FormData(this);
      const params = new URLSearchParams();
      for (const pair of formData) {
        params.append(pair[0], pair[1]);
      }
      
      fetch('/update', {
        method: 'POST',
        body: params,
      }).then(response => {
        if(response.ok) alert('Pengaturan berhasil disimpan!');
        else alert('Gagal menyimpan pengaturan.');
      });
    });

    function resetDevice() {
      if (confirm('Apakah Anda yakin ingin mereset perangkat?')) {
        fetch('/reset').then(() => alert('Perangkat akan di-reset.'));
      }
    }
  </script>
</body>
</html>
)rawliteral";

// --- 4. Definisi Struktur Data (dari Penyimpanan.h) ---
struct TimeSchedule {
  uint8_t hour;
  uint8_t minute;
  bool enabled;
};

struct UserSettings {
  char patientName[32];
  int dose;
  TimeSchedule schedules[3];
  char wifi_ssid[32];
  char wifi_password[64];
};

// --- 5. Variabel Global & Objek Library ---
// Variabel Status Sistem
volatile int g_pill_count = 0;
volatile bool g_isSystemStable = true;
volatile bool g_isFrozen = false;
volatile float g_battery_voltage = 0.0f;
volatile bool g_alarm_triggered = false;
volatile int g_dose_to_take = 0;

// Handel Sinkronisasi RTOS
SemaphoreHandle_t g_dataMutex;
TaskHandle_t sensorTaskHandle;
TaskHandle_t displayTaskHandle;
TaskHandle_t logicTaskHandle;
TaskHandle_t networkTaskHandle;

// Objek Library
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
AsyncWebServer server(80);
extEEPROM eeprom(kbits_32, 1, 32, EEPROM_ADDRESS);
RTC_DS3231 rtc;
HX711 scale;

// --- 6. Forward Declarations (PENTING untuk Arduino IDE) ---
// Deklarasi ini memberitahu compiler tentang fungsi-fungsi yang ada
// sebelum mereka benar-benar didefinisikan.
void sensorTask(void *pvParameters);
void displayTask(void *pvParameters);
void logicTask(void *pvParameters);
void networkTask(void *pvParameters);

void initDisplay();
void loopDisplay();
void initJaringan();
void loopLogic();
void initPenyimpanan();
void saveSettings(const UserSettings &settings);
UserSettings loadSettings();
void initWaktu();
DateTime getCurrentTime();
void initSensor();
void loopSensor();

// =================================================================
// SETUP - Inisialisasi Sistem
// =================================================================
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println("\n--- Booting Smart Pill Box System ---");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  initPenyimpanan();
  initWaktu();
  initDisplay();
  initSensor();
  initJaringan();

  Serial.println("--- Inisialisasi Modul Selesai ---");

  g_dataMutex = xSemaphoreCreateMutex();
  if (g_dataMutex == NULL) {
    Serial.println("Kesalahan: Gagal membuat Mutex!");
    while (1)
      ;
  }

  Serial.println("--- Membuat Tasks (dengan Core Affinity) ---");

  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 3, &sensorTaskHandle, 1);
  xTaskCreatePinnedToCore(networkTask, "NetworkTask", 4096, NULL, 1, &networkTaskHandle, 0);
  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 2048, NULL, 2, &displayTaskHandle, 0);
  xTaskCreatePinnedToCore(logicTask, "LogicTask", 2048, NULL, 2, &logicTaskHandle, 0);

  Serial.println("--- Sistem Siap. Scheduler RTOS dimulai. ---");
}

// =================================================================
// LOOP - Dibiarkan Kosong
// =================================================================
void loop() {
  // Semua pekerjaan dilakukan oleh task FreeRTOS.
}

// =================================================================
// IMPLEMENTASI TASK RTOS
// =================================================================
void sensorTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE) {
      loopSensor();
      checkAlarms();
      handleAlarmState();
      xSemaphoreGive(g_dataMutex);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void displayTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE) {
      loopDisplay();
      xSemaphoreGive(g_dataMutex);
    }
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void logicTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE) {
      loopLogic();
      xSemaphoreGive(g_dataMutex);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void networkTask(void *pvParameters) {
  for (;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// =================================================================
// MODUL PENYIMPANAN (Penyimpanan.cpp)
// =================================================================
const uint16_t SETTINGS_ADDRESS = 0;

void initPenyimpanan() {
  byte status = eeprom.begin(eeprom.twiClock100kHz);
  if (status) {
    Serial.print("Kesalahan: Inisialisasi extEEPROM gagal, status = ");
    Serial.println(status);
  }
}

void saveSettings(const UserSettings &settings) {
  Serial.println("Menyimpan pengaturan ke EEPROM...");
  byte i2cStat = eeprom.write(SETTINGS_ADDRESS, (byte *)&settings, sizeof(settings));
  if (i2cStat != 0) {
    Serial.print("Kesalahan saat menulis ke EEPROM, status = ");
    Serial.println(i2cStat);
  } else {
    Serial.println("Pengaturan berhasil disimpan.");
  }
}

UserSettings loadSettings() {
  UserSettings settings;
  Serial.println("Memuat pengaturan dari EEPROM...");
  byte i2cStat = eeprom.read(SETTINGS_ADDRESS, (byte *)&settings, sizeof(settings));
  if (i2cStat != 0) {
    Serial.print("Kesalahan saat membaca dari EEPROM, status = ");
    Serial.println(i2cStat);
  } else {
    Serial.println("Pengaturan berhasil dimuat.");
  }
  return settings;
}

// =================================================================
// MODUL WAKTU (Waktu.cpp)
// =================================================================
void initWaktu() {
  if (!rtc.begin()) {
    Serial.println("Kesalahan: Modul RTC tidak ditemukan!");
  }
  if (rtc.lostPower()) {
    Serial.println("RTC kehilangan daya, mengatur waktu ke waktu kompilasi.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

DateTime getCurrentTime() {
  return rtc.now();
}

// =================================================================
// MODUL LOGIKA (Logic.cpp)
// =================================================================
unsigned long last_battery_check_time = 0;
unsigned long last_buzzer_toggle_time = 0;
int count_sebelum_minum = 0;

float getBatteryVoltage() {
  int adc_raw_value = analogRead(BATTERY_MONITOR_PIN);
  float adc_voltage = (adc_raw_value / (float)ADC_RESOLUTION) * ADC_REF_VOLTAGE;
  float battery_voltage = adc_voltage * ((R1 + R2) / R2);
  return battery_voltage;
}

void checkAlarms() {
  if (g_alarm_triggered) {
    return;
  }
  DateTime now = getCurrentTime();
  UserSettings settings = loadSettings();
  for (int i = 0; i < 3; i++) {
    if (settings.schedules[i].enabled) {
      if (settings.schedules[i].hour == now.hour() && settings.schedules[i].minute == now.minute() && now.second() == 0) {
        Serial.println("ALARM TERPICU!");
        g_alarm_triggered = true;
        g_dose_to_take = settings.dose;
        count_sebelum_minum = g_pill_count;
        break;
      }
    }
  }
}

void handleAlarmState() {
  if (!g_alarm_triggered) {
    noTone(BUZZER_PIN);
    return;
  }
  if (g_alarm_triggered && millis() - last_buzzer_toggle_time > 2000) {
    tone(BUZZER_PIN, 5);
    last_buzzer_toggle_time = millis();
  }
  if (!g_isFrozen && g_isSystemStable) {
    if (g_pill_count <= (count_sebelum_minum - g_dose_to_take)) {
      Serial.println("Pengambilan obat terkonfirmasi. Mematikan alarm.");
      noTone(BUZZER_PIN);
      g_alarm_triggered = false;
    }
  }
}

void loopLogic() {
  // checkAlarms();
  // handleAlarmState();
  if (millis() - last_battery_check_time > 30000) {
    g_battery_voltage = getBatteryVoltage();
    last_battery_check_time = millis();
  }
}

// =================================================================
// MODUL JARINGAN (Jaringan.cpp)
// =================================================================
void initJaringan() {
  Serial.print("Membuat Access Point bernama: ");
  Serial.println(WIFI_SSID);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Alamat IP AP: ");
  Serial.println(myIP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    UserSettings settings = loadSettings();
    JsonDocument doc;
    doc["patientName"] = settings.patientName;
    doc["dose"] = settings.dose;
    JsonArray schedules = doc.createNestedArray("schedules");
    for (int i = 0; i < 3; i++) {
      JsonObject sched = schedules.add<JsonObject>();
      sched["hour"] = settings.schedules[i].hour;
      sched["minute"] = settings.schedules[i].minute;
      sched["enabled"] = settings.schedules[i].enabled;
    }
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    UserSettings settings;
    if (request->hasParam("patientName", true))
      strlcpy(settings.patientName, request->getParam("patientName", true)->value().c_str(), sizeof(settings.patientName));
    if (request->hasParam("dose", true))
      settings.dose = request->getParam("dose", true)->value().toInt();
    for (int i = 0; i < 3; i++) {
      String timeName = "sched" + String(i + 1) + "_time";
      String enabledName = "sched" + String(i + 1) + "_enabled";
      settings.schedules[i].enabled = request->hasParam(enabledName.c_str(), true);
      if (request->hasParam(timeName.c_str(), true)) {
        String timeValue = request->getParam(timeName.c_str(), true)->value();
        settings.schedules[i].hour = timeValue.substring(0, 2).toInt();
        settings.schedules[i].minute = timeValue.substring(3, 5).toInt();
      }
    }
    saveSettings(settings);
    request->send(200, "text/plain", "OK");
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Perangkat akan di-reset.");
    delay(1000);
    ESP.restart();
  });

  server.begin();
}

// =================================================================
// MODUL DISPLAY (Display.cpp)
// =================================================================
enum ScreenState { SCREEN_BOOT,
                   SCREEN_HOME,
                   SCREEN_POPUP };
ScreenState currentScreen = SCREEN_BOOT;
DateTime lastDisplayedTime;
int last_displayed_pill_count = -1;
int last_displayed_battery_percent = -1;
unsigned long popupBlinkTimer = 0;

#define COLOR_BACKGROUND 0x0186
#define COLOR_CARD 0x2309
#define COLOR_TEXT_HEADER 0xFFFF
#define COLOR_TEXT_BODY 0x9CF3
#define COLOR_ACCENT 0xFBE0

void drawBatteryIcon(int x, int y, int percentage) {
  tft.drawRect(x, y, 22, 12, COLOR_TEXT_BODY);
  tft.fillRect(x + 22, y + 3, 2, 6, COLOR_TEXT_BODY);
  if (percentage > 0) {
    int barWidth = map(percentage, 0, 100, 0, 18);
    uint16_t barColor = (percentage < 20) ? ILI9341_RED : ILI9341_GREEN;
    tft.fillRect(x + 2, y + 2, barWidth, 8, barColor);
  }
}

char daysOfTheWeek[7][12] = { "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu" };

void updateHomeScreenData() {
  UserSettings settings = loadSettings();
  DateTime now = getCurrentTime();
  if (now.second() != lastDisplayedTime.second()) {
    tft.setTextColor(COLOR_TEXT_HEADER, COLOR_BACKGROUND);
    tft.setTextSize(2);
    tft.setCursor(10, 8);
    char timeBuffer[25];
    sprintf(timeBuffer, "%s - %02d:%02d:%02d", daysOfTheWeek[now.dayOfTheWeek()], now.hour(), now.minute(), now.second());
    tft.print(timeBuffer);
    lastDisplayedTime = now;
  }

  if (g_pill_count != last_displayed_pill_count) {
    tft.fillRect(220, 70, 70, 32, COLOR_CARD);
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(4);
    tft.setCursor(220, 70);
    tft.print(g_pill_count);
    Serial.print("Obat :");
    Serial.println(g_pill_count);
    last_displayed_pill_count = g_pill_count;
  }

  int battery_percent = map(g_battery_voltage, 3.0, 4.2, 0, 100);
  if (battery_percent != last_displayed_battery_percent) {
    tft.fillRect(285, 8, 24, 12, COLOR_BACKGROUND);
    drawBatteryIcon(285, 8, battery_percent);
    last_displayed_battery_percent = battery_percent;
  }

  tft.setTextSize(2);
  tft.setCursor(30, 45);
  tft.setTextColor(COLOR_TEXT_BODY);
  tft.print("Pasien: ");
  tft.setTextColor(COLOR_TEXT_HEADER, COLOR_CARD);
  tft.print(settings.patientName);

  tft.setTextColor(COLOR_TEXT_HEADER, COLOR_CARD);
  tft.setCursor(110, 142);
  char scheduleBuffer[6];
  sprintf(scheduleBuffer, "%02d:%02d", settings.schedules[0].hour, settings.schedules[0].minute);
  tft.print(scheduleBuffer);

  tft.setTextColor(COLOR_TEXT_HEADER, COLOR_CARD);
  tft.setCursor(260, 142);
  tft.print(settings.dose);
}

void drawHomeScreen() {

  tft.fillScreen(COLOR_BACKGROUND);
  tft.fillRoundRect(15, 35, 290, 85, 8, COLOR_CARD);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT_BODY);
  tft.setCursor(30, 85);
  tft.print("Jumlah Obat:");
  tft.fillRoundRect(15, 128, 290, 45, 8, COLOR_CARD);
  tft.setTextColor(COLOR_TEXT_BODY);
  tft.setTextSize(2);
  tft.setCursor(30, 142);
  tft.print("Jadwal:");
  tft.setCursor(185, 142);
  tft.print("Dosis:");

  tft.fillRoundRect(15, 181, 290, 50, 8, COLOR_CARD);
  tft.setTextColor(COLOR_TEXT_BODY);
  tft.setTextSize(1);
  tft.setCursor(30, 192);
  tft.print("ID: ");
  tft.print(WIFI_SSID);
  tft.setCursor(30, 207);
  tft.print("IP: 192.168.4.1");
  updateHomeScreenData();
}

void drawPopupScreen() {
  uint16_t popupColors[] = { 0xFB20, ILI9341_DARKCYAN, ILI9341_PURPLE, 0x0433 };
  if (millis() - popupBlinkTimer > 1000) {
    uint16_t randomBgColor = popupColors[random(4)];
    tft.fillScreen(randomBgColor);
    tft.fillCircle(160, 80, 40, COLOR_BACKGROUND);
    tft.fillRect(120, 75, 80, 5, COLOR_BACKGROUND);
    tft.fillCircle(160, 130, 8, COLOR_BACKGROUND);
    tft.drawRect(150, 120, 20, 10, randomBgColor);
    tft.setTextColor(COLOR_TEXT_HEADER);
    tft.setTextSize(3);
    tft.setCursor(60, 160);
    tft.print("WAKTUNYA");
    tft.setCursor(45, 190);
    tft.print("MINUM OBAT!");
    popupBlinkTimer = millis();
  }
}

void drawBootScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setTextColor(COLOR_TEXT_HEADER);
  tft.setTextSize(2);
  tft.setCursor(40, 100);
  tft.print("Mempersiapkan Sistem");
  for (int i = 0; i < 4; i++) {
    tft.print(".");
    delay(500);
  }
}

void initDisplay() {
  tft.begin();
  tft.setRotation(1);
  drawBootScreen();
  currentScreen = SCREEN_HOME;
  drawHomeScreen();
}

void loopDisplay() {
  if (g_alarm_triggered && currentScreen != SCREEN_POPUP) {
    currentScreen = SCREEN_POPUP;
    popupBlinkTimer = 0;
  } else if (!g_alarm_triggered && currentScreen == SCREEN_POPUP) {
    currentScreen = SCREEN_HOME;
    drawHomeScreen();
  }
  if (currentScreen == SCREEN_HOME) {
    updateHomeScreenData();
  } else if (currentScreen == SCREEN_POPUP) {
    drawPopupScreen();
  }
}

// =================================================================
// MODUL SENSOR (Sensor.cpp)
// =================================================================
class EMAFilter {
private:
  float alpha, last_ema;
  bool has_run;
public:
  EMAFilter(float a)
    : alpha(a), last_ema(0.0f), has_run(false) {}
  float filter(float val) {
    if (!has_run) {
      last_ema = val;
      has_run = true;
    }
    last_ema = (alpha * val) + ((1.0 - alpha) * last_ema);
    return last_ema;
  }
};

const int K_VALUE = 21;
const int SAMPEL_PER_KELAS = 11;
const int JUMLAH_KELAS = 43;
struct KNNDataPoint {
  int jumlah_obat;
  long nilai_mentah[SAMPEL_PER_KELAS];
};

EMAFilter raw_filter(0.16);
const KNNDataPoint knn_dataset_original[JUMLAH_KELAS] = {
  { 42, { 334619, 334654, 334689, 334725, 334760, 334796, 334831, 334866, 334902, 334937, 334973 } },
  { 41, { 328293, 328381, 328469, 328558, 328646, 328735, 328823, 328911, 329000, 329088, 329177 } },
  { 40, { 324023, 324087, 324151, 324215, 324279, 324344, 324408, 324472, 324536, 324600, 324665 } },
  { 39, { 320521, 320587, 320653, 320720, 320786, 320853, 320919, 320985, 321052, 321118, 321185 } },
  { 38, { 311485, 311530, 311575, 311620, 311665, 311711, 311756, 311801, 311846, 311891, 311937 } },
  { 37, { 307080, 307116, 307153, 307190, 307227, 307264, 307300, 307337, 307374, 307411, 307448 } },
  { 36, { 302257, 302308, 302359, 302410, 302461, 302512, 302563, 302614, 302665, 302716, 302767 } },
  { 35, { 297717, 297771, 297826, 297881, 297936, 297991, 298045, 298100, 298155, 298210, 298265 } },
  { 34, { 293154, 293225, 293296, 293367, 293438, 293510, 293581, 293652, 293723, 293794, 293866 } },
  { 33, { 287759, 287795, 287832, 287868, 287905, 287942, 287978, 288015, 288051, 288088, 288125 } },
  { 32, { 280344, 280404, 280464, 280524, 280584, 280644, 280704, 280764, 280824, 280884, 280944 } },
  { 31, { 272980, 273046, 273112, 273179, 273245, 273312, 273378, 273444, 273511, 273577, 273644 } },
  { 30, { 267475, 267530, 267586, 267642, 267698, 267754, 267809, 267865, 267921, 267977, 268033 } },
  { 29, { 258550, 258602, 258654, 258707, 258759, 258812, 258864, 258916, 258969, 259021, 259074 } },
  { 28, { 252224, 252266, 252309, 252352, 252395, 252438, 252480, 252523, 252566, 252609, 252652 } },
  { 27, { 246569, 246641, 246713, 246785, 246857, 246930, 247002, 247074, 247146, 247218, 247291 } },
  { 26, { 240276, 240314, 240353, 240391, 240430, 240469, 240507, 240546, 240584, 240623, 240662 } },
  { 25, { 235666, 235727, 235789, 235850, 235912, 235974, 236035, 236097, 236158, 236220, 236282 } },
  { 24, { 228229, 228270, 228311, 228352, 228393, 228435, 228476, 228517, 228558, 228599, 228641 } },
  { 23, { 226149, 226194, 226240, 226286, 226332, 226378, 226423, 226469, 226515, 226561, 226607 } },
  { 22, { 219711, 219759, 219807, 219855, 219903, 219951, 219999, 220047, 220095, 220143, 220191 } },
  { 21, { 214689, 214731, 214773, 214816, 214858, 214901, 214943, 214985, 215028, 215070, 215113 } },
  { 20, { 208232, 208277, 208322, 208367, 208412, 208457, 208502, 208547, 208592, 208637, 208682 } },
  { 19, { 199027, 199082, 199138, 199194, 199250, 199306, 199361, 199417, 199473, 199529, 199585 } },
  { 18, { 194486, 194528, 194570, 194612, 194654, 194696, 194738, 194780, 194822, 194864, 194906 } },
  { 17, { 186954, 187000, 187046, 187092, 187138, 187184, 187230, 187276, 187322, 187368, 187414 } },
  { 16, { 181114, 181155, 181196, 181237, 181278, 181319, 181360, 181401, 181442, 181483, 181524 } },
  { 15, { 178951, 178995, 179039, 179083, 179127, 179171, 179215, 179259, 179303, 179347, 179391 } },
  { 14, { 173719, 173789, 173859, 173929, 173999, 174070, 174140, 174210, 174280, 174350, 174421 } },
  { 13, { 166185, 166275, 166365, 166456, 166546, 166637, 166727, 166817, 166908, 166998, 167089 } },
  { 12, { 160791, 160831, 160871, 160911, 160951, 160991, 161031, 161071, 161111, 161151, 161191 } },
  { 11, { 153493, 153551, 153609, 153668, 153726, 153785, 153843, 153901, 153960, 154018, 154077 } },
  { 10, { 149305, 149367, 149430, 149493, 149556, 149619, 149681, 149744, 149807, 149870, 149933 } },
  { 9, { 141084, 141139, 141194, 141250, 141305, 141361, 141416, 141471, 141527, 141582, 141638 } },
  { 8, { 138154, 138200, 138246, 138293, 138339, 138386, 138432, 138478, 138525, 138571, 138618 } },
  { 7, { 130646, 130883, 131121, 131358, 131596, 131834, 132071, 132309, 132546, 132784, 133022 } },
  { 6, { 124850, 124879, 124908, 124937, 124966, 124995, 125024, 125053, 125082, 125111, 125140 } },
  { 5, { 119421, 119466, 119511, 119556, 119601, 119647, 119692, 119737, 119782, 119827, 119873 } },
  { 4, { 112789, 112836, 112883, 112930, 112977, 113024, 113071, 113118, 113165, 113212, 113259 } },
  { 3, { 109166, 109215, 109265, 109314, 109364, 109414, 109463, 109513, 109562, 109612, 109662 } },
  { 2, { 102958, 103001, 103045, 103088, 103132, 103176, 103219, 103263, 103306, 103350, 103394 } },
  { 1, { 96820, 96854, 96889, 96924, 96959, 96994, 97028, 97063, 97098, 97133, 97168 } },
  { 0, { 89850, 89903, 89956, 90009, 90062, 90115, 90168, 90221, 90274, 90327, 90380 } }
};

KNNDataPoint knn_dataset_adjusted[JUMLAH_KELAS];
int last_stable_pill_count = 0;
unsigned long lastInteractionTime = 0;
float lastFilteredRawForStabilityCheck = 0.0;
float anchor_filtered_raw;
unsigned long last_drift_check_time = 0;
float current_stable_raw;
const unsigned long SETTLING_TIME_MS = 2500;
const float INTERACTION_THRESHOLD_RAW = 500;
const long HAND_INTERACTION_THRESHOLD_RAW = 70000;
const unsigned long DRIFT_CHECK_INTERVAL_MS = 10000;
const long STABILITY_THRESHOLD_RAW = 3000;

void adjustDatasetByOffset(long offset) {
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      knn_dataset_adjusted[i].nilai_mentah[j] += offset;
    }
  }
}

void adjustDatasetForDrift() {
  long original_zero_raw = knn_dataset_original[0].nilai_mentah[5];
  long current_zero_raw = scale.read_average(50);
  long drift_offset = current_zero_raw - original_zero_raw;
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    knn_dataset_adjusted[i].jumlah_obat = knn_dataset_original[i].jumlah_obat;
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      knn_dataset_adjusted[i].nilai_mentah[j] = knn_dataset_original[i].nilai_mentah[j] + drift_offset;
    }
  }
  anchor_filtered_raw = raw_filter.filter(current_zero_raw);
}

void updateSystemStability(float current_filtered_raw) {
  float change = abs(current_filtered_raw - lastFilteredRawForStabilityCheck);
  if (change > HAND_INTERACTION_THRESHOLD_RAW) {
    g_isSystemStable = false;
    g_isFrozen = true;
    lastInteractionTime = millis();
  } else if (change > INTERACTION_THRESHOLD_RAW) {
    g_isSystemStable = false;
    lastInteractionTime = millis();
  }
  if (!g_isSystemStable && (millis() - lastInteractionTime > SETTLING_TIME_MS)) {
    g_isSystemStable = true;
    g_isFrozen = false;
  }
  lastFilteredRawForStabilityCheck = current_filtered_raw;
}

int classifyPillCount(float current_filtered_raw) {
  const int MAX_K_SUPPORTED = 27;
  long nearest_distances[MAX_K_SUPPORTED];
  int nearest_classes[MAX_K_SUPPORTED];
  for (int i = 0; i < K_VALUE; i++) nearest_distances[i] = LONG_MAX;

  for (int i = 0; i < JUMLAH_KELAS; i++) {
    for (int j = 0; j < SAMPEL_PER_KELAS; j++) {
      long distance = abs(long(current_filtered_raw) - knn_dataset_adjusted[i].nilai_mentah[j]);
      for (int k = 0; k < K_VALUE; k++) {
        if (distance < nearest_distances[k]) {
          for (int m = K_VALUE - 1; m > k; m--) {
            nearest_distances[m] = nearest_distances[m - 1];
            nearest_classes[m] = nearest_classes[m - 1];
          }
          nearest_distances[k] = distance;
          nearest_classes[k] = knn_dataset_adjusted[i].jumlah_obat;
          break;
        }
      }
    }
  }

  float weighted_votes[JUMLAH_KELAS] = { 0.0f };
  for (int i = 0; i < K_VALUE; i++) {
    float weight = 1.0 / (nearest_distances[i] + 1e-6);
    weighted_votes[nearest_classes[i]] += weight;
  }

  float max_weight = -1.0f;
  int result = 0;
  for (int i = 0; i < JUMLAH_KELAS; i++) {
    if (weighted_votes[i] > max_weight) {
      max_weight = weighted_votes[i];
      result = i;
    }
  }
  return result;
}

void runStabilityGuardian(int current_pill_count) {
  if (millis() - last_drift_check_time < DRIFT_CHECK_INTERVAL_MS) return;
  if (g_isSystemStable) {
    long drift_diff = current_stable_raw - anchor_filtered_raw;
    if (current_pill_count == 0) {
      long old_zero_center = knn_dataset_adjusted[0].nilai_mentah[5];
      long refinement_offset = current_stable_raw - old_zero_center;
      if (abs(refinement_offset) > 50) {
        adjustDatasetByOffset(refinement_offset);
      }
    } else if (abs(drift_diff) < STABILITY_THRESHOLD_RAW) {
      adjustDatasetByOffset(drift_diff);
    }
    anchor_filtered_raw = current_stable_raw;
  }
  last_drift_check_time = millis();
}

void initSensor() {
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  adjustDatasetForDrift();
  lastFilteredRawForStabilityCheck = anchor_filtered_raw;
  last_drift_check_time = millis();
}

void loopSensor() {
  float filtered_raw = raw_filter.filter(scale.read());
  updateSystemStability(filtered_raw);
  int current_pill_prediction;
  if (g_isFrozen) {
    current_pill_prediction = last_stable_pill_count;
  } else {
    current_pill_prediction = classifyPillCount(filtered_raw);
    if (g_isSystemStable) {
      last_stable_pill_count = current_pill_prediction;
    }
  }
  g_pill_count = current_pill_prediction;
  runStabilityGuardian(current_pill_prediction);
  current_stable_raw = filtered_raw;
}
