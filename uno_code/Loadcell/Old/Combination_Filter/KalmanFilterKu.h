// --- Kelas Kalman Filter Sederhana (1D) ---
class KalmanFilter {
private:
  float Q;  // Process noise covariance
  float R;  // Measurement noise covariance
  float x;  // Estimated value
  float p;  // Estimation error covariance
  float k;  // Kalman gain

public:
  KalmanFilter(float process_noise, float measurement_noise, float initial_value = 0) {
    Q = process_noise;
    R = measurement_noise;
    x = initial_value;
    p = 1.0;
  }

  float filter(float measurement) {
    p = p + Q;
    k = p / (p + R);
    x = x + k * (measurement - x);
    p = (1 - k) * p;
    return x;
  }
};