#include <Arduino.h>

int sensorPin = A0;
int val = 0;

void setup() {
    Serial.begin(115200);
    pinMode(sensorPin, INPUT);
    analogSetAttenuation(ADC_11db);
}

void loop() {
    val = analogRead(sensorPin);
    Serial.println(val);
    delay(200);
}