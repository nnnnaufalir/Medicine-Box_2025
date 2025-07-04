#include "EEPROM_Manager.h"
#include <Wire.h>
#include <RTClib.h>

EEPROM_Manager::EEPROM_Manager() {}

bool EEPROM_Manager::begin()
{
    // Inisialisasi EEPROM di alamat I2C 0x57 (default untuk AT24C32)
    if (!_eeprom.begin(0x57))
    {
        Serial.println("Kesalahan: Chip EEPROM AT24C32 tidak ditemukan!");
        return false;
    }
    Serial.println("EEPROM Manager berhasil dimulai.");
    return true;
}

void EEPROM_Manager::loadConfig(ConfigData &config)
{
    Serial.println("Membaca konfigurasi dari EEPROM...");
    ConfigData tempConfig;

    // Baca seluruh blok data dari EEPROM ke struct sementara
    _eeprom.read(0, (uint8_t *)&tempConfig, sizeof(ConfigData));

    // Validasi dengan magic number
    if (tempConfig.magic_number == config.magic_number)
    {
        // Jika valid, salin data dari EEPROM ke config utama
        memcpy(&config, &tempConfig, sizeof(ConfigData));
        Serial.println("Konfigurasi berhasil dimuat.");
    }
    else
    {
        // Jika tidak valid, berarti ini pertama kali jalan atau data korup.
        // Simpan konfigurasi default ke EEPROM.
        Serial.println("Konfigurasi tidak valid. Menyimpan nilai default...");
        saveConfig(config);
    }
}

void EEPROM_Manager::saveConfig(const ConfigData &config)
{
    Serial.println("Menyimpan konfigurasi ke EEPROM...");
    // Tulis seluruh blok data dari config ke EEPROM
    _eeprom.write(0, (uint8_t *)&config, sizeof(ConfigData));
    Serial.println("Konfigurasi berhasil disimpan.");
}