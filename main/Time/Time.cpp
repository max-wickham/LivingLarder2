#include "Time.h"
#include "Arduino.h"
#include <Wire.h>  // for I2C with RTC module
#include "RTClib.h"       //to show time


Time::Time(){
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
    }
}

unsigned int Time::getSeconds(){
    DateTime now = rtc.now();
    return (now.second() + now.minute() * 60 + now.hour() * 60 * 60);
}