#include <Arduino.h>
#line 1 "/home/max/Sync/Projects/HardwareProjects/LivingLarder (2)/LivingLarder2/src/main/main.ino"
#include "../Time/Time.h"
#include "../Hardware/Hardware.h"
#include "../Communication/Communication.h"

Hardware hardware;
Communication communication;
Time time;
int memoryOffset = 0;

void start(){
    hardware = Hardware(memoryOffset);
    communication = Communication();
    time = Time();
}

void loop(){
    // seconds since the start of the day
    unsigned int seconds = time.getSeconds();
    hardware.run(seconds);
    communication.run(hardware, memoryOffset); // communication will update harware settings
}
