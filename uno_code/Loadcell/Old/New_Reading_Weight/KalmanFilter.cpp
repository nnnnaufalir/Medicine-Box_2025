#include "KalmanFilter.h" 

KalmanFilter::KalmanFilter(float process_noise, float measurement_noise, float initial_value) {
    Q = process_noise;
    R = measurement_noise;
    x = initial_value;
    p = 1.0; // Inisialisasi error covariance
    k = 0;
}

float KalmanFilter::update(float measurement) {
    // Tahap Prediksi
    p = p + Q;

    // Tahap Pengukuran (Koreksi)
    k = p / (p + R);
    x = x + k * (measurement - x);
    p = (1 - k) * p;

    return x;
}

void KalmanFilter::setParameters(float process_noise, float measurement_noise) {
    Q = process_noise;
    R = measurement_noise;
}
