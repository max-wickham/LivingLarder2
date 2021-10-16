#include "Arduino.h"
#include "Memory.h"
#include <EEPROM.h>

static int Memory::readInt(int wordAddress){
    int byteAddress = (wordAddress << 2);
    int byte0 = EEPROM.read(byteAddress++);
    int byte1 = EEPROM.read(byteAddress++);
    int byte2 = EEPROM.read(byteAddress++);
    int byte3 = EEPROM.read(byteAddress);
    int result = byte0;
    int mask = (1 << 8) - 1;
    int result = (result & mask);
    mask <<= 8;
    result = (result | (byte1 & mask));
    mask <<= 8;
    result = (result | (byte2 & mask));
    mask <<= 8;
    result = (result | (byte3 & mask));
}

static int Memory::writeInt(int data, int wordAddress){
    int byteAddress = (wordAddress << 2);
    int mask = (1 << 8) - 1;
    int byte0 = data&mask;
    mask <<= 8;
    int byte1 = (data&mask) >> 8;
    mask <<= 8;
    int byte2 = (data&mask) >> 8;
    mask <<= 8;
    int byte3 = (data&mask) >> 8;
    EEPROM.write(byteAddress++,byte0);
    EEPROM.write(byteAddress++,byte1);
    EEPROM.write(byteAddress++,byte2);
    EEPROM.write(byteAddress++,byte3);
    EEPROM.commit();
}

