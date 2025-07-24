#include "myHX711.h"
#include "KalmanFilter.h"

myHX711 scale;
KalmanFilter kalman_filter(0.1, 15.0);

long bulatkanKeRatusan(long angka) {
  // 1. Bagi dengan 100.0 untuk mendapatkan nilai desimal
  float nilaiSementara = angka / 10.0;

  // 2. Gunakan round() untuk membulatkan ke bilangan bulat terdekat
  long hasilBulat = round(nilaiSementara);

  // 3. Kembalikan ke skala ratusan
  return hasilBulat * 10;
}

void tare() {
  float sum;
  for (int i = 0; i < 50; i++) {
    long raw_value = scale.read();
    scale.filter_read(raw_value);
    float offset_value = scale.read_average(20);
    sum += offset_value;
    Serial.println(offset_value);
    Serial.println(i);
    delay(1);
  }

  scale.set_offset(sum/50);
  delay(1);
}

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);
  Serial.println("Taring.....");
  // scale.set_scale(8793.00f);
  scale.set_scale();
  tare();
  Serial.println("Taring Done!");
  delay(2000);
}

void loop() {
  long raw_value = scale.read();
  float raw_filter = kalman_filter.update(raw_value);
  float filter_value = scale.filter_read(raw_filter);
  float final_weight = scale.get_units(10);

  Serial.println("====================================");
  Serial.print("Offset: ");
  Serial.println(scale.get_offset());
  Serial.print("Mentah: ");
  Serial.println(raw_value);
  Serial.print("Mentah Filter: ");
  Serial.println(raw_filter);
  Serial.print("Filter: ");
  Serial.println(filter_value);
  Serial.print("Weight: ");
  Serial.print(final_weight);
  Serial.println("");
}
