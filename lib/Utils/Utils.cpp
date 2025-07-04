#include "Utils.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- Implementasi WiFi_Manager (BARU) ---
WiFi_Manager::WiFi_Manager(const char *ssid) : _ssid(ssid) {}

void WiFi_Manager::startAPMode()
{
    Serial.println("Memulai Access Point Mode...");
    WiFi.softAP(_ssid); // Tidak menggunakan password

    IPAddress apIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(apIP);
}

// --- Implementasi EMAFilter ---
template <typename T>
EMAFilter<T>::EMAFilter(float alpha)
    : _alpha(alpha), _filteredValue(0), _isInitialized(false) {}

template <typename T>
T EMAFilter<T>::update(T newValue)
{
    if (!_isInitialized)
    {
        _filteredValue = newValue;
        _isInitialized = true;
    }
    else
    {
        _filteredValue = (_alpha * newValue) + ((1.0f - _alpha) * _filteredValue);
    }
    return _filteredValue;
}

template <typename T>
T EMAFilter<T>::getValue() const
{
    return _filteredValue;
}

template <typename T>
void EMAFilter<T>::reset()
{
    _isInitialized = false;
    _filteredValue = 0;
}

// Inisialisasi eksplisit untuk tipe data yang akan kita gunakan (float)
template class EMAFilter<float>;

// --- Implementasi BatteryMonitor ---
BatteryMonitor::BatteryMonitor(int pin) : _pin(pin) {}

void BatteryMonitor::begin()
{
    pinMode(_pin, INPUT);
}

float BatteryMonitor::getBatteryLevel()
{
    int adcValue = analogRead(_pin);
    float voltage = adcValue * (3.3 / 4095.0);
    return (voltage / 4.2) * 100.0;
}

// --- Implementasi BuzzerModule ---
BuzzerModule::BuzzerModule(int pin) : _pin(pin) {}

void BuzzerModule::begin()
{
    pinMode(_pin, OUTPUT);
}

void BuzzerModule::buzz(int freq, int duration)
{
    ledcSetup(0, freq, 8);
    ledcAttachPin(_pin, 0);
    ledcWrite(0, 128);
    delay(duration);
    ledcWrite(0, 0);
    ledcDetachPin(_pin);
}

void BuzzerModule::playNotification()
{
    buzz(1000, 150);
    delay(50);
    buzz(1000, 150);
}

// Implementasi ButtonMonitor sudah dihapus.