// --- Definisi Pin & Konstanta (tanpa AppConfig.h) ---
const int BUZZER_PIN = 32;
const int BATTERY_MONITOR_PIN = 34;  // ADC1_CH6, input only
const long SERIAL_BAUD_RATE = 115200;

// --- Konfigurasi Monitor Baterai ---
// ESP32 ADC memiliki resolusi 12-bit (0-4095) dan tegangan referensi ~3.3V.
// Sesuaikan nilai R1 dan R2 sesuai dengan rangkaian voltage divider Anda.
// Contoh umum untuk baterai LiPo (maks 4.2V): R1=100k, R2=100k
// Vout = Vin * (R2 / (R1 + R2))
// Vin = Vout * ((R1 + R2) / R2)
const float R1 = 1000.0;  // Resistor dari Vin ke pin ADC
const float R2 = 3300.0;  // Resistor dari pin ADC ke GND
const float ADC_REF_VOLTAGE = 3.223;
const int ADC_RESOLUTION = 4095;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  // Konfigurasi pin buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Pin ADC tidak perlu pinMode

  Serial.println("--- Uji Coba Buzzer & Monitor Baterai ---");
}

void loop() {
  // --- Uji Buzzer ---
  Serial.println("Mengaktifkan buzzer...");
  // tone(BUZZER_PIN, 1500, 250);  // Frekuensi 1500Hz, durasi 250ms

  // --- Uji Monitor Baterai ---
  // Baca nilai mentah dari ADC
  int adc_raw_value = analogRead(BATTERY_MONITOR_PIN);

  // Konversi nilai ADC ke tegangan pada pin
  float adc_voltage = (adc_raw_value / (float)ADC_RESOLUTION) * ADC_REF_VOLTAGE;

  // Hitung kembali tegangan baterai asli menggunakan rumus voltage divider
  float battery_voltage = adc_voltage * ((R1 + R2) / R2);

  // Tampilkan hasil
  Serial.print("  Nilai Mentah ADC: ");
  Serial.print(adc_raw_value);
  Serial.print("  |  Tegangan Baterai (Perkiraan): ");
  Serial.print(battery_voltage, 2);
  Serial.println(" V");

  delay(5000);  // Tunggu 5 detik sebelum pengujian berikutnya
}
