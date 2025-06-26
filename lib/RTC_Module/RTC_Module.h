#ifndef RTC_MODULE_H
#define RTC_MODULE_H

#include <RTClib.h>

class RTC_Module
{
public:
    RTC_Module();
    bool begin();
    DateTime now();

private:
    RTC_DS3231 _rtc;
};

#endif // RTC_MODULE_H
