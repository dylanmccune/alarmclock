#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <SpotifyEsp32.h>
#include <Preferences.h>
#include "secrets.h"
#include "config.h"
#include "time.h"

WebServer server(80);
AutoConnect portal(server);

int val = 0;
bool alarmRunning = false;

int lastAlarmStart;

void webhook(String url);
Preferences preferences;
Spotify *sp = nullptr;

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
}

void stopAlarm()
{
  Serial.println("Stopping alarm");
  sp->pause_playback();
  alarmRunning = false;
}

void webhook(String url)
{
  Serial.println("Webhook:");

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
      Serial.println(http.getString());
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
  if (DEBUG == true)
  {
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
  JsonDocument currentSong = sp->get_currently_playing_track().reply;
  if (currentSong["is_playing"] == true)
  {
    return true;
  }
  return false;
}
void setup()
{
  Serial.begin(115200);
  preferences.begin("alarmclock", false);
  String refreshToken = preferences.getString("refresh_token", "");
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

  if (!refreshToken.equals(""))
  {
    sp = new Spotify(SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET, refreshToken.c_str());
  }
  else
  {
    sp = new Spotify(SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET);
  }

  if (DEBUG)
  {
    sp->set_log_level(SPOTIFY_LOG_DEBUG);
  }

  sp->begin();

  while (!sp->is_auth())
  {
    sp->handle_client();
  }

  if (sp->has_access_token())
  {
    Serial.println("Saving access token to preferences");
    refreshToken = sp->get_user_tokens().refresh_token;
    preferences.putString("refresh_token", refreshToken);
  }
  else
  {
    Serial.println("Spotify authentication completed, but access-token acquisition failed.");
    while (true)
    {
      delay(1000);
    }
  }

  Serial.printf("Authenticated! Refresh token: %s\n", sp->get_user_tokens().refresh_token);
  Serial.println("Has auth: " + String(sp->is_auth()));
  Serial.println("Has access token:" + String(sp->has_access_token()));
  Serial.println(musicPlaying());
}

void loop()
{
  portal.handleClient();

  if (shouldStartAlarm())
  {
    startAlarm();
    delay(3000);
  }

  if (shouldStopAlarm())
  {
    stopAlarm();
  }

  if (alarmRunning && !musicPlaying())
  {
    Serial.println("Oooh someone stopped the music! DON'T DO THAT!");
    sp->start_a_users_playback();
    delay(1000);
  }

  delay(100);
}