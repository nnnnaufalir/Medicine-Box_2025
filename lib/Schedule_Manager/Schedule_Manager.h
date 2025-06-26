#ifndef SCHEDULE_MANAGER_H
#define SCHEDULE_MANAGER_H

#include <RTClib.h>
#include <vector>

struct MedicationSchedule
{
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    bool taken;
};

class Schedule_Manager
{
public:
    void addSchedule(uint8_t hour, uint8_t minute);
    void checkSchedules(const DateTime &now);
    void loadSchedules(); // Dari EEPROM
    void saveSchedules(); // Ke EEPROM

private:
    std::vector<MedicationSchedule> _schedules;
};

#endif // SCHEDULE_MANAGER_H