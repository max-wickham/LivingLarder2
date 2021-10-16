# Living Larder Arduino Code
This is the source code for the microcontroller that runs the Living Larder system.
The microncontroller is an ESP32.

## Code Structure
The "src/main/main.ino" file is the entry point to the app, here a communication class, time class and hardware class are instantiated.

### Hardware Class
The hardware class controlls all the electronic inputs and outputs to the system. This is comprised of two pumps, 3 valves, 4 float switches, a conductivity sensor and the lights. The lights, pumps and valves are all connected to relays that are controlled by the micrtocontroller. The hardware class can save the state needed to know how long to water plants for and the lighting times in the EEPROM and can update this information. The class contains a run function that should be called in the "void loop" of the main.ino file.

### Time Class
This class is simply used to interface with the RTC connected to the ESP32. The class contains a function "getSeconds()" that returns the number of seconds since the start of the day.

### Communication Class
This class handles all of the bluetooth communication of the system with the mobile application used to control the system. 

### Memory Class
This class is used by the hardware class to interface with onboard EEPROM.