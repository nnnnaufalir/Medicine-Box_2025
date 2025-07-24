#include "Display.h"
#include "Config/AppConfig.h"
#include "Waktu/Waktu.h"
#include "Penyimpanan/Penyimpanan.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// --- Deklarasi Variabel Global dari main.cpp ---
extern volatile int g_pill_count;
extern volatile bool g_isSystemStable;
extern volatile bool g_isFrozen;
extern volatile float g_battery_voltage;
extern volatile bool g_alarm_triggered;

// --- Inisialisasi Driver & Variabel Internal ---
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
ScreenState currentScreen = SCREEN_BOOT;
DateTime lastDisplayedTime;
int last_displayed_pill_count = -1;
int last_displayed_battery_percent = -1;
unsigned long popupBlinkTimer = 0;

// --- Definisi Warna UI/UX ---
#define COLOR_BACKGROUND 0x0186
#define COLOR_CARD 0x2309
#define COLOR_TEXT_HEADER 0xFFFF
#define COLOR_TEXT_BODY 0x9CF3
#define COLOR_ACCENT 0xFBE0

// --- Implementasi Fungsi Gambar (diadaptasi dari kode Anda) ---

void drawBatteryIcon(int x, int y, int percentage)
{
    tft.drawRect(x, y, 22, 12, COLOR_TEXT_BODY);
    tft.fillRect(x + 22, y + 3, 2, 6, COLOR_TEXT_BODY);
    if (percentage > 0)
    {
        int barWidth = map(percentage, 0, 100, 0, 18);
        uint16_t barColor = (percentage < 20) ? ILI9341_RED : ILI9341_GREEN;
        tft.fillRect(x + 2, y + 2, barWidth, 8, barColor);
    }
}

void updateHomeScreenData()
{
    DateTime now = getCurrentTime();
    if (now.second() != lastDisplayedTime.second())
    {
        tft.setTextColor(COLOR_TEXT_HEADER, COLOR_BACKGROUND);
        tft.setTextSize(2);
        tft.setCursor(10, 8);
        char timeBuffer[20];
        sprintf(timeBuffer, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        tft.print(timeBuffer);
        lastDisplayedTime = now;
    }

    if (g_pill_count != last_displayed_pill_count)
    {
        tft.fillRect(220, 70, 70, 32, COLOR_CARD); // Hapus area angka lama
        tft.setTextColor(COLOR_ACCENT);
        tft.setTextSize(4);
        tft.setCursor(220, 70);
        tft.print(g_pill_count);
        last_displayed_pill_count = g_pill_count;
    }

    int battery_percent = map(g_battery_voltage, 3.0, 4.2, 0, 100);
    if (battery_percent != last_displayed_battery_percent)
    {
        tft.fillRect(285, 8, 24, 12, COLOR_BACKGROUND); // Hapus ikon baterai lama
        drawBatteryIcon(285, 8, battery_percent);
        last_displayed_battery_percent = battery_percent;
    }
}

void drawHomeScreen()
{
    UserSettings settings = loadSettings();
    tft.fillScreen(COLOR_BACKGROUND);

    // Status Bar
    updateHomeScreenData();

    // Kartu Utama
    tft.fillRoundRect(15, 35, 290, 85, 8, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT_HEADER);
    tft.setTextSize(2);
    tft.setCursor(30, 45);
    tft.print("Pasien: ");
    tft.print(settings.patientName);
    tft.setTextColor(COLOR_TEXT_BODY);
    tft.setCursor(30, 85);
    tft.print("Jumlah Obat:");

    // Kartu Info
    tft.fillRoundRect(15, 128, 290, 45, 8, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT_BODY);
    tft.setTextSize(2);
    tft.setCursor(30, 142);
    tft.print("Jadwal:");
    tft.setCursor(185, 142);
    tft.print("Dosis:");
    tft.setTextColor(COLOR_TEXT_HEADER);
    tft.setCursor(110, 142);
    char scheduleBuffer[6];
    sprintf(scheduleBuffer, "%02d:%02d", settings.schedules[0].hour, settings.schedules[0].minute);
    tft.print(scheduleBuffer);
    tft.setCursor(260, 142);
    tft.print(settings.dose);

    // Kartu Jaringan
    tft.fillRoundRect(15, 181, 290, 50, 8, COLOR_CARD);
    tft.setTextColor(COLOR_TEXT_BODY);
    tft.setTextSize(1);
    tft.setCursor(30, 192);
    tft.print("ID: ");
    tft.print(settings.wifi_ssid);
    tft.setCursor(30, 207);
    tft.print("IP: 192.168.4.1");
}

void drawPopupScreen()
{
    uint16_t popupColors[] = {0xFB20, ILI9341_DARKCYAN, ILI9341_PURPLE, 0x0433};
    if (millis() - popupBlinkTimer > 1000)
    {
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

void drawBootScreen()
{
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(COLOR_TEXT_HEADER);
    tft.setTextSize(2);
    tft.setCursor(40, 100);
    tft.print("Mempersiapkan Sistem");
    for (int i = 0; i < 4; i++)
    {
        tft.print(".");
        delay(500);
    }
}

// --- Fungsi Utama Modul ---

void initDisplay()
{
    tft.begin();
    tft.setRotation(3);
    drawBootScreen();
    currentScreen = SCREEN_HOME; // Langsung ke home setelah boot
    drawHomeScreen();            // Gambar layar utama sekali
}

void loopDisplay()
{
    if (g_alarm_triggered && currentScreen != SCREEN_POPUP)
    {
        currentScreen = SCREEN_POPUP;
        popupBlinkTimer = 0; // Reset timer agar langsung ganti warna
    }
    else if (!g_alarm_triggered && currentScreen == SCREEN_POPUP)
    {
        currentScreen = SCREEN_HOME;
        drawHomeScreen(); // Gambar ulang home screen
    }

    if (currentScreen == SCREEN_HOME)
    {
        updateHomeScreenData();
    }
    else if (currentScreen == SCREEN_POPUP)
    {
        drawPopupScreen();
    }
}
