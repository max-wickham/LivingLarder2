
// all sensors and valves are numbered from top down
// float switches
const int floatSwitch0; // top float switch
const int floatSwitch1;
const int floatSwitch2;
const int floatSwitch3; // bottom float switch
// valves
const int valve0; // top valve
const int valve1; // bottom valve
const int valve2;

const int pump;
const int lights;


void setup()
{
    pinMode(floatSwitch0,INPUT);
    pinMode(floatSwitch1,INPUT);
    pinMode(floatSwitch2,INPUT);
    pinMode(floatSwitch3,INPUT);

    pinMode(valve0,OUPUT);
    pinMode(valve1,OUPUT);
    pinMode(valve2,OUPUT);

    pinMode(pump,OUPUT);
    pinMode(lights,OUPUT);
}

void loop()
{
}