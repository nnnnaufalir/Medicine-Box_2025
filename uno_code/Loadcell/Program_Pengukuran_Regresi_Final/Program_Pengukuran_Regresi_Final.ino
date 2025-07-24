#include "HX711.h"

HX711 scale;

// --- Kelas Filter EMA ---
class EMAFilter {
private:
    float alpha, last_ema;
    bool has_run;
public:
    EMAFilter(float a) : alpha(a), last_ema(0.0f), has_run(false) {}
    float filter(float val) {
        if (!has_run) { last_ema = val; has_run = true; }
        last_ema = (alpha * val) + ((1.0 - alpha) * last_ema);
        return last_ema;
    }
};

// --- Parameter Sistem ---
EMAFilter raw_filter(0.16);

// --- HASIL KALIBRASI REGRESI ANDA ---
// Ini adalah "peta" asli kita, jangan diubah.
const long CALIBRATED_OFFSET = 295049;
const float RAW_PER_OBAT_FINAL = 5674.9313f;

// [LOGIKA BARU] Variabel untuk menyimpan offset yang disesuaikan saat runtime
long dynamic_offset = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("Inisialisasi Sistem Pengukuran Regresi (Adaptif)...");
    scale.begin(13, 14);

    // [LOGIKA BARU] Kalibrasi Ulang Titik Nol Otomatis
    Serial.println("Menyesuaikan titik nol berdasarkan kondisi saat ini...");
    long current_zero_raw = scale.read_average(20);
    dynamic_offset = current_zero_raw; // Offset kita untuk sesi ini adalah pembacaan saat ini
    
    Serial.print("Offset Kalibrasi Asli: "); Serial.println(CALIBRATED_OFFSET);
    Serial.print("Offset Saat Ini (Dinamis): "); Serial.println(dynamic_offset);
    Serial.println("Sistem siap.");
}

void loop() {
    long raw_reading = scale.read();
    float filtered_raw = raw_filter.filter(raw_reading);

    // [PERUBAHAN] Gunakan dynamic_offset, bukan CALIBRATED_OFFSET
    float pill_count_float = (filtered_raw - dynamic_offset) / RAW_PER_OBAT_FINAL;

    int pill_count = round(pill_count_float);

    Serial.print("Jumlah Obat (Bulat): ");
    Serial.print(pill_count);
    Serial.print(" | Perkiraan: ");
    Serial.println(pill_count_float, 2);
    
    delay(1);
}
