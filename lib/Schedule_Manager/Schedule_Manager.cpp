#include "Schedule_Manager.h"

Schedule_Manager::Schedule_Manager() {}

void Schedule_Manager::addSchedule(uint8_t hour, uint8_t minute)
{
    _schedules.push_back({hour, minute, false});
}

bool Schedule_Manager::checkSchedules(const DateTime &now)
{
    for (auto &schedule : _schedules)
    {
        // Cek jika waktu cocok dan alarm belum pernah berbunyi hari ini
        if (now.hour() == schedule.hour && now.minute() == schedule.minute && !schedule.triggered)
        {
            schedule.triggered = true; // Tandai alarm sudah aktif
            return true;               // Kirim sinyal bahwa alarm harus berbunyi
        }
    }
    return false; // Tidak ada alarm yang cocok
}

void Schedule_Manager::resetAllTriggers()
{
    for (auto &schedule : _schedules)
    {
        schedule.triggered = false;
    }
}
