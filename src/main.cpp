#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

int sensorPin = A0;
int val = 0;

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

    
}

void loop() {
    val = analogRead(sensorPin);
    Serial.println(val);
    delay(200);
}