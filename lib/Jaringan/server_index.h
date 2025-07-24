#ifndef SERVER_INDEX_H
#define SERVER_INDEX_H

#include <Arduino.h>

// Menyimpan halaman HTML di dalam Flash memory (PROGMEM) untuk menghemat RAM.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Konfigurasi Kotak Obat</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 20px; color: #333; }
    .container { max-width: 500px; margin: 0 auto; background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h3 { color: #0056b3; text-align: center; }
    .input-group { margin-bottom: 15px; }
    label { display: block; margin-bottom: 5px; font-weight: bold; }
    input[type="text"], input[type="number"], input[type="time"] {
      width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box;
    }
    .btn {
      width: 100%; padding: 10px; border: none; border-radius: 4px; color: white; font-size: 16px; cursor: pointer;
    }
    .btn-save { background-color: #28a745; }
    .btn-reset { background-color: #dc3545; margin-top: 10px; }
    .schedule-group { display: flex; align-items: center; gap: 10px; }
    .schedule-group input[type="time"] { flex-grow: 1; }
  </style>
</head>
<body>
  <div class="container">
    <h3>Konfigurasi Kotak Obat</h3>
    <form id="settingsForm">
      <div class="input-group">
        <label for="patientName">Nama Pasien:</label>
        <input type="text" id="patientName" name="patientName" maxlength="31">
      </div>
      <div class="input-group">
        <label for="dose">Dosis (butir):</label>
        <input type="number" id="dose" name="dose">
      </div>
      
      <h4>Jadwal Minum Obat</h4>
      <div class="input-group schedule-group">
        <input type="checkbox" id="sched1_enabled" name="sched1_enabled">
        <input type="time" id="sched1_time" name="sched1_time">
      </div>
      <div class="input-group schedule-group">
        <input type="checkbox" id="sched2_enabled" name="sched2_enabled">
        <input type="time" id="sched2_time" name="sched2_time">
      </div>
      <div class="input-group schedule-group">
        <input type="checkbox" id="sched3_enabled" name="sched3_enabled">
        <input type="time" id="sched3_time" name="sched3_time">
      </div>
      
      <button type="submit" class="btn btn-save">Simpan Pengaturan</button>
    </form>
    <button onclick="resetDevice()" class="btn btn-reset">Reset Perangkat</button>
  </div>

  <script>
    // Fungsi untuk memuat pengaturan saat ini dari ESP32
    window.onload = function() {
      fetch('/settings')
        .then(response => response.json())
        .then(data => {
          document.getElementById('patientName').value = data.patientName;
          document.getElementById('dose').value = data.dose;
          for (let i = 0; i < 3; i++) {
            let sched = data.schedules[i];
            document.getElementById(`sched${i+1}_enabled`).checked = sched.enabled;
            let timeStr = (sched.hour < 10 ? '0' : '') + sched.hour + ':' + (sched.minute < 10 ? '0' : '') + sched.minute;
            document.getElementById(`sched${i+1}_time`).value = timeStr;
          }
        });
    };

    // Fungsi untuk mengirim data form
    document.getElementById('settingsForm').addEventListener('submit', function(event) {
      event.preventDefault();
      const formData = new FormData(this);
      const params = new URLSearchParams();
      for (const pair of formData) {
        params.append(pair[0], pair[1]);
      }
      
      fetch('/update', {
        method: 'POST',
        body: params,
      }).then(response => {
        if(response.ok) alert('Pengaturan berhasil disimpan!');
        else alert('Gagal menyimpan pengaturan.');
      });
    });

    // Fungsi untuk mereset perangkat
    function resetDevice() {
      if (confirm('Apakah Anda yakin ingin mereset perangkat?')) {
        fetch('/reset').then(() => alert('Perangkat akan di-reset.'));
      }
    }
  </script>
</body>
</html>
)rawliteral";

#endif // SERVER_INDEX_H
