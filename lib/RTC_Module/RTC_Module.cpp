#include "RTC_Module.h"

RTC_Module::RTC_Module() {}

bool RTC_Module::begin()
{
    // Hanya inisialisasi RTC
    if (!_rtc.begin())
    {
        Serial.println("Kesalahan: RTC tidak ditemukan!");
        return false;
    }

    if (_rtc.lostPower())
    {
        Serial.println("Info: RTC kehilangan daya, waktu diatur.");
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    return true; // Selalu kembalikan true jika RTC ditemukan
}

DateTime RTC_Module::now()
{
    return _rtc.now();
}

void RTC_Module::adjust(const DateTime &dt)
{
    _rtc.adjust(dt);
}

String RTC_Module::getFormattedTime(bool withSeconds)
{
    DateTime now = _rtc.now();
    char buf[10];
    if (withSeconds)
    {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    }
    else
    {
        snprintf(buf, sizeof(buf), "%02d:%02d", now.hour(), now.minute());
    }
    return String(buf);
}
