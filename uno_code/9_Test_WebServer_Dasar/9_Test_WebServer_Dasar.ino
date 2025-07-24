#include <WiFi.h>
#include <WebServer.h>

// --- Konfigurasi Jaringan ---
const char* ssid = "KotakObat_ESP32";  // Nama jaringan Wi-Fi yang akan dibuat
const char* password = "password123";  // Kata sandi untuk jaringan Wi-Fi

// Buat objek WebServer di port 80 (port standar untuk HTTP)
WebServer server(80);

// --- Fungsi untuk menangani permintaan ke halaman utama ---
void handleRoot() {
  String html = "<html><head><title>ESP32 Web Server</title>";
  html += "<style>body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f0f0f0; }</style>";
  html += "</head><body>";
  html += "<h1>Halo dari Kotak Obat!</h1>";
  html += "<p>Web Server ESP32 aktif.</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);  // Kirim halaman HTML ke browser
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Uji Coba Web Server Dasar ---");

  // Mulai mode Access Point (AP)
  Serial.print("Membuat Access Point bernama: ");
  Serial.println(ssid);
  WiFi.softAP(ssid, password);

  // Tampilkan alamat IP dari Access Point
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Alamat IP AP: ");
  Serial.println(myIP);

  // Atur handler untuk permintaan web
  server.on("/", HTTP_GET, handleRoot);  // Jika ada permintaan ke halaman utama, panggil handleRoot
  server.onNotFound(handleNotFound);     // Jika halaman tidak ditemukan

  // Mulai server
  server.begin();
  Serial.println("Server HTTP dimulai.");
}

void loop() {
  // Terus periksa permintaan dari klien (browser)
  server.handleClient();
}
