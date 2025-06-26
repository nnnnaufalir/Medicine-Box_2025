#include "HX711_Module.h"
#include "AppConfig.h" // Diperlukan untuk EMA_ALPHA

HX711_Module::HX711_Module(byte dout, byte sck)
    : _emaFilter(EMA_ALPHA)
{
    _scale.begin(dout, sck);
}

void HX711_Module::begin()
{
    // Faktor kalibrasi akan diatur dari luar setelah inisialisasi
    _scale.set_scale(1.0f);
    tare();
}

bool HX711_Module::isReady()
{
    return _scale.is_ready();
}

void HX711_Module::tare()
{
    if (_scale.is_ready())
    {
        _scale.tare();
    }
}

void HX711_Module::setCalibrationFactor(float factor)
{
    _scale.set_scale(factor);
}

float HX711_Module::getWeight()
{
    if (_scale.is_ready())
    {
        // Ambil rata-rata dari 5 bacaan untuk stabilitas
        float reading = _scale.get_units(5);
        // Perhalus hasilnya dengan filter
        return _emaFilter.update(reading);
    }
    // Jika sensor tidak siap, kembalikan nilai terakhir yang diketahui
    return _emaFilter.getValue();
}
