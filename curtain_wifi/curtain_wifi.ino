#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include "CronAlarms.h"
#include <Servo.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "";        // your network SSID (name)
char pass[] = ""; // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//wifi config
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -14400, 3600);

//servo config
Servo servo1;
int servoPin = 10;

//timing config
CronId id;
unsigned long millenium_seconds_offset= 946684800;
unsigned long current_epoch;
unsigned long total_offset;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  Serial.println("Starting setup...");

  //clock startup
  timeClient.begin();
  timeClient.update();
  current_epoch = timeClient.getEpochTime();
  total_offset = current_epoch - millenium_seconds_offset;
  
  setCronJob(6,40,0);
  Serial.println("Alarm Set------");
  //set a 24 hour interation of that cron job

  // create the alarms, to trigger at specific times
  //Cron.create("seconds minutes hours dayOfMonth month dayOfWeek", MorningAlarm, true)
  //DAYS_ARR[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
  // Cron.create("5 * * * * 6", MorningAlarm, true);  // 8:30am every day
  //tuesday = 6??
}

void loop() {
  // time_t now = time(nullptr)+total_offset;  
  #ifdef __AVR__
  system_tick(); 
  #endif
  Cron.delay();
  delay(1000);
}

void setCronJob(int target_hour, int target_minute, int target_second) {
  time_t now = time(nullptr)+total_offset;
  int now_hour = (now/3600)%24;
  int now_minute = (now%3600)/60;
  int now_second = (now%3600)%60;
  double now_day_seconds = convertToSeconds(now_hour, now_minute, now_second);
  double target = convertToSeconds(target_hour, target_minute, target_second);

  double time_delta;
  if (target > now_day_seconds){
    time_delta = target-now_day_seconds;
  } else if (target == now_day_seconds){
    time_delta = 0;
  } else if (target < now_day_seconds){
    time_delta = target + 86400 - now_day_seconds;
  }

  time_delta = time_delta;

  int calc_target_hour = fmod((time_delta/3600),24);
  int calc_target_minute = fmod(time_delta,3600)/60;
  int calc_target_second = fmod(fmod(time_delta,3600),60);

  String cron_str = String(calc_target_second);
  cron_str.concat(" ");
  cron_str.concat(String(calc_target_minute));
  cron_str.concat(" ");
  cron_str.concat(String(calc_target_hour));
  cron_str.concat(" * * *");

  //convert to char array
  char arr[cron_str.length() + 1]; 
  strcpy(arr, cron_str.c_str()); 
  Serial.println(cron_str);
  Cron.create(arr, MorningAlarm, false);
}

double convertToSeconds(int hours, int minutes, int seconds){
    double hours_seconds = hours * 3600L;
    double minutes_seconds = minutes * 60;
    double total = hours_seconds + minutes_seconds + seconds;
    return total;
}

void MorningAlarm() {
    servo1.attach(servoPin);
    servo1.write(90);
    delay(10);
    servo1.write(0);
    delay(7000);
    // servo1.write(180);
    // delay(3000);
    servo1.write(90);
    delay(500);
    servo1.detach();
}
