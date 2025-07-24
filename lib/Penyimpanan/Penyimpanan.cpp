#include "Penyimpanan.h"
#include "Config/AppConfig.h"
#include <Wire.h>
#include <extEEPROM.h>

// Inisialisasi objek EEPROM
extEEPROM eeprom(kbits_32, 1, 32, EEPROM_ADDRESS);

// Alamat memori di mana kita akan menyimpan struct settings
const uint16_t SETTINGS_ADDRESS = 0;

void initPenyimpanan()
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    byte status = eeprom.begin(eeprom.twiClock100kHz);
    if (status)
    {
        Serial.print("Kesalahan: Inisialisasi extEEPROM gagal, status = ");
        Serial.println(status);
    }
}

void saveSettings(const UserSettings &settings)
{
    Serial.println("Menyimpan pengaturan ke EEPROM...");
    // [PERBAIKAN] Menggunakan eeprom.write() sesuai dengan library Anda
    byte i2cStat = eeprom.write(SETTINGS_ADDRESS, (byte *)&settings, sizeof(settings));
    if (i2cStat != 0)
    {
        Serial.print("Kesalahan saat menulis ke EEPROM, status = ");
        Serial.println(i2cStat);
    }
    else
    {
        Serial.println("Pengaturan berhasil disimpan.");
    }
}

UserSettings loadSettings()
{
    UserSettings settings;

    Serial.println("Memuat pengaturan dari EEPROM...");
    // [PERBAIKAN] Menggunakan eeprom.read() sesuai dengan library Anda
    byte i2cStat = eeprom.read(SETTINGS_ADDRESS, (byte *)&settings, sizeof(settings));
    if (i2cStat != 0)
    {
        Serial.print("Kesalahan saat membaca dari EEPROM, status = ");
        Serial.println(i2cStat);
        // Jika gagal membaca, kembalikan struct default untuk keamanan
        // (Anda bisa mengisinya dengan nilai awal yang aman di sini)
    }
    else
    {
        Serial.println("Pengaturan berhasil dimuat.");
    }

    return settings;
}
