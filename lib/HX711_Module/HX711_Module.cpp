#include "HX711_Module.h" // Include header kita sendiri
#include "HardwareConfig.h" // Untuk pin HX711
#include <Arduino.h>        // Untuk Serial.println dan delay

// --- Implementasi Kelas HX711_Module ---

void HX711_Module::begin(int doutPin, int sckPin) {
    Serial.println("HX711_Module: Menginisialisasi sensor HX711...");
    _scale.begin(doutPin, sckPin);

    if (_scale.is_ready()) {
        Serial.println("HX711_Module: Sensor HX711 siap.");
        tare(); // Lakukan tare awal saat start
        // Set faktor kalibrasi default atau dari EEPROM jika ada
        _calibrationFactor = 1.0f; // Nilai placeholder, akan di-override setelah kalibrasi
    } else {
        Serial.println("HX711_Module: Sensor HX711 tidak ditemukan atau tidak siap.");
    }
}

void HX711_Module::setCalibrationFactor(float scale) {
    _calibrationFactor = scale;
    _scale.set_scale(scale); // Set faktor skala di pustaka HX711
    Serial.printf("HX711_Module: Faktor kalibrasi diatur ke: %.2f\n", _calibrationFactor);
}

void HX711_Module::tare() {
    _scale.tare(); // Lakukan tare di pustaka HX711
    Serial.println("HX711_Module: Tare (zeroing) selesai.");
}

float HX711_Module::getCalibratedWeight(byte times) {
    if (_scale.is_ready()) {
        // Pustaka HX711 akan menggunakan faktor skala yang sudah diset
        // dan offset dari tare() secara internal.
        return _scale.get_units(times); // Mengambil nilai dalam unit yang dikalibrasi
    }
    Serial.println("HX711_Module: Sensor tidak siap untuk membaca berat.");
    return 0.0f; // Kembalikan 0.0 jika sensor tidak siap
}

long HX711_Module::getRawValue(byte times) {
    if (_scale.is_ready()) {
        return _scale.read_average(times); // Mengambil nilai mentah rata-rata
    }
    Serial.println("HX711_Module: Sensor tidak siap untuk membaca nilai mentah.");
    return 0; // Kembalikan 0 jika sensor tidak siap
}

bool HX711_Module::isReady() {
    return _scale.is_ready();
}

float HX711_Module::getScale() {
    return _scale.get_scale();
}

long HX711_Module::getOffset() {
    return _scale.get_offset();
}