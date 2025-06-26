#ifndef RTC_MODULE_H
#define RTC_MODULE_H

#include <RTClib.h>
// #include "Adafruit_EEPROM_I2C.h" // Tidak kita perlukan untuk sekarang

class RTC_Module
{
public:
    RTC_Module();
    bool begin();
    DateTime now();
    String getFormattedTime(bool withSeconds = true);
    void adjust(const DateTime &dt);

    // Fungsi EEPROM kita nonaktifkan sementara
    // void writeFloat(uint16_t address, float value);
    // float readFloat(uint16_t address);

private:
    RTC_DS3231 _rtc;
    // Adafruit_EEPROM_I2C _eeprom; // Nonaktifkan
};

#endif // RTC_MODULE_H
