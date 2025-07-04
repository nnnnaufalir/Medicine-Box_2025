#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Adafruit_FRAM_I2C.h>
#include "ConfigData.h" // Menggunakan struktur data yang sudah kita buat

class EEPROM_Manager
{
public:
    EEPROM_Manager();
    bool begin();
    void loadConfig(ConfigData &config);       // Memuat konfigurasi dari EEPROM
    void saveConfig(const ConfigData &config); // Menyimpan konfigurasi ke EEPROM

private:
    Adafruit_FRAM_I2C _eeprom;
};

#endif // EEPROM_MANAGER_H