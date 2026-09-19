#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "config.h"
#include "time.h"


int val = 0;
bool alarmRunning = false;

void webhook(String id) {

  HTTPClient http;

  http.begin(String(HA_IP) + "/api/webhook/" + id);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST("");

  // httpCode will be negative on error
  if (httpCode > 0) {
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("200 OK");
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    pinMode(sensorPin, INPUT);
    analogSetAttenuation(ADC_11db);

    if (ALARM_TIMEOUT_MIN > 60) {
          Serial.println("Alarm timeout cannot be over an hour. Please change ALARM_TIMEOUT_MIN in config.h");
          while (true) {
              delay(1000);
          }
        }
        
    if (START_HOUR > 23 || START_HOUR < 0 || START_MINUTE > 59 || START_MINUTE < 0 || START_SECOND > 59 || START_SECOND < 0) {
        Serial.println("Invalid alarm start time.");
        while (true) {
            delay(1000);
        }
      }
    
      if (TRIGGER_THRESHOLD < 0 || TRIGGER_THRESHOLD > 4095) {
        Serial.println("Invalid treshold. Must be between 0 and 4095.");
        while (true) {
          delay(1000);
        }
      }
    
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Wifi connecting");
    while (WiFi.status() != WL_CONNECTED) {
         Serial.print('.');
         delay(500);
    }
    Serial.println();

    Serial.println("Setting clock...");
    configTzTime(TZ, ntpServer);
    printLocalTime();

    Serial.println("Connected!");    

}

void loop() {
    val = analogRead(sensorPin);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        if (timeinfo.tm_hour == START_HOUR && timeinfo.tm_min == START_MINUTE && timeinfo.tm_sec == START_SECOND && !alarmRunning) {
            alarmRunning = true;
            webhook(WEBHOOK_START);
            Serial.println("Alarm started");
            Serial.println(val);
            
        }
        if (alarmRunning && (timeinfo.tm_hour > START_HOUR || (timeinfo.tm_hour == START_HOUR && timeinfo.tm_min >= START_MINUTE + ALARM_TIMEOUT_MIN))) {
            webhook(WEBHOOK_STOP);
            alarmRunning = false;
        }
    }
    if (val < TRIGGER_THRESHOLD && alarmRunning) {
        webhook(WEBHOOK_STOP);
        alarmRunning = false;
    }
    delay(100);
}