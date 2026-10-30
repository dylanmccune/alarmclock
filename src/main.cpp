#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include "secrets.h"
#include "config.h"
#include "time.h"

WebServer server(80);
AutoConnect portal(server);

int val = 0;
bool alarmRunning = false;

int lastAlarmStart;

void webhook(String url);

void startAlarm()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return;
  }
  lastAlarmStart = timeinfo.tm_yday;
  Serial.println("Starting alarm");
  alarmRunning = true;
  webhook(VOICEMONKEY_ENDPOINT);
  // either a loop here to keep checking if it's still playing or put that in some other script, oh also could be in main loop
}

void stopAlarm()
{
  Serial.println("Stopping alarm");
  alarmRunning = false;
  // spotify api
}

void webhook(String url)
{
  Serial.println("webhook");

  WiFiClient client;
  HTTPClient http;

  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST("");

  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      Serial.println("200 OK");
    }
    else
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

bool checkConfig()
{
  if (ALARM_TIMEOUT_MIN > 60)
  {
    Serial.println("Alarm timeout cannot be over an hour. Please change ALARM_TIMEOUT_MIN in config.h");
    return false;
  }

  if (START_HOUR > 23 || START_HOUR < 0 || START_MINUTE > 59 || START_MINUTE < 0 || START_SECOND > 59 || START_SECOND < 0)
  {
    Serial.println("Invalid alarm start time.");
    return false;
  }

  if (TRIGGER_THRESHOLD < 0 || TRIGGER_THRESHOLD > 4095)
  {
    Serial.println("Invalid treshold. Must be between 0 and 4095.");
    return false;
  }
  return true;
}

int secsSinceMidnight()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return -1;
  }
  return timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
}

int startSecs()
{
  return START_HOUR * 3600 + START_MINUTE * 60 + START_SECOND;
}

bool shouldStartAlarm()
{
  val = analogRead(sensorPin);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return false;
  }
  if (DEBUG == true) {
    Serial.println("Sensor value: " + String(val));
    Serial.println(alarmRunning);
    Serial.println(lastAlarmStart);
    return !alarmRunning && val <= TRIGGER_THRESHOLD && lastAlarmStart != timeinfo.tm_yday;
  }
  return secsSinceMidnight() > startSecs() && !alarmRunning && val >= TRIGGER_THRESHOLD && lastAlarmStart != timeinfo.tm_yday;
}

bool shouldStopAlarm()
{
  val = analogRead(sensorPin);
  return alarmRunning && (secsSinceMidnight() > startSecs() + ALARM_TIMEOUT_MIN * 60 || val < TRIGGER_THRESHOLD);
}

bool musicPlaying()
{
  // some sort of spotify api thing, todo
  return false;
}
void setup()
{
  Serial.begin(115200);
  delay(2000);
  pinMode(sensorPin, INPUT);
  analogSetAttenuation(ADC_11db);
  if (!checkConfig())
  {
    Serial.println("Invalid config. Please check config.h");
    while (true)
    {
      delay(1000);
    }
  }

  AutoConnectConfig config;
  portal.config(config);
  portal.begin();
  Serial.println("IP: " + WiFi.localIP().toString());

  Serial.println("Setting clock...");
  configTzTime(TZ, ntpServer);
  printLocalTime();
}

void loop()
{
  portal.handleClient();

  if (shouldStartAlarm())
  {
    startAlarm();
  }

  if (shouldStopAlarm())
  {
    stopAlarm();
  }

  delay(100);
}