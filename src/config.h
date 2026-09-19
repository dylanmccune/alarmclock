#ifndef CONFIG_H
#define CONFIG_H

const char* ntpServer = "pool.ntp.org";
int sensorPin = A0;

// Timezone
// Sets it to PST with daylight savings time. 
// See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for the full lisst
const char* TZ = "PST8PDT,M3.2.0,M11.1.0";
const int START_HOUR   = 07;
const int START_MINUTE = 00;
const int START_SECOND = 00;

const int ALARM_TIMEOUT_MIN = 10; // Alarm timeout in minutes. Must be less than 60

const int TRIGGER_THRESHOLD = 1500;
// Photoresistor trigger threshold (0-4095). Lower values make it more sensitive.

#endif