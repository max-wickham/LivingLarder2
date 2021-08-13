// all sensors and valves are numbered from top down
// float switches
const int floatSwitch[4]; // float switch pins, index 0 is the top, active LOW 0 means the float is up
// valves
const int valve[3]; // valve pins, index 0 is the top
bool valveOut[3];   // the values to write to the valves at the end of the loop

const int pump;   // the pump pin out
bool pumpOut;     // the value to send to the pump at the end of the loop
const int lights; // the lights pin out
bool lightsOut;   // the value to send to the lights at the end of the loop

WaterState waterState;
void setup()
{
    pinMode(floatSwitch[0], INPUT);
    pinMode(floatSwitch[1], INPUT);
    pinMode(floatSwitch[2], INPUT);
    pinMode(floatSwitch[3], INPUT);

    pinMode(valve[0], OUTPUT);
    pinMode(valve[1], OUTPUT);
    pinMode(valve[2], OUTPUT);

    pinMode(pump, OUTPUT);
    pinMode(lights, OUTPUT);
}

void loop()
{
    water(waterState);
    light(lightsState);
    output();
}

struct WaterState
{
    char prevFloatSwitchState[4];              // state of the float switches at the begining of a switch
    unsigned long prevWaterTime = 0;           // time when the last watering occured
    unsigned long waterDelay = 10 * 60 * 1000; // time between watering
};
WaterState waterState;
void water(WaterState &ws)
{
    // initilise all the outputs to LOW
    pumpOut = LOW;
    for (int i = 0; i < 3; i++)
    {
        valveOut[i] = LOW;
    }

    long mill = millis();
    // check that it is time to change the water
    if (mill < ws.prevWaterTime + ws.waterDelay)
    {
        return;
    }

    // check that the float switch state has changed
    bool changed = false;
    for (int i = 0; i < 4; i++)
    {
        changed = changed || (digitalRead(floatSwitch[i]) == ws.prevFloatSwitchState[i]);
    }
    if (changed)
    {
        ws.prevWaterTime = mill; // change complete
        return;
    }

    // control the valves
    for (int i = 0; i < 2; i++)
    {
        valveOut[i] = !digitalRead(floatSwitch[i]) && digitalRead(floatSwitch[i + 1]);
    }
    pumpOut == !ws.prevFloatSwitchState[3] && digitalRead(floatSwitch[0]);
}

struct LightsState
{
    bool lightState = HIGH;
    unsigned long doubleprevLightTime = 0;
    unsigned long lightDelay = 12 * 60 * 60 * 1000;
};
LightsState lightsState;
void light(LightsState &ls)
{
    lightsOut = ls.lightState;
    if (millis() < ls.doubleprevLightTime + ls.lightDelay)
    {
        return;
    }
    ls.lightState = !ls.lightState;
    ls.doubleprevLightTime = millis();
}

void output()
{
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(valve[i], valveOut[i]);
    }
    digitalWrite(pump, pumpOut);
    digitalWrite(lights, lightsOut);
}