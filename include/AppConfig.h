#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// EMA Filter
#define EMA_ALPHA 0.3f

// KNN
#define KNN_K 3

// FreeRTOS Tasks
#define TASK_SENSOR_INTERVAL_MS 200   // Interval pembacaan sensor
#define TASK_DISPLAY_INTERVAL_MS 1000 // Interval pembaruan tampilan

#endif // APP_CONFIG_H
