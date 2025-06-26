#include "RTC_Module.h"

RTC_Module::RTC_Module() {}

bool RTC_Module::begin()
{
    if (!_rtc.begin())
    {
        Serial.println("Kesalahan: RTC tidak ditemukan!");
        return false;
    }

    // Atur waktu ke waktu kompilasi jika RTC kehilangan daya
    if (_rtc.lostPower())
    {
        Serial.println("Info: RTC kehilangan daya, waktu diatur.");
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    return true;
}

DateTime RTC_Module::now()
{
    return _rtc.now();
}
