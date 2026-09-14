#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "time.h"

const char* ntpServer = "pool.ntp.org";

int sensorPin = A0;
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
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Wifi connecting");
    while (WiFi.status() != WL_CONNECTED) {
         Serial.print('.');
         delay(500);
    }
    Serial.println();

    Serial.println("Setting clock...");
    configTzTime("PST8PDT,M3.2.0,M11.1.0", ntpServer);
    printLocalTime();

    Serial.println("Connected!");    

}

void loop() {
    val = analogRead(sensorPin);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        if (timeinfo.tm_hour == 7 && timeinfo.tm_min == 0 && !alarmRunning) {
            webhook(WEBHOOK_START);
            alarmRunning = true;
        }
        if (alarmRunning && (timeinfo.tm_hour > 7 || (timeinfo.tm_hour == 7 && timeinfo.tm_min >= 10))) {
            webhook(WEBHOOK_STOP);
            alarmRunning = false;
        }
    }
    if (val < 1500 && alarmRunning) {
        webhook(WEBHOOK_STOP);
        alarmRunning = false;
    }
    delay(100);
}