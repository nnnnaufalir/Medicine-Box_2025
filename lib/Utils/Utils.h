#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/**
 * @class EMAFilter
 * @brief Sebuah template untuk filter Exponential Moving Average.
 * Berguna untuk memperhalus data sensor yang berfluktuasi.
 */
template <typename T>
class EMAFilter
{
public:
    // Konstruktor dengan faktor alpha (0 < alpha < 1)
    EMAFilter(float alpha);

    // Memperbarui filter dengan nilai baru dan mengembalikan nilai yang sudah difilter
    T update(T newValue);

    // Mendapatkan nilai terfilter terakhir tanpa memperbarui
    T getValue() const;

    // Mereset filter
    void reset();

private:
    float _alpha;
    T _filteredValue;
    bool _isInitialized;
};

/**
 * @class BatteryMonitor
 * @brief Untuk memantau level tegangan baterai melalui pin ADC.
 */
class BatteryMonitor
{
public:
    BatteryMonitor(int pin);
    void begin();
    float getBatteryLevel(); // Mengembalikan level baterai (misal: 0-100%)

private:
    int _pin;
};

/**
 * @class BuzzerModule
 * @brief Untuk mengontrol buzzer.
 */
class BuzzerModule
{
public:
    BuzzerModule(int pin);
    void begin();
    void buzz(int freq, int duration);
    void playNotification(); // Memainkan suara notifikasi standar

private:
    int _pin;
};

/**
 * @class ButtonMonitor
 * @brief Untuk memantau tombol dengan logika debouncing.
 */
class ButtonMonitor
{
public:
    ButtonMonitor(int pin1, int pin2);
    void check(); // Harus dipanggil secara berkala di dalam loop/task
    bool isButton1Pressed();
    bool isButton2Pressed();

private:
    int _pin1, _pin2;
    // Variabel untuk state dan debouncing
    unsigned long _lastDebounceTime1 = 0;
    unsigned long _lastDebounceTime2 = 0;
    bool _lastButtonState1 = HIGH;
    bool _lastButtonState2 = HIGH;
    bool _buttonState1 = HIGH;
    bool _buttonState2 = HIGH;
    bool _button1Pressed = false;
    bool _button2Pressed = false;
    const unsigned long DEBOUNCE_DELAY = 50;
};

#endif // UTILS_H
