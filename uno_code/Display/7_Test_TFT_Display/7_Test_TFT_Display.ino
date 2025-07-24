#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

// --- Definisi Pin TFT sesuai konfigurasi Anda ---
#define TFT_CS 26
#define TFT_DC 25
#define TFT_RST 27
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_CLK 18

// --- Inisialisasi driver TFT dengan 6 argumen ---
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
RTC_DS3231 rtc;

// --- Definisi Warna UI/UX (Tema Kebiruan) ---
#define COLOR_BACKGROUND 0x0186   // Biru Tua Gelap
#define COLOR_CARD 0x2309         // Biru Sedikit Lebih Terang
#define COLOR_TEXT_HEADER 0xFFFF  // Putih
#define COLOR_TEXT_BODY 0x9CF3    // Abu-abu Terang
#define COLOR_ACCENT 0xFBE0       // Kuning

// --- Variabel State & Data Dummy ---
enum ScreenState { SCREEN_BOOT,
                   SCREEN_HOME,
                   SCREEN_POPUP };
ScreenState currentScreen = SCREEN_BOOT;
unsigned long screenTimer = 0;
char daysOfTheWeek[7][12] = { "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu" };
DateTime lastDisplayedTime;

// --- FUNGSI-FUNGSI TAMPILAN ---

void drawBatteryIcon(int x, int y, int percentage) {
  tft.drawRect(x, y, 22, 12, COLOR_TEXT_BODY);
  tft.fillRect(x + 22, y + 3, 2, 6, COLOR_TEXT_BODY);
  if (percentage > 0) {
    int barWidth = map(percentage, 0, 100, 0, 18);
    uint16_t barColor = (percentage < 20) ? ILI9341_RED : ILI9341_GREEN;
    tft.fillRect(x + 2, y + 2, barWidth, 8, barColor);
  }
}

void updateHomeScreenData() {
  DateTime now = rtc.now();
  if (now.second() != lastDisplayedTime.second()) {
    tft.setTextColor(COLOR_TEXT_HEADER, COLOR_BACKGROUND);
    tft.setTextSize(2);
    tft.setCursor(10, 8);

    tft.print(daysOfTheWeek[now.dayOfTheWeek()]);
    tft.print(", ");
    if (now.hour() < 10) tft.print('0');
    tft.print(now.hour());
    tft.print(":");
    if (now.minute() < 10) tft.print('0');
    tft.print(now.minute());
    tft.print(":");
    if (now.second() < 10) tft.print('0');
    tft.print(now.second());

    lastDisplayedTime = now;
  }
}

void drawHomeScreen() {
  tft.fillScreen(COLOR_BACKGROUND);
  drawBatteryIcon(285, 8, 82);

  // Kartu Utama (Nama & Jumlah Obat)
  tft.fillRoundRect(15, 35, 290, 85, 8, COLOR_CARD);
  tft.setTextColor(COLOR_TEXT_HEADER);
  tft.setTextSize(2);
  tft.setCursor(30, 45);
  tft.print("Pasien: Naufal Ilham");
  tft.setTextColor(COLOR_TEXT_BODY);
  tft.setCursor(30, 85);
  tft.print("Jumlah Obat:");
  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(4);
  tft.setCursor(220, 70);
  tft.print("25");

  // Kartu Info (Jadwal & Dosis) dengan tata letak baru
  tft.fillRoundRect(15, 128, 290, 45, 8, COLOR_CARD);
  tft.setTextColor(COLOR_TEXT_BODY);
  tft.setTextSize(2);
  tft.setCursor(30, 142);
  tft.print("Jadwal:");
  tft.setCursor(185, 142);
  tft.print("Dosis:");
  tft.setTextColor(COLOR_TEXT_HEADER);
  tft.setCursor(110, 142);
  tft.print("07:00");
  tft.setCursor(260, 142);
  tft.print("1");

  // Kartu Jaringan (Web Server)
  tft.fillRoundRect(15, 181, 290, 50, 8, COLOR_CARD);
  tft.setTextColor(COLOR_TEXT_BODY);
  tft.setTextSize(1);
  tft.setCursor(30, 192);
  tft.print("ID: KotakObat_ESP32 | Pass: password123");
  tft.setCursor(30, 207);
  tft.print("IP Address: 192.168.4.1");
}

void drawPopupScreen() {
  uint16_t popupColors[] = { 0xFB20, ILI9341_DARKCYAN, ILI9341_PURPLE, 0x0433 };
  uint16_t randomBgColor = popupColors[random(4)];  // Simpan warna acak
  tft.fillScreen(randomBgColor);                    // Gunakan warna acak

  // Gambar ikon lonceng
  tft.fillCircle(160, 80, 40, COLOR_BACKGROUND);
  tft.fillRect(120, 75, 80, 5, COLOR_BACKGROUND);
  tft.fillCircle(160, 130, 8, COLOR_BACKGROUND);
  // [PERBAIKAN] Gunakan warna acak yang tersimpan untuk "menghapus" bagian dari lonceng
  tft.drawRect(150, 120, 20, 10, randomBgColor);

  // Teks Peringatan
  tft.setTextColor(COLOR_TEXT_HEADER);
  tft.setTextSize(3);
  tft.setCursor(60, 160);
  tft.print("WAKTUNYA");
  tft.setCursor(45, 190);
  tft.print("MINUM OBAT!");
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

void setup() {
  Serial.begin(115200);
  tft.begin();

  if (rtc.begin()) {
    if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    Serial.println("RTC tidak ditemukan.");
  }

  tft.setRotation(3);
  randomSeed(analogRead(0));
}

void loop() {
  switch (currentScreen) {
    case SCREEN_BOOT:
      drawBootScreen();
      currentScreen = SCREEN_HOME;
      drawHomeScreen();
      screenTimer = millis();
      break;

    case SCREEN_HOME:
      updateHomeScreenData();
      if (millis() - screenTimer > 15000) {
        currentScreen = SCREEN_POPUP;
        drawPopupScreen();
        screenTimer = millis();
      }
      break;

    case SCREEN_POPUP:
      if (millis() - screenTimer > 10000) {
        currentScreen = SCREEN_HOME;
        drawHomeScreen();
        screenTimer = millis();
      }
      break;
  }
}
