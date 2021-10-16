#include "Arduino.h"
#include <EEPROM.h>


class Memory{
    static int readInt(int wordAddress);
    static int writeInt(int wordAddress);
};