#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>

class TFT_Display
{
public:
    TFT_Display();
    void begin();
    void drawHomePage(const char *patientName, int pillCount, const char *date);

private:
    Adafruit_ILI9341 _tft;

    // Variabel untuk optimasi agar tidak menggambar ulang jika data sama
    String _prevPatientName;
    int _prevPillCount = -1;
    String _prevDate;
};

#endif // TFT_DISPLAY_H
