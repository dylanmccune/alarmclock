#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

int sensorPin = A0;
int val = 0;
bool alarmRunning = true;

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

    Serial.println("Connected!");
    if (val >= 1500) {
        webhook(WEBHOOK_START);
        Serial.println("Alarm started");
    }
    alarmRunning = true;

}

void loop() {
    val = analogRead(sensorPin);
    Serial.println(val);
    if (val < 1500 && alarmRunning) {
        webhook(WEBHOOK_STOP);
        Serial.println("Alarm stopped");
        alarmRunning = false;
    }
    delay(200);
}