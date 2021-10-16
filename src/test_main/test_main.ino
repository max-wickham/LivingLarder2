
// all sensors and valves are numbered from top down
// float switches
const int floatSwitch[4] = {27,26,25,33}; // float switch pins, index 0 is the top, active LOW 0 means the float is up
// valves
const int valve[3] = {19,21,18}; // valve pins, index 0 is the top
bool valveOut[3] = {false, false, false};   // the values to write to the valves at the end of the loop

const int pump = 4;   // the pump pin out
bool pumpOut = false;     // the value to send to the pump at the end of the loop
const int lights = 2; // the lights pin out
bool lightsOut = false;   // the value to send to the lights at the end of the loop

struct WaterState
{
    char prevFloatSwitchState[4];              // state of the float switches at the begining of a switch
    unsigned long prevWaterTime = 0;           // time when the last watering occured
    unsigned long waterDelay = 10 * 60 * 1000; // time between watering
};

struct LightsState
{
    bool lightState = HIGH;
    unsigned long doubleprevLightTime = 0;
    unsigned long lightDelay = 12 * 60 * 60 * 1000;
};

void water(WaterState &ws);
void light(LightsState &ls);
void output();

WaterState waterState;
LightsState lightsState;
void setup()
{
    //setup
    Serial.begin(9600);
    Serial.println("Hello");
    // set pin modes
    pinMode(floatSwitch[0], INPUT);
    pinMode(floatSwitch[1], INPUT);
    pinMode(floatSwitch[2], INPUT);
    pinMode(floatSwitch[3], INPUT);

    pinMode(valve[0], OUTPUT);
    pinMode(valve[1], OUTPUT);
    pinMode(valve[2], OUTPUT);

    pinMode(pump, OUTPUT);
    pinMode(lights, OUTPUT);

    pinMode(21,OUTPUT);
}

void test(){
    for (int i = 0; i < 3; i++)
    {
        valveOut[i] = HIGH;
    }
    valveOut[2] = HIGH;
    pumpOut = HIGH;
    lightsOut = HIGH;
}

void loop()
{
    water(waterState);
    light(lightsState);
    output();
    delay(500);
}

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
        // save state of switches
        for(int i = 0; i < 4; i++){
            ws.prevFloatSwitchState[i] = digitalRead(floatSwitch[i]);
        }
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
    //if(digitalRead(floatSwitch[3])){
    //    pumpOut = LOW;
    //}
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(valve[i], !valveOut[i]);
    }
    digitalWrite(pump, !pumpOut);
    lightsOut = (!pumpOut)&&lightsOut; // turn off lights if pump is on
    digitalWrite(lights, !lightsOut);
    // /*
    Serial.print("Switch0 ");
    Serial.println(digitalRead(floatSwitch[0]));
    Serial.print("Switch1 ");
    Serial.println(digitalRead(floatSwitch[1]));
    Serial.print("Switch2 ");
    Serial.println(digitalRead(floatSwitch[2]));
    Serial.print("Switch3 ");
    Serial.println(digitalRead(floatSwitch[3]));
    // */
}