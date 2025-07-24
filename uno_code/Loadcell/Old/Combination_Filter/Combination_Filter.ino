#include "HX711.h"
#include "KalmanFilterKu.h"
#include "EMAFilterKu.h"

const float current_q = 0.1;
const float current_r = 15.0;
const float alpha = 0.4;

HX711 scale;
KalmanFilter kalman_filter(current_q, current_r);
EMAFilter ema_filter(alpha); // float filtered_raw = raw_filter.filter(raw_reading);

long bulatkanKeRatusan(long angka) {
  // 1. Bagi dengan 100.0 untuk mendapatkan nilai desimal
  float nilaiSementara = angka / 100.0;
  
  // 2. Gunakan round() untuk membulatkan ke bilangan bulat terdekat
  long hasilBulat = round(nilaiSementara);
  
  // 3. Kembalikan ke skala ratusan
  return hasilBulat * 100;
}

long raw_reading;

//only
float kalman_only;
float ema_only;
float bulat_only;

//combine 2
float kalman_ema;
float kalman_bulat;
float ema_bulat;
float bulat_ema;

//combine 3
float kalman_ema_bulat;
float kalman_bulat_ema;

void setup() {
  Serial.begin(115200);
  scale.begin(13, 14);

}

void loop() {
  //raw
  raw_reading = scale.read();
  
  //kalman
  kalman_only = kalman_filter.filter(raw_reading);

  //ema
  ema_only = ema_filter.filter(raw_reading);

  //bulat
  bulat_only = bulatkanKeRatusan(raw_reading);

  //kalman + ema
  kalman_ema = ema_filter.filter(kalman_only);

  //kalman + bulat
  kalman_bulat = bulatkanKeRatusan(kalman_only);

  //ema + bulat 
  ema_bulat = bulatkanKeRatusan(ema_only);

  //bulat + ema
  bulat_ema = ema_filter.filter(bulat_only);

  // kalman + ema + bulat 
  kalman_ema_bulat = bulatkanKeRatusan(kalman_ema);

  //kalman + bulat + ema
  kalman_bulat_ema = ema_filter.filter(kalman_bulat);


  Serial.print("Raw:");
  Serial.print(raw_reading);
  Serial.print(", ");
  Serial.print("Kalman:");
  Serial.print(kalman_only);
  Serial.print(", ");
  Serial.print("EMA:");
  Serial.print(ema_only);
  Serial.print(", ");
  Serial.print("KalmanEMA:");
  Serial.print(kalman_ema);
  Serial.print(", ");
  Serial.print("KalmanBulat:");
  Serial.print(kalman_bulat);
  Serial.print(", ");
  Serial.print("EMABulat:");
  Serial.print(ema_bulat);
  Serial.print(", ");
  Serial.print("K-EMA-b:");
  Serial.print(kalman_ema_bulat);
  Serial.print(", ");
  Serial.print("K-b-EMA:");
  Serial.println(kalman_bulat_ema);

}
