#include "Arduino.h"
#include <Wire.h>  // for I2C with RTC module
// TODO add connection to RTC
// REF https://circuitdigest.com/microcontroller-projects/esp32-real-time-clock-using-ds3231-module

class Time{
    public:
    Time();
    unsigned int getSeconds();
}