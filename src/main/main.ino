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