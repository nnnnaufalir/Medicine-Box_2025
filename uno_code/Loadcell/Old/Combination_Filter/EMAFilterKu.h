// --- Kelas Filter EMA Sederhana ---
class EMAFilter {
private:
  float alpha;
  float last_ema;
  bool has_run;
public:
  EMAFilter(float alpha_val)
    : alpha(alpha_val), last_ema(0.0f), has_run(false) {}

  float filter(float new_value) {
    if (!has_run) {
      last_ema = new_value;
      has_run = true;
    }
    last_ema = (alpha * new_value) + ((1.0 - alpha) * last_ema);
    return last_ema;
  }
};