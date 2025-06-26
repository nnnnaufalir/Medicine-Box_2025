#include "TFT_Display.h"
#include "HardwareConfig.h" // Diperlukan untuk mengakses pin hardware

// Definisi warna kustom
#define THEME_BG 0x020C
#define THEME_TEXT ILI9341_WHITE
#define THEME_ACCENT ILI9341_YELLOW
#define THEME_LABEL ILI9341_LIGHTGREY

// Konstruktor sekarang menggunakan pin dari HardwareConfig.h
TFT_Display::TFT_Display()
    : _tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_MOSI, PIN_TFT_SCLK, PIN_TFT_RST) {}

void TFT_Display::begin()
{
    _tftMutex = xSemaphoreCreateMutex();
    _tft.begin();
    _tft.setRotation(3);
    _tft.fillScreen(THEME_BG);
}

SemaphoreHandle_t TFT_Display::getMutex()
{
    return _tftMutex;
}

void TFT_Display::drawHomePage(const char *patientName, int pillCount, const char *date)
{
    if (_prevPatientName.equals(patientName) && _prevPillCount == pillCount && _prevDate.equals(date))
    {
        return;
    }

    if (xSemaphoreTake(_tftMutex, portMAX_DELAY) == pdTRUE)
    {

        if (_prevPillCount == -1)
        {
            _tft.fillScreen(THEME_BG);
            _tft.setCursor(10, 10);
            _tft.setTextColor(THEME_ACCENT);
            _tft.setTextSize(2);
            _tft.println("Kotak Obat Pintar");
        }

        _tft.setCursor(10, 40);
        _tft.setTextColor(THEME_LABEL);
        _tft.setTextSize(2);
        _tft.println("Pasien:");
        _tft.fillRect(10, 60, 220, 24, THEME_BG);
        _tft.setCursor(10, 60);
        _tft.setTextColor(THEME_TEXT);
        _tft.setTextSize(3);
        _tft.println(patientName);
        _prevPatientName = patientName;

        _tft.setCursor(10, 110);
        _tft.setTextColor(THEME_LABEL);
        _tft.setTextSize(2);
        _tft.println("Jumlah Obat Saat Ini:");
        _tft.fillRect(70, 140, 100, 50, THEME_BG);
        _tft.setCursor(80, 140);
        _tft.setTextColor(THEME_ACCENT);
        _tft.setTextSize(7);
        _tft.println(pillCount);
        _prevPillCount = pillCount;

        _tft.fillRect(10, 220, 300, 16, THEME_BG);
        _tft.setCursor(10, 220);
        _tft.setTextColor(THEME_LABEL);
        _tft.setTextSize(2);
        _tft.println(date);
        _prevDate = date;

        xSemaphoreGive(_tftMutex);
    }
}
