#include "../Time/Time.h"
#include "../Hardware/Hardware.h"

Hardware hardware;
Time time;
int memoryOffset = 0;

void start(){
    hardware = Hardware(memoryOffset);
    time = Time();
}

void loop(){
    // seconds since the start of the day
    long int seconds = time.getSeconds();
    hardware.run(seconds);
}