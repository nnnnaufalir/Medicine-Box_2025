#include <Wire.h>
#include "RTClib.h"
#include "AppConfig.h" // We still use this for constants

// Create an RTC object
RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.println("RTC Test Initialized...");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC module!");
    Serial.flush();
    while (1) delay(10);
  }

  // --- MANUAL TIME SET: FOLLOW THESE STEPS ---
  // 1. Uncomment the line below.
  // 2. Change the values to the current date and time (a minute or two in the future).
  //    Format: DateTime(YYYY, M, D, H, M, S)
  // 3. Upload the code ONCE.
  // 4. AFTER successful upload, COMMENT the line again and RE-UPLOAD.
  //    This prevents the time from being reset every time the ESP32 reboots.

  // rtc.adjust(DateTime(2025, 7, 15, 2, 40, 0)); // EXAMPLE: 15 July 2024, 14:30:00

  // The automatic time set is a good fallback for when the battery is replaced.
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time to compile time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop () {
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    if(now.minute() < 10) Serial.print('0');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    if(now.second() < 10) Serial.print('0');
    Serial.print(now.second(), DEC);
    Serial.println();

    delay(1000);
}
