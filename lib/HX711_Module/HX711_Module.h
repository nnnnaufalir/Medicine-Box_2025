#ifndef HX711_MODULE_H
#define HX711_MODULE_H

#include <HX711.h>
#include "Utils/Utils.h" // Diperlukan untuk EMAFilter

class HX711_Module
{
public:
    HX711_Module(byte dout, byte sck);
    void begin();
    bool isReady();
    void tare();
    void setCalibrationFactor(float factor);
    float getWeight();

private:
    HX711 _scale;
    EMAFilter<float> _emaFilter;
};

#endif // HX711_MODULE_H
