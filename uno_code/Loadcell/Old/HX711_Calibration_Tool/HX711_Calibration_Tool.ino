#include <HX711.h>

HX711 scale;

// We start with your known factor.
// float calibration_factor = 8410.82f;
float calibration_factor = 1320.8003f; // offset 1. 288143, 2.xxxxxx, scale 3010.8203, scale 

void printInstructions() {
  Serial.println("\n--- Alat Kalibrasi Loadcell Lanjutan ---");
  Serial.println("Tujuan: Sesuaikan 'Faktor Skala' hingga 'Berat (g)' cocok dengan berat referensi Anda.");
  Serial.println("\nPerintah (kirim lalu Enter):");
  Serial.println("  t = Tare (Shortcut untuk mengatur Offset ke nilai mentah saat ini)");
  Serial.println("  --- Penyesuaian Faktor Skala (SCALE) ---");
  Serial.println("  a/s/d/f = Tambah 1000/100/10/1 | g/h/j = Tambah 0.1/0.01/0.001");
  Serial.println("  z/x/c/v = Kurangi 1000/100/10/1 | b/n/m = Kurangi 0.1/0.01/0.001");
  Serial.println("  --- Penyesuaian Manual Offset (untuk expert) ---");
  Serial.println("  q/w/e/r = Tambah Offset 1000/100/10/1");
  Serial.println("  y/u/i/o = Kurangi Offset 1000/100/10/1");
  Serial.println("----------------------------------------------------------------");
}

void setup() {
 Serial.begin(115200);
  scale.begin(13, 14);

  printInstructions();

  // Apply the initial scale factor
  scale.set_scale(calibration_factor);
  
  // Perform an initial tare to set a sensible starting offset
  Serial.println("Melakukan Tare awal... Pastikan tidak ada beban di atas timbangan.");
  scale.tare(10);
  Serial.println("Skala siap.");
}

void loop() {
  // Handle user input for adjustments
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      // --- Control Commands ---
      case 't':
        scale.tare(20);
        Serial.println("\n>>> Perintah TARE diterima. Offset diatur ulang.");
        break;
        
      // --- SCALE Factor Adjustments ---
      case 'a': calibration_factor += 1000; break; case 's': calibration_factor += 100; break;
      case 'd': calibration_factor += 10; break;   case 'f': calibration_factor += 1; break;
      case 'g': calibration_factor += 0.1f; break;  case 'h': calibration_factor += 0.01f; break;
      case 'j': calibration_factor += 0.001f; break;
      case 'z': calibration_factor -= 1000; break; case 'x': calibration_factor -= 100; break;
      case 'c': calibration_factor -= 10; break;   case 'v': calibration_factor -= 1; break;
      case 'b': calibration_factor -= 0.1f; break;  case 'n': calibration_factor -= 0.01f; break;
      case 'm': calibration_factor -= 0.001f; break;

      // --- Manual OFFSET Adjustments ---
      case 'q': scale.set_offset(scale.get_offset() + 1000); break;
      case 'w': scale.set_offset(scale.get_offset() + 100); break;
      case 'e': scale.set_offset(scale.get_offset() + 10); break;
      case 'r': scale.set_offset(scale.get_offset() + 1); break;
      case 'y': scale.set_offset(scale.get_offset() - 1000); break;
      case 'u': scale.set_offset(scale.get_offset() - 100); break;
      case 'i': scale.set_offset(scale.get_offset() - 10); break;
      case 'o': scale.set_offset(scale.get_offset() - 1); break;
    }
    // Apply the new scale factor immediately after any change
    scale.set_scale(calibration_factor);
  }

  // --- Display Detailed Diagnostic Information ---
  long raw_value = scale.read_average(10);
  long offset_value = scale.get_offset();
  float scale_factor = scale.get_scale();
  float final_weight = (float)(raw_value - offset_value) / scale_factor;
  float final_weightv2 = scale.get_units(10);

  Serial.println("---");
  Serial.print("1. Nilai Mentah (Raw)    : "); Serial.println(raw_value);
  Serial.print("2. Offset                : "); Serial.println(offset_value);
  Serial.print("3. Nilai Setelah Offset  : "); Serial.println(raw_value - offset_value);
  Serial.print("4. Faktor Skala (SCALE)  : "); Serial.println(scale_factor, 4);
  Serial.print("5. Variabel Faktor Skala : "); Serial.println(calibration_factor, 4);
  Serial.println("   ========================================");
  Serial.print("   Berat (g)              : "); Serial.println(final_weight, 2);
  Serial.print("   Berat v2 (g)           : "); Serial.println(final_weightv2, 2);
  
  delay(20);
}
