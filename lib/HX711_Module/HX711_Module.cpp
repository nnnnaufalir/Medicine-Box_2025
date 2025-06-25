#include "HX711_Module.h"   // Include header
#include "HardwareConfig.h" // Untuk pin HX711
#include <Arduino.h>        //

// --- Implementasi Kelas HX711_Module ---

void HX711_Module::begin(int doutPin, int sckPin)
{
    Serial.println("HX711_Module: Menginisialisasi sensor HX711...");
    _scale.begin(doutPin, sckPin);

    Serial.println("HX711_Module: Sensor HX711 siap.");
    tare();                    // Lakukan tare awal saat start
                               // Set faktor kalibrasi default atau dari EEPROM jika ada
    _calibrationFactor = 1.0f; // Nilai placeholder, akan di-override setelah kalibrasi
}

void HX711_Module::setCalibrationFactor(float scale)
{
    _calibrationFactor = scale;
    _scale.set_scale(scale); // Set faktor skala di pustaka HX711
    Serial.printf("HX711_Module: Faktor kalibrasi diatur ke: %.2f\n", _calibrationFactor);
}

void HX711_Module::tare()
{
    _scale.tare(); // Lakukan tare di pustaka HX711
    Serial.println("HX711_Module: Tare (zeroing) selesai.");
}

float HX711_Module::getCalibratedWeight(byte times)
{
    return _scale.get_units(times); // Mengambil nilai dalam unit yang dikalibrasi
}

long HX711_Module::getRawValue(byte times)
{
    return _scale.read_average(times); // Mengambil nilai mentah rata-rata
}

bool HX711_Module::isReady()
{
    return _scale.is_ready();
}

float HX711_Module::getScale()
{
    return _scale.get_scale();
}

long HX711_Module::getOffset()
{
    return _scale.get_offset();
}