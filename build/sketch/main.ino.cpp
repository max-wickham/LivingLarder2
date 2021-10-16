#include <Arduino.h>
#line 1 "/home/max/Sync/Projects/HardwareProjects/LivingLarder (2)/LivingLarder2/src/main/main.ino"
#include "../Hardware/Hardware.h"

Hardware hardware;

void start(){
    // TODO add check to see if this is the first run
    hardware = Hardware();
    
}

void loop(){
    long int seconds;
    hardware.run(seconds);
}
