#include "Communication.h"
#include "../Hardware/Hardware.h"

Communication::Communication(){
    // Add support for name change
    SerialBT.begin("LivingLarder");
}

void Communication::run(Hardware &hardware, int memoryOffset){
    char message[40]; //40 character maximum message
    int messageIndex = 0;
    bool newMessage = false;
    while(SerialBT.available()) {
        char character = SerialBT.read();
        message[messageIndex++] = character;
        if(character == eot){
            newMessage = true;
            break;
        }
    }
    if(!newMessage){return;}
    // decode message
    switch(message[0]){
        case 'S': // change the hardware settings 
            int waterSettingsSize = sizeof(WaterSettings) >> 2;
            hardware.updateState(
                memoryOffset,decodeWaterSettings(message + 1),decodeLightSettings(message + 1 + waterSettingsSize));
            SerialBT.write(srt);
            break;
        case 'T': //change the time settings
            //TODO implement
            SerialBT.write(srt);
            break;
    }
}

inline WaterSettings Communication::decodeWaterSettings(const char message[20]){
    WaterSettings waterSettings;
    int offset = 0;
    for(int i = 0; i < 4; i++){
        waterSettings.trayWaterTimes[i] = bytesToInt(message + offset);
        offset += 4;
    }
    waterSettings.trayWaterUnits = bytesToInt(message + offset);
    return waterSettings;
}

inline LightSettings Communication::decodeLightSettings(const char message[8]){
    LightSettings lightSettings;
    lightSettings.startTime = bytesToInt(message);
    lightSettings.endTime = bytesToInt(message + 4);
    return lightSettings;
}

int Communication::bytesToInt(const char bytes[4]){
    int result = 0;
    result = (((int)bytes[0]) << 24) 
        | (((int)bytes[1]) << 16) 
        | (((int)bytes[2]) << 8) 
        | ((int)bytes[3]);
    return result;
}
