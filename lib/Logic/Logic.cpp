#include <Arduino.h>
#include "Logic.h"
#include "Config/AppConfig.h"
#include "Waktu/Waktu.h"
#include "Penyimpanan/Penyimpanan.h"

// --- Deklarasi Variabel Global dari main.cpp ---
extern volatile int g_pill_count;
extern volatile bool g_isSystemStable;
extern volatile bool g_isFrozen;
extern volatile float g_battery_voltage;
extern volatile bool g_alarm_triggered;
extern volatile int g_dose_to_take;

// --- Variabel Internal Modul Logika ---
static unsigned long last_battery_check_time = 0;
static unsigned long last_buzzer_toggle_time = 0;
static int count_sebelum_minum = 0;

// --- Deklarasi Fungsi Internal ---
static float getBatteryVoltage();
static void checkAlarms();
static void handleAlarmState();

/**
 * @brief Fungsi utama yang dipanggil oleh logicTask.
 */
void loopLogic()
{
    // Periksa jadwal alarm
    checkAlarms();

    // Tangani status alarm (jika aktif)
    handleAlarmState();

    // Perbarui voltase baterai setiap 30 detik
    if (millis() - last_battery_check_time > 30000)
    {
        g_battery_voltage = getBatteryVoltage();
        last_battery_check_time = millis();
    }
}

/**
 * @brief Membaca dan menghitung voltase baterai dari pin ADC.
 * Diadaptasi dari 8_Test_Buzzer_Battery.ino
 */
static float getBatteryVoltage()
{
    int adc_raw_value = analogRead(BATTERY_MONITOR_PIN);
    float adc_voltage = (adc_raw_value / (float)ADC_RESOLUTION) * ADC_REF_VOLTAGE;
    float battery_voltage = adc_voltage * ((R1 + R2) / R2);
    return battery_voltage;
}

/**
 * @brief Memeriksa waktu saat ini terhadap jadwal yang tersimpan di EEPROM.
 */
static void checkAlarms()
{
    // Jangan periksa alarm baru jika alarm sebelumnya masih aktif
    if (g_alarm_triggered)
    {
        return;
    }

    DateTime now = getCurrentTime();
    UserSettings settings = loadSettings();

    for (int i = 0; i < 3; i++)
    {
        if (settings.schedules[i].enabled)
        {
            if (settings.schedules[i].hour == now.hour() && settings.schedules[i].minute == now.minute() && now.second() == 0)
            {
                Serial.println("ALARM TERPICU!");
                g_alarm_triggered = true;
                g_dose_to_take = settings.dose;
                count_sebelum_minum = g_pill_count; // Simpan jumlah obat saat ini
                break;                              // Hentikan pengecekan setelah alarm pertama ditemukan
            }
        }
    }
}

/**
 * @brief Mengelola perilaku sistem saat alarm aktif.
 */
static void handleAlarmState()
{
    if (!g_alarm_triggered)
    {
        return;
    }

    // 1. Bunyikan buzzer secara berkala (setiap 2 detik)
    if (millis() - last_buzzer_toggle_time > 2000)
    {
        tone(BUZZER_PIN, 1500, 500); // Bip selama 0.5 detik
        last_buzzer_toggle_time = millis();
    }

    // 2. Periksa konfirmasi pengambilan obat
    // Hanya periksa jika sistem tidak beku dan stabil
    if (!g_isFrozen && g_isSystemStable)
    {
        // Logika konfirmasi: jumlah saat ini harus lebih kecil atau sama dengan
        // jumlah sebelum dikurangi dosis.
        if (g_pill_count <= (count_sebelum_minum - g_dose_to_take))
        {
            Serial.println("Pengambilan obat terkonfirmasi. Mematikan alarm.");
            g_alarm_triggered = false;
        }
    }
}
