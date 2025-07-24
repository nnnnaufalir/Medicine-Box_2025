#ifndef SENSOR_H
#define SENSOR_H

/**
 * @brief Menginisialisasi sensor HX711 dan melakukan kalibrasi drift awal.
 * Fungsi ini harus dipanggil sekali sebelum scheduler RTOS dimulai.
 */
void initSensor();

/**
 * @brief Menjalankan satu siklus pembacaan, pemfilteran, dan klasifikasi.
 * Fungsi ini akan dipanggil secara berulang oleh sensorTask.
 */
void loopSensor();

#endif // SENSOR_H
