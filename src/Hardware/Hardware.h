#include "Arduino.h"
/*
All times are measured in seconds unless specified otherwise
*/
// TODO add support for nutirent pump
// TODO add support for flow rate meter

struct WaterSettings { 
    unsigned int trayWaterTimes[4];
    unsigned int trayWaterUnits;
};

struct WaterState
{
    WaterSettings waterSettings;
    char prevFloatSwitchState[4];              // state of the float switches at the begining of a switch
    //load the state from memory from a given offset and modify the offset by number of memory address used by the struct
    void load(int &memoryOffset);
    //save the state in memory at a given offset and modify the offset by number of memory address used by the struct
    void save(int &memoryOffset, WaterSettings newWaterSettings);
};

struct LightSettings {
    unsigned int startTime;
    unsigned int endTime;
};

struct LightState
{
    LightSettings lightSettings;
    //load the state from memory from a given offset and modify the offset by number of memory address used by the struct
    void load(int &memoryOffset);
    //save the state in memory at a given offset and modify the offset by number of memory address used by the struct
    void save(int &memoryOffset, LightSettings newLightSettings);
};

class Hardware{

    private:
    /* 
    all device pins are counted from top down within the device,
    so the top tray will use the pins in the 0 item of the pin arrays 
    */
    // outputs
    int lightsPin;
    bool lightOutput;
    int waterPumpPin;
    bool waterPumpOutput;
    int valvePins[3]; 
    bool valveOutput[3];
    // inputs
    int floatSwitchPins[4];
    bool floatSwitchState[3];

    WaterState waterState;
    LightState lightState;

    void setupPins();
    // only called in one place so should be inline to impove efficiency
    inline void runWaterSystem(unsigned int seconds);
    inline void runLightSystem(unsigned int seconds);
    inline void writePins();
    inline void readPins();

    public:
    Hardware(int &memoryOffset);
    Hardware();
    // takes the time in seconds since the start of the day, should be run on in void loop
    void run(unsigned int seconds);
    // should update the necessary watering times for each tray in seconds and the light start and end times
    void updateState(int &memoryOffset, WaterSettings newWaterSettings, LightSettings newLightSettings);

};