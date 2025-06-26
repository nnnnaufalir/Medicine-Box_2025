#include "Utils.h"

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
// Ini diperlukan agar linker dapat menemukan implementasi template.
template class EMAFilter<float>;

// --- Implementasi BatteryMonitor ---
BatteryMonitor::BatteryMonitor(int pin) : _pin(pin) {}

void BatteryMonitor::begin()
{
    pinMode(_pin, INPUT);
}

float BatteryMonitor::getBatteryLevel()
{
    // Implementasi placeholder
    // Anda perlu memetakan nilai ADC (0-4095) ke persentase baterai (0-100)
    // sesuai dengan rangkaian pembagi tegangan Anda.
    int adcValue = analogRead(_pin);
    // Contoh mapping sederhana, harus disesuaikan:
    float voltage = adcValue * (3.3 / 4095.0);
    return (voltage / 4.2) * 100.0; // Asumsi baterai Li-Ion (maks 4.2V)
}

// --- Implementasi BuzzerModule ---
BuzzerModule::BuzzerModule(int pin) : _pin(pin) {}

void BuzzerModule::begin()
{
    pinMode(_pin, OUTPUT);
}

void BuzzerModule::buzz(int freq, int duration)
{
    // Menggunakan LEDC untuk nada pada ESP32 agar tidak blocking
    ledcSetup(0, freq, 8); // Channel 0, Freq, Resolusi 8-bit
    ledcAttachPin(_pin, 0);
    ledcWrite(0, 128); // Duty cycle 50%
    delay(duration);
    ledcWrite(0, 0); // Matikan
    ledcDetachPin(_pin);
}

void BuzzerModule::playNotification()
{
    // Contoh suara notifikasi
    buzz(1000, 150);
    delay(50);
    buzz(1000, 150);
}

// --- Implementasi ButtonMonitor ---
ButtonMonitor::ButtonMonitor(int pin1, int pin2) : _pin1(pin1), _pin2(pin2)
{
    pinMode(_pin1, INPUT_PULLUP);
    pinMode(_pin2, INPUT_PULLUP);
}

void ButtonMonitor::check()
{
    // Reset status penekanan tombol
    _button1Pressed = false;
    _button2Pressed = false;

    // Logika Debounce untuk Tombol 1
    bool reading1 = digitalRead(_pin1);
    if (reading1 != _lastButtonState1)
    {
        _lastDebounceTime1 = millis();
    }
    if ((millis() - _lastDebounceTime1) > DEBOUNCE_DELAY)
    {
        if (reading1 != _buttonState1)
        {
            _buttonState1 = reading1;
            if (_buttonState1 == LOW)
            { // Tombol ditekan (LOW karena PULLUP)
                _button1Pressed = true;
            }
        }
    }
    _lastButtonState1 = reading1;

    // Logika Debounce untuk Tombol 2
    bool reading2 = digitalRead(_pin2);
    if (reading2 != _lastButtonState2)
    {
        _lastDebounceTime2 = millis();
    }
    if ((millis() - _lastDebounceTime2) > DEBOUNCE_DELAY)
    {
        if (reading2 != _buttonState2)
        {
            _buttonState2 = reading2;
            if (_buttonState2 == LOW)
            { // Tombol ditekan
                _button2Pressed = true;
            }
        }
    }
    _lastButtonState2 = reading2;
}

bool ButtonMonitor::isButton1Pressed()
{
    return _button1Pressed;
}

bool ButtonMonitor::isButton2Pressed()
{
    return _button2Pressed;
}
