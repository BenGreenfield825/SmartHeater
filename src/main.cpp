#include <WiFi.h>
#include <ESP32Servo.h>
#include "time.h"
#include "..\include\WifiCredentials.h"

#define uS_TO_S_FACTOR 1000000LL /* Conversion factor for micro seconds to seconds */

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
bool sleepFlag = false;
int timeToSleep = 0;

Servo servoMotor;
int servoPin = 15;
int servoPos = 0;

int switchPin = 25;
int switchVal;

void getTime()
{
  // TODO: Sometimes this fails! Make a better error handling/show user this failed. Maybe wiggle the servo arm?
  while (!getLocalTime(&currentTime))
  {
    Serial.println("Failed to obtain time, trying again");
    // Wiggle the arm so we know it failed
    servoMotor.attach(servoPin, 500, 2400);
    for (int pos = 0; pos <= 60; pos += 1)
    {
      servoMotor.write(pos);
      delay(10);
    }
    for (int pos = 60; pos >= 0; pos -= 1)
    {
      servoMotor.write(pos);
      delay(10);
    }
    servoMotor.detach(); // detach after use in case of electrical buzzing
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
  else
    return false;
}

void sleepyTime()
{
  Serial.println("------------------------------------------");
  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  Serial.println("Going to sleep...");
  Serial.println("------------------------------------------");
  esp_light_sleep_start();
  Serial.println("Waking up from sleep...");
  Serial.println("------------------------------------------");
}

void setTimeOption()
{
  switchVal = digitalRead(switchPin);
  if (switchVal)
  {
    activationTime.tm_hour = timeOne.tm_hour;
    activationTime.tm_min = timeOne.tm_min;
  }
  else
  {
    activationTime.tm_hour = timeTwo.tm_hour;
    activationTime.tm_min = timeTwo.tm_min;
  }
  Serial.println("------------------------------------------");
  Serial.print("Using time option: ");
  Serial.println(switchVal);
  Serial.println("Activation time is set to: " + String(activationTime.tm_hour) + ":" + String(activationTime.tm_min));
}

double getTimeDiff()
{
  tm tempActTime;
  tm tempCurTime;

  //basically we just need to set the times up to a default place so we can get an accurate difference of seconds
  //without needing the day or month

  //Update: okay this actually still kinda sucks for an accurate amount of time to sleep for but whatever man
  tempCurTime = currentTime;
  tempCurTime.tm_mon = 0; tempCurTime.tm_mday = 1;

  tempActTime = tempCurTime;
  tempActTime.tm_hour = activationTime.tm_hour; tempActTime.tm_min = activationTime.tm_min;

  return (difftime(mktime(&tempActTime), mktime(&tempCurTime)));
}

void setup()
{
  //------- Set what time you want the heater to activate here -------//
  // Time option 1
  timeOne.tm_hour = 7;
  timeOne.tm_min = 0;
  // Time option 2
  timeTwo.tm_hour = 17;
  timeTwo.tm_min = 55;
  //------------------------------------------------------------------//

  Serial.begin(115200);
  servoMotor.setPeriodHertz(50);
  servoMotor.attach(servoPin, 500, 2400);
  servoMotor.write(0); // Make sure we start at 0 deg
  delay(1000);
  servoMotor.detach(); // detach after use in case of electrical buzzing
  pinMode(switchPin, INPUT_PULLUP);
  setTimeOption();
  startWifiandTime();
}

void loop()
{
  double timeDiff = getTimeDiff();
  Serial.println("------------------------------------------");
  Serial.println("Activation time is " + String(timeDiff) + " seconds away");
  if (timeDiff > 3600) // check if more than an hour away
  {
    sleepFlag = true;
    timeToSleep = timeDiff - 3600;
    Serial.println("Setting sleep time to " + String(timeToSleep) + " seconds");
  }
  else if (timeDiff < 0)
  {
    Serial.println("Negative time - add 24hrs");
    timeDiff += 86400;
    Serial.println("Activation time is " + String(timeDiff) + " seconds away");
    sleepFlag = true;
    timeToSleep = timeDiff - 3600;
    Serial.println("Setting sleep time to " + String(timeToSleep) + " seconds");
  }
  else
  {
    sleepFlag = false;
    Serial.println("Sleep not needed - continuing");
  }
  Serial.println("------------------------------------------");

  getTime();
  if (checkTime())
  {
    servoMotor.attach(servoPin, 500, 2400);
    for (int pos = 0; pos <= 180; pos += 1)
    {
      servoMotor.write(pos);
      delay(10);
    }
    for (int pos = 180; pos >= 0; pos -= 1)
    {
      servoMotor.write(pos);
      delay(10);
    }
    servoMotor.detach(); // detach after use in case of electrical buzzing
    delay(65000);        // delay for 65 seconds so that we don't trigger again
  }

  if (sleepFlag)
  {
    sleepyTime(); // go to sleep
    sleepFlag = false;
  }

  delay(1000);
}
