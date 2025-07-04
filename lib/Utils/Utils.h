#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/**
 * @class WiFi_Manager (BARU)
 * @brief Untuk mengelola konektivitas WiFi dalam mode Access Point.
 */
class WiFi_Manager
{
public:
    WiFi_Manager(const char *ssid);
    void startAPMode();

private:
    const char *_ssid;
};

/**
 * @class EMAFilter
 * @brief Template untuk filter Exponential Moving Average.
 */
template <typename T>
class EMAFilter
{
public:
    EMAFilter(float alpha);
    T update(T newValue);
    T getValue() const;
    void reset();

private:
    float _alpha;
    T _filteredValue;
    bool _isInitialized;
};

/**
 * @class BatteryMonitor
 * @brief Untuk memantau level tegangan baterai.
 */
class BatteryMonitor
{
public:
    BatteryMonitor(int pin);
    void begin();
    float getBatteryLevel();

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
    void playNotification();

private:
    int _pin;
};

/**
 * @class ButtonMonitor (DIHAPUS)
 * @brief Deklarasi untuk kelas ButtonMonitor telah dihapus.
 */

#endif // UTILS_H