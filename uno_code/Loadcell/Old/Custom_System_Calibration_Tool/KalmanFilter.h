#ifndef KalmanFilter_h
#define KalmanFilter_h

class KalmanFilter {
private:
    float Q; // Process noise covariance
    float R; // Measurement noise covariance
    float x; // Estimated value
    float p; // Estimation error covariance
    float k; // Kalman gain

public:
    // Constructor
    KalmanFilter(float process_noise, float measurement_noise, float initial_value = 0);

    // Fungsi utama untuk memfilter nilai baru
    float update(float measurement);

    // Fungsi untuk mengubah parameter secara real-time
    void setParameters(float process_noise, float measurement_noise);
};

#endif