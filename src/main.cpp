#include <WiFi.h>
#include <ESP32Servo.h>
#include "time.h"
#include "..\include\WifiCredentials.h"

const char *ssid = SSID;
const char *password = PASSWORD;

const char *ntpServer = "pool.ntp.org";
// Adjust these values for your timezone and daylight saving time
const long gmtOffset_sec = -18000; // -18000 for -5.00 UTC
const int daylightOffset_sec = 3600;
struct tm currentTime;
struct tm activationTime;
struct tm timeOne;
struct tm timeTwo;

Servo servoMotor;
int servoPin = 15;
int servoPos = 0;

int switchPin = 25;
int switchVal;

void getTime()
{
  if (!getLocalTime(&currentTime))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&currentTime, "%A, %B %d %Y %H:%M:%S");
}

void startWifiandTime()
{
  // connect to WiFi
  Serial.println("------------------------------------------");
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Initial time from NTP: ");
  getTime();

  // disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("Wifi disconnected.");
  Serial.println("------------------------------------------");
}

bool checkTime()
{
  if ((currentTime.tm_hour == activationTime.tm_hour) && (currentTime.tm_min == activationTime.tm_min))
  {
    Serial.println("Activation time reached!");
    return true;
  }
  else return false;
}

void setTimeOption() {
  switchVal = digitalRead(switchPin);
  if (switchVal)
  {
    activationTime.tm_hour = timeOne.tm_hour;
    activationTime.tm_min = timeOne.tm_min;
  }
  else {
    activationTime.tm_hour = timeTwo.tm_hour;
    activationTime.tm_min = timeTwo.tm_min;
  }
  Serial.println("------------------------------------------");
  Serial.print("Using time option: ");
  Serial.println(switchVal);
}

void setup()
{
  //------- Set what time you want the heater to activate here -------//
  // Time option 1
  timeOne.tm_hour = 8;
  timeOne.tm_min = 0;
  // Time option 2
  timeTwo.tm_hour = 9;
  timeTwo.tm_min = 0;
  //------------------------------------------------------------------//

  Serial.begin(115200);
  servoMotor.setPeriodHertz(50);
  servoMotor.attach(servoPin, 500, 2400);
  servoMotor.write(0);  //Make sure we start at 0 deg
  delay(1000);
  servoMotor.detach();  //detach after use in case of electrical buzzing
  pinMode(switchPin, INPUT_PULLUP);
  setTimeOption();
  startWifiandTime();
}

void loop()
{
  //TODO: Check to see how much time is left between currentTime and activationTime. If more than 1 hour, go to sleep for <time left - 1 hour>
  //      This lets us wake up 1 hour before activation time just so that we have some wiggle room I guess idk will probably change this

  getTime();
  //TODO: figure out how to deal with it being the correct time 60 times (it checks every second, so it will be "correct" every second of that minute)
  checkTime();
  delay(1000);

  //   for (int pos = 0; pos <= 180; pos += 1) {
  //     servoMotor.write(pos);
  //     delay(10);
  //   }
  //  // Rotation from 180° to 0
  //   for (int pos = 180; pos >= 0; pos -= 1) {
  //     servoMotor.write(pos);
  //     delay(10);
  //   }
}
