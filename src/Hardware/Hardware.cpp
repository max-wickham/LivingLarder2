#include "Hardware.h"
#include "Arduino.h"

void WaterState::load(int &memoryOffset){
    //TODO implement read from memory
}

void WaterState::save(int &memoryOffset, WaterSettings newWaterSettings){
    waterSettings = newWaterSettings;
    //TODO implement write to memory
}

void LightState::load(int &memoryOffset){
    //TODO implement read from memory
}

void LightState::save(int &memoryOffset, LightSettings newLightSettings){
    lightSettings = newLightSettings;
    //TODO implement write to memory
}

Hardware::Hardware(int &memoryOffset){
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

void Hardware::run(unsigned long int seconds){
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


inline void Hardware::runWaterSystem(unsigned long int seconds){

}

inline void Hardware::runLightSystem(unsigned long int seconds){
    if(lightState.lightSettings.startTime > lightState.lightSettings.endTime){
        // case the in which the lights are on at midnight
        lightOutput = ((seconds > lightState.lightSettings.startTime) || (seconds < lightState.lightSettings.endTime));
    }
    else{
        lightOutput = ((seconds > lightState.lightSettings.startTime) && (seconds < lightState.lightSettings.endTime));
    }
}