#include "Jaringan.h"
#include "Config/AppConfig.h"
#include "Penyimpanan/Penyimpanan.h"
#include "server_index.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Membuat objek server
AsyncWebServer server(80);

void initJaringan()
{
    Serial.print("Membuat Access Point bernama: ");
    Serial.println(WIFI_SSID);

    // Memulai WiFi dalam mode Access Point
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Alamat IP AP: ");
    Serial.println(myIP);

    // Handler untuk halaman utama
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    // Handler untuk menyediakan data pengaturan saat ini dalam format JSON
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        UserSettings settings = loadSettings();
        JsonDocument doc;
        doc["patientName"] = settings.patientName;
        doc["dose"] = settings.dose;
        JsonArray schedules = doc.createNestedArray("schedules");
        for(int i=0; i<3; i++){
            JsonObject sched = schedules.add<JsonObject>();
            sched["hour"] = settings.schedules[i].hour;
            sched["minute"] = settings.schedules[i].minute;
            sched["enabled"] = settings.schedules[i].enabled;
        }
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json); });

    // Handler untuk menerima data yang diperbarui dari form
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        UserSettings settings;
        
        if(request->hasParam("patientName", true))
            strlcpy(settings.patientName, request->getParam("patientName", true)->value().c_str(), sizeof(settings.patientName));
        
        if(request->hasParam("dose", true))
            settings.dose = request->getParam("dose", true)->value().toInt();

        for(int i=0; i<3; i++){
            String timeName = "sched" + String(i+1) + "_time";
            String enabledName = "sched" + String(i+1) + "_enabled";
            
            settings.schedules[i].enabled = request->hasParam(enabledName.c_str(), true);
            
            if(request->hasParam(timeName.c_str(), true)){
                String timeValue = request->getParam(timeName.c_str(), true)->value();
                settings.schedules[i].hour = timeValue.substring(0,2).toInt();
                settings.schedules[i].minute = timeValue.substring(3,5).toInt();
            }
        }
        
        saveSettings(settings);
        request->send(200, "text/plain", "OK"); });

    // Handler untuk mereset perangkat
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        request->send(200, "text/plain", "Perangkat akan di-reset.");
        delay(1000);
        ESP.restart(); });

    // Memulai server
    server.begin();
}
