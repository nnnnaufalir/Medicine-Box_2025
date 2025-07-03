#ifndef SCHEDULE_MANAGER_H
#define SCHEDULE_MANAGER_H

#include <RTClib.h>
#include <vector>

// Struktur untuk menyimpan satu data jadwal
struct MedicationSchedule
{
    uint8_t hour;
    uint8_t minute;
    bool triggered; // Penanda apakah alarm untuk jadwal ini sudah aktif
};

class Schedule_Manager
{
public:
    Schedule_Manager();

    // Menambah jadwal baru ke dalam daftar
    void addSchedule(uint8_t hour, uint8_t minute);

    // Memeriksa semua jadwal berdasarkan waktu saat ini
    // Mengembalikan true jika ada alarm yang harus berbunyi
    bool checkSchedules(const DateTime &now);

    // Mereset semua status 'triggered' (misal, untuk hari baru)
    void resetAllTriggers();

private:
    std::vector<MedicationSchedule> _schedules;
};

#endif // SCHEDULE_MANAGER_H
