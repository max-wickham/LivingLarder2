#include "Arduino.h"
#include "BluetoothSerial.h"
#include "../Hardware/Hardware.h"

class Communication{

    private:
    BluetoothSerial SerialBT;
    const char eot = '#'; // End of transmission
    const char srt = 'S'; // Successfully received transmission
    inline WaterSettings decodeWaterSettings(const char message[40]);
    inline LightSettings decodeLightSettings(const char message[40]);
    int bytesToInt(char bytes[4]);

    public:
    Communication();
    void run(Hardware &hardware, int memoryOffset);
};