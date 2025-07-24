#include "Waktu.h"
#include "Config/AppConfig.h"
#include <Wire.h>

// Membuat objek RTC yang akan digunakan di dalam modul ini
RTC_DS3231 rtc;

void initWaktu()
{
    // Memulai komunikasi I2C dengan pin yang ditentukan di AppConfig.h
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (!rtc.begin())
    {
        Serial.println("Kesalahan: Modul RTC tidak ditemukan!");
        // Di sistem final, kita mungkin ingin ada indikasi error lain,
        // seperti LED berkedip atau pesan di layar.
    }

    // Jika RTC kehilangan daya (misal: baterai koin habis),
    // atur waktunya ke waktu saat kode ini di-compile.
    if (rtc.lostPower())
    {
        Serial.println("RTC kehilangan daya, mengatur waktu ke waktu kompilasi.");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}

DateTime getCurrentTime()
{
    return rtc.now();
}
