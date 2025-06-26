#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>

class TFT_Display
{
public:
    // Konstruktor diubah menjadi parameter-less
    TFT_Display();

    void begin();
    void drawHomePage(const char *patientName, int pillCount, const char *date);
    SemaphoreHandle_t getMutex();

private:
    Adafruit_ILI9341 _tft;
    SemaphoreHandle_t _tftMutex;

    String _prevPatientName;
    int _prevPillCount = -1;
    String _prevDate;
};

#endif // TFT_DISPLAY_H
