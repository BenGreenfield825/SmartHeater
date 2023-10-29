# SmartHeater

## Instructions
- Add a file named `WifiCredentials.h` under the `include` folder
    - Add two lines: 
        ```c++
        #define SSID "yourSSID"
        #define PASSWORD "yourPassword"
        ```
- Adjust `gmtOffset_sec` and `daylightOffset_sec` in `main.cpp` based on your location
- Set what time you want the heater to turn on
    - You can set two time options that you can choose using a switch (value of the switch is read during startup - move the switch and restart device to change time option)
    - Example:
        ```c++
        // Time option 1 (8:00 AM)
        timeOne.tm_hour = 8; 
        timeOne.tm_min = 0;
        // Time option 2 (9:45 AM)
        timeTwo.tm_hour = 9;
        timeTwo.tm_min = 45;
        ```
