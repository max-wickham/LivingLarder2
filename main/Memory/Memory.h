#include "Arduino.h"
#include <EEPROM.h>


class Memory{
    public:
    static int readInt(int wordAddress);
    static int writeInt(int data, int wordAddress);
};