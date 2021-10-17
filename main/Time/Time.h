#include "Arduino.h"
// REF https://circuitdigest.com/microcontroller-projects/esp32-real-time-clock-using-ds3231-module

class Time{
    public:
    Time();
    unsigned int getSeconds();
}