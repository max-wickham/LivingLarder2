#include "Hardware.h"
#include "Arduino.h"
#include "../Memory/Memory.h"

void WaterState::load(int &memoryOffset){
    // Read each value from memory and then increment the memory offset
    for(int i = 0; i < 4; i++){
        waterSettings.trayWaterTimes[i] = Memory.readInt(memoryOffset++);
    }
    waterSettings.trayWaterUnits =  Memory.readInt(memoryOffset++);
}

void WaterState::save(int &memoryOffset, WaterSettings newWaterSettings){
    this->waterSettings = newWaterSettings;
    // Write each value to memory and then increment the memory offset
    for(int i = 0; i < 4; i++){
        Memory.writeInt(this->waterSettings.trayWaterTimes[i],memoryOffset++);
    }
    Memory.writeInt(this->waterSettings.trayWaterUnits,memoryOffset++);
}

void LightState::load(int &memoryOffset){
    // Read each value from memory and then increment the memory offset
    this->lightSettings.startTime = Memory.readInt(memoryOffset++);
    this->lightSettings.endTime = Memory.readInt(memoryOffset++);
}

void LightState::save(int &memoryOffset, LightSettings newLightSettings){
    this->lightSettings = newLightSettings;
    // Write each value to memory and then increment the memory offset
    Memory.writeInt(this->lightSettings.startTime,memoryOffset++);
    Memory.writeInt(this->lightSettings.endTime,memoryOffset++);
}

Hardware::Hardware(int &memoryOffset){
    // TODO add logic for first ever start up
    setupPins();
    // load state from memory
    waterState.load(memoryOffset);
    lightState.load(memoryOffset);
}

Hardware::Hardware(){
    setupPins();
    // no state to load
}

void Hardware::setupPins(){
    pinMode(lightsPin,OUTPUT);
    pinMode(waterPumpPin,OUTPUT);
    for(int i = 0; i < 3; i++){
        pinMode(valvePins[i],OUTPUT);
    }
    for(int i = 0; i < 4; i++){
        pinMode(floatSwitchPins[i],INPUT);
    }
}

void Hardware::updateState(int &memoryOffset, WaterSettings newWaterSettings, LightSettings newLightSettings){
    // save new state
    waterState.save(memoryOffset, newWaterSettings);
    lightState.save(memoryOffset, newLightSettings);
}

void Hardware::run(unsigned int seconds){
    // read the inputs
    readPins();
    // run the state management systems
    runWaterSystem(seconds);
    runLightSystem(seconds);
    // write the outputs
    writePins();
}

inline void Hardware::writePins(){
    digitalWrite(lightsPin,lightOutput);
    digitalWrite(waterPumpPin,waterPumpOutput);
    for(int i = 0; i < 3; i++){
        digitalWrite(valvePins[i],valveOutput[i]);
    }
}

inline void Hardware::readPins(){
    for(int i = 0; i < 4; i++){
        floatSwitchState[i] = digitalRead(floatSwitchPins[i]);
    }
}


inline void Hardware::runWaterSystem(unsigned int seconds){

}

inline void Hardware::runLightSystem(unsigned int seconds){
    if(lightState.lightSettings.startTime > lightState.lightSettings.endTime){
        // case the in which the lights are on at midnight
        lightOutput = ((seconds > lightState.lightSettings.startTime) || (seconds < lightState.lightSettings.endTime));
    }
    else{
        lightOutput = ((seconds > lightState.lightSettings.startTime) && (seconds < lightState.lightSettings.endTime));
    }
}