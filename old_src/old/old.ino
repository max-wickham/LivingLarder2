#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include "ESP8266WiFi.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


String deviceName;
//EEPROM Write
void eeWriteInt(int pos, int val) {
  int Pos = pos * 4;//takes 4 spaces to write number in ESP8266
  byte* p = (byte*)&val;
  EEPROM.write(Pos, *p);
  EEPROM.write(Pos + 1, *(p + 1));
  EEPROM.write(Pos + 2, *(p + 2));
  EEPROM.write(Pos + 3, *(p + 3));
  EEPROM.commit();
}

//EEPROM Read
int eeGetInt(int pos) {
  int Pos = pos * 4;//takes 4 spaces to write number in ESP8266
  int val;
  byte* p = (byte*)&val;
  *p = EEPROM.read(Pos);
  *(p + 1) = EEPROM.read(Pos + 1);
  *(p + 2) = EEPROM.read(Pos + 2);
  *(p + 3) = EEPROM.read(Pos + 3);
  return val;
}

//Write string to EEPROM
void EEPROM_ESP8266_GRABAR(String buffer, int N) {
  
  for (int L = 0; L < 32; ++L) {
    EEPROM.write(N + L, buffer[L]);
  }
  EEPROM.commit();
}

//Read String to EEPROM
String EEPROM_ESP8266_LEER(int min, int max) {
    String buffer;
  for (int L = min; L < max; ++L)
    if (isAlphaNumeric(EEPROM.read(L)))
      buffer += char(EEPROM.read(L));
  return buffer;
}
//////////////////////////////////

//Setting Up RTC

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
////////////////////////////////

//enable to pause all processes
bool pauseProcesses = false;

//Sets Holiday Mode on
bool holidayMode = false;

//Sets Automatic addition of nutrients
bool autoNutrients = false;
////////////////////////

//finds a value in a string at a given index with a given seperator char
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
////////////////////////////////


//Plant Variables

#define periodNumber 5
enum PlantTypeTimePeriod { Period1 = 0, Period2 = 1, Period3 = 2, Period4 = 3, Period5  = 4};
enum TrayNumberIndexKeys { Tray1 = 0, Tray2 = 1, Tray3 = 2 };
enum PlantTypeTimePeriodLength { PeriodLength1 = 7, PeriodLength2 =14, PeriodLength3 =21, PeriodLength4 = 28};//cumulative
typedef struct
{ 
  int StartAge;//start day in unix time in days
  int PlantType;
  int Age = 0;//days
  int WaterTime = 0;
  int DesiredConcentration = 0;
}  TrayStruct;
//use format Tray[TrayNumberIndexKey].TrayStructValue
TrayStruct Tray[3]; 
/////////////////////////////////////////////////////////////////////



//EEPROM Addresses and Keys /////////////////////////////////////////////////////////////////////////////////////////////////////

#define FirstTime 0
#define nutrientCycleMinutes_EEPROM 1 //number of minutes between nutrient cycling
#define waterCycleTime_EEPROM 2 //time between watering cycles in seconds


typedef struct
{
  int PlantType[3] = { 7,8,9 };//store a code at this location, use format SlotInformation_EEPROM.PlantType[Slot#]
  int WaterTimePeriod[3][5] = {{10, 11, 12, 13, 14},   //use format SlotInformation_EEPROM.WaterTimePeriod[Slot#][# or Period#]
                 {15,16,17,18,19},
                 {20,21,22,23,24} };//period of time for which to water
  int DesiredConcentration[3][5] = { {25, 26, 27, 28, 29},
                 {30,31,32,33,34},
                 {35,36,37,38,39} };//desired nutrient concentration
  int PlantAge[3] = { 3,4,5 };//wont be used often
  int StartAge[3] = { 46,47,48 };
}  SlotInformation_EEPROM;
SlotInformation_EEPROM slotInformation_EEPROM;

int time_eeprom = 6;//stores unix time in days
int LEDStart_EEPROM = 44;//stores the LED turn on time in hours
int LEDEnd_EEPROM = 45;//stores LED turn off time in hours

int wifiSSID_EEPROM = 46;//ends at 78 32byte
int wifiPASS_EEPROM = 79;//ends at 111 32byte

int name_EEPROM = 112;
/////////////////////////////////////////////////////////////////////////


//Pins //////////
//Water depth pins
const int Tray_Depth_Pin[3] = { 1,2,3 };
//Water drain valve pins
const int Tray_DrainValve_Pin[3] = { 10,11,12 };
//Device Pins
const int LED = 9;
const int waterPump = 8;
const int airPump = 7;
const int nutrientPump = 6;
//Sensor Pins
int conductivitySensor = 4;


////////////////////////
//must be continuosly called, requires the current unix day
void updateAge(int days){
  Tray[Tray1].Age = days - Tray[Tray1].StartAge;
  Tray[Tray2].Age = days - Tray[Tray2].StartAge;
  Tray[Tray3].Age = days - Tray[Tray3].StartAge;
}

//updates the water time periods and required nutrients for all trays, should be called daily
void updateWaterNutrient(){
  for (int i; i < 3; i++) {
    if (Tray[i].Age > PeriodLength4) {
      Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period5]);
      Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period5]);
    }
    else {
      if (Tray[i].Age > PeriodLength3) {
        Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period4]);
        Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period4]);
      }
      else {
        if (Tray[i].Age > PeriodLength2) {
          Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period3]);
          Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period3]);
        }
        else {
          if (Tray[i].Age > PeriodLength1) {
            Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period2]);
            Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period2]);
          }
          else {
            Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period1]);
            Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period1]);
          }
        }
      }
    }
  }
}


//turns the leds on and off at correct time, should be called continuosly
void cycleLED(int hour) {
  int on = eeGetInt(LEDStart_EEPROM);
  int off = eeGetInt(LEDEnd_EEPROM);
  if (hour >= on && hour < off) {
    digitalWrite(LED, LOW);
  }
  else {
    digitalWrite(LED, HIGH);
  }
}
bool LEDbool = false;


//waters the plants, should be called continuosly, requires the unix time in seconds
void cycleWater(int unixseconds) {
  int waterCycleTime = eeGetInt(waterCycleTime_EEPROM); //in seconds
  if (lastWaterCycleTime + waterCycleTime < unixseconds) {
    if (lastWaterCycleTime + waterCycleTime + Tray[Tray1].WaterTime > unixseconds) {
      if (digitalRead(Tray_Depth_Pin[Tray1]) == LOW) {
        digitalWrite(waterPump, HIGH);
      }
      else {
        digitalWrite(waterPump, LOW);
      }
      digitalWrite(Tray_DrainValve_Pin[Tray1], HIGH);
      digitalWrite(Tray_DrainValve_Pin[Tray2], LOW);
      digitalWrite(Tray_DrainValve_Pin[Tray3], LOW);
    }
    else {
      if (lastWaterCycleTime + waterCycleTime + Tray[Tray1].WaterTime + Tray[Tray2].WaterTime > unixseconds) {
        if (digitalRead(Tray_Depth_Pin[Tray2]) == LOW) {
          digitalWrite(waterPump, HIGH);
        }
        else {
          digitalWrite(waterPump, LOW);
        }
        digitalWrite(Tray_DrainValve_Pin[Tray1], LOW);
        digitalWrite(Tray_DrainValve_Pin[Tray2], HIGH);
        digitalWrite(Tray_DrainValve_Pin[Tray3], LOW);
      }
      else {
        if (lastWaterCycleTime + waterCycleTime + Tray[Tray1].WaterTime + Tray[Tray2].WaterTime + Tray[Tray3].WaterTime > unixseconds) {
          if (digitalRead(Tray_Depth_Pin[Tray3]) == LOW) {
            digitalWrite(waterPump, HIGH);
          }
          else {
            digitalWrite(waterPump, LOW);
          }
          digitalWrite(Tray_DrainValve_Pin[Tray1], LOW);
          digitalWrite(Tray_DrainValve_Pin[Tray2], LOW);
          digitalWrite(Tray_DrainValve_Pin[Tray3], HIGH);
        }
      }
    }

  }
}
int lastWaterCycleTime = 0;  //contains the time in seconds of the last water cycle

//updates the water time periods and required nutrients for all trays, should be called daily
void updateTrayStats() {
  for (int i; i < 3; i++) {
    Tray[i].PlantType = eeGetInt(slotInformation_EEPROM.PlantType[i]);


    for (int j; j < 5; j++) {
      if (Tray[i].Age > PeriodLength4) {
        Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period5]);
        Tray[i].DesiredConcentration = EEPROM.read(slotInformation_EEPROM.DesiredConcentration[i][Period5]);
      }
      else {
        if (Tray[i].Age > PeriodLength3) {
          Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period4]);
          Tray[i].DesiredConcentration = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period4]);
        }
        else {
          if (Tray[i].Age > PeriodLength2) {
            Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period3]);
            Tray[i].DesiredConcentration = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period3]);
          }
          else {
            if (Tray[i].Age > PeriodLength1) {
              Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period2]);
              Tray[i].DesiredConcentration = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period2]);
            }
            else {
              Tray[i].WaterTime = eeGetInt(slotInformation_EEPROM.WaterTimePeriod[i][Period1]);
              Tray[i].DesiredConcentration = eeGetInt(slotInformation_EEPROM.DesiredConcentration[i][Period1]);
            }
          }
        }
      }
    }

  }
}

//updates the nutrient concentration, should be called continuosly, requires unix seconds
void cycleNutrients(int seconds) {
  float nutrients; /////
  int nutrientCycleTime = eeGetInt(nutrientCycleMinutes_EEPROM); //in minutes
  if ((seconds - lastNutrientCycleTime > nutrientCycleTime * 60) && (seconds - lastNutrientCycleTime < nutrientCycleTime * 60 + 3)) {
    float desiredNutrients = Tray[Tray1].DesiredConcentration;
    for (int i; i < 3; i++) {
      if (Tray[i].DesiredConcentration < desiredNutrients) {
        desiredNutrients = Tray[i].DesiredConcentration;
      }
      if (desiredNutrients < nutrients) {
        digitalWrite(nutrientPump, HIGH);
      }
      else {
        digitalWrite(nutrientPump, LOW);
      }
    }
    lastNutrientCycleTime = millis();
  }
  else { digitalWrite(nutrientPump, LOW); }
}
int lastNutrientCycleTime;




/*wifi functions, variables and constants*/
const char* ssid = "LivingLarder";         // The SSID (name) of the AP
const char* password = "PASSWORD";
char* ssid_router = "SSID";         // The SSID (name) of the Wi-Fi network you want to connect to
char* password_router = "PASSWORD";
ESP8266WebServer server(80);
WiFiClient RemoteClient;


void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();
void handleTrayInfo();
void handlePause();
void handleSetTray1();
void handleSetTray2();
void handleSetTray3();
void handleWifiSettings();
void handleSetTime();
void handleHolidayMode();
void handleSettings();
void handleAutoNutrients();
void handlePauseAllProcesses();
void handleRenameDevice();

//sends the model type
void handleRoot() {
  server.send(200, "text/html", "<h1>LivingLarder1</h1>");
}
//returns an error if page not found
void handleNotFound() {

  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
//sends the tray info
void handleTrayInfo() {
  String data = "TrayInfo," + String(Tray[Tray1].Age) + "," + String(Tray[Tray2].Age) + "," + String(Tray[Tray2].Age) + ","  
        + String(Tray[Tray1].PlantType) + "," + String(Tray[Tray2].PlantType) + "," + String(Tray[Tray3].PlantType) + ",";
  server.send(200, "text/plain", data);
}
//toggles the pauseProcesses variable
void handlePause() {
  pauseProcesses = !pauseProcesses;
  server.send(200, "text/plain", "Paused");
}
//sets tray1 stats
void handleSetTray1() {
  if (!server.hasArg("WaterTimePeriod") || !server.hasArg("DesiredConcentration") || !server.hasArg("PlantType") || !server.hasArg("PlantStartAge")) {
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  String watertimeperiod = server.arg("WaterTimePeriod");
  int _watertimeperiod[5];
  for (int i; i < 5; i++) {
    _watertimeperiod[i] = (getValue(watertimeperiod, ',', i)).toInt();
    eeWriteInt(slotInformation_EEPROM.WaterTimePeriod[Tray1][i], _watertimeperiod[i]);
  }
  String desiredconcentration = server.arg("DesiredConcentration");
  int _desiredconcentration[5];
  for (int i; i < 5; i++) {
    _desiredconcentration[i] = (getValue(desiredconcentration, ',', i)).toInt();
    eeWriteInt(slotInformation_EEPROM.DesiredConcentration[Tray1][i], _desiredconcentration[i]);
  }
  int _planttype = server.arg("PlantType").toInt();
  eeWriteInt(slotInformation_EEPROM.PlantType[Tray1], _planttype);

  int _plantage = server.arg("PlantStartAge").toInt();
  eeWriteInt(slotInformation_EEPROM.StartAge[Tray1], _planttype);

  server.send(200, "text/plain", "Received");
}
//sets tray2 stats
void handleSetTray2() {
  if (!server.hasArg("WaterTimePeriod") || !server.hasArg("DesiredConcentration") || !server.hasArg("PlantType") || !server.hasArg("PlantStartAge")) {
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  String watertimeperiod = server.arg("WaterTimePeriod");
  int _watertimeperiod[5];
  for (int i; i < 5; i++) {
    _watertimeperiod[i] = (getValue(watertimeperiod, ',', i)).toInt();
    eeWriteInt(slotInformation_EEPROM.WaterTimePeriod[Tray2][i], _watertimeperiod[i]);
  }
  String desiredconcentration = server.arg("DesiredConcentration");
  int _desiredconcentration[5];
  for (int i; i < 5; i++) {
    _desiredconcentration[i] = (getValue(desiredconcentration, ',', i)).toInt();
    eeWriteInt(slotInformation_EEPROM.DesiredConcentration[Tray2][i], _desiredconcentration[i]);
  }
  int _planttype = server.arg("PlantType").toInt();
  eeWriteInt(slotInformation_EEPROM.PlantType[Tray2], _planttype);

  int _plantage = server.arg("PlantStartAge").toInt();
  eeWriteInt(slotInformation_EEPROM.StartAge[Tray2], _planttype);

  server.send(200, "text/plain", "Received");
}
//sets tray3 stats
void handleSetTray3() {
  if (!server.hasArg("WaterTimePeriod") || !server.hasArg("DesiredConcentration") || !server.hasArg("PlantType") || !server.hasArg("PlantStartAge")) {
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  String watertimeperiod = server.arg("WaterTimePeriod");
  int _watertimeperiod[5];
  for (int i; i < 5; i++) {
    _watertimeperiod[i] = (getValue(watertimeperiod, ',', i)).toInt();
    eeWriteInt(slotInformation_EEPROM.WaterTimePeriod[Tray3][i], _watertimeperiod[i]);
  }
  String desiredconcentration = server.arg("DesiredConcentration");
  int _desiredconcentration[5];
  for (int i; i < 5; i++) {
    _desiredconcentration[i] = (getValue(desiredconcentration, ',', i)).toInt();
    eeWriteInt(slotInformation_EEPROM.DesiredConcentration[Tray3][i], _desiredconcentration[i]);
  }
  int _planttype = server.arg("PlantType").toInt();
  eeWriteInt(slotInformation_EEPROM.PlantType[Tray3], _planttype);

  int _plantage = server.arg("PlantStartAge").toInt();
  eeWriteInt(slotInformation_EEPROM.StartAge[Tray3], _planttype);

  server.send(200, "text/plain", "Received");
}
//changes the wifi settings
void handleWifiSettings() {
  String _ssid = server.arg("ssid");
  String _password = server.arg("password");
  EEPROM_ESP8266_GRABAR(_ssid, wifiSSID_EEPROM);
  EEPROM_ESP8266_GRABAR(_password, wifiPASS_EEPROM);
  delay(10);////////
  RecallWifiSettings();

  server.send(200, "text/plain", "Received");
}
//changes the unix time
void handleSetTime() {
  String Unixtime = server.arg("unixtime");
  int _unixtime = Unixtime.toInt();
  DateTime now = rtc.now();
  rtc.adjustunix() = _unixtime;
  server.send(200, "text/plain", "Received");
}

void handleHolidayMode(){
  holidayMode = !holidayMode;
  server.send(200, "text/plain", "Received");
}

void handleAutoNutrients()
{
  autoNutrients = !autoNutrients;
  server.send(200, "text/plain", "Received");
}

void handlePauseAllProcesses(){
  pauseProcesses != pauseProcesses;
}

void handleSettings(){
  String data = "Settings," + String(holidayMode) + "," + String(autoNutrients) + "," + String(pauseProcesses);
  server.send(200, "text/plain", data);
}

void handleRenameDevice(){
  if(!server.hasArg("Name")){
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  deviceName = server.arg("Name");
  EEPROM_ESP8266_GRABAR(deviceName,name_EEPROM);
}
/*void CheckForConnections()
{
  if (server.hasClient())
  {
    // If we are already connected to another computer, 
    // then reject the new connection. Otherwise accept
    // the connection. 
    if (RemoteClient.connected())
    {
      Serial.println("Connection rejected");
      server.available().stop();
    }
    else
    {
      Serial.println("Connection accepted");
      RemoteClient = server.available();
    }
  }
}*/

//connects to the wifi, if unable sets up access point 
void ConnectionSetUp() {
  //RecallWifiSettings();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid_router, password_router);  //try connecting to wifi 
  bool APactive = false;
  int counter = 0;
  bool endloop = false;

  while (WiFi.status() != WL_CONNECTED || !endloop) { // Wait for the Wi-Fi to connect
    delay(500);
    Serial.print('.');
    counter += 1;
    if (counter > 20) {//if wifi doesn't connect after 20 counts start own access point
      Serial.print("Configuring access point...");
      /* You can remove the password parameter if you want the AP to be open. */
      WiFi.softAP(ssid);

      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      server.on("/", handleRoot);
      server.on("/WifiSettings", handleWifiSettings);
      server.begin();
      Serial.println("HTTP server started");
      endloop = true;
      isconnected = false;
      APactive = true;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    isconnected = true;
  }
  //server setup
  if (!APactive){
  server.on("/", handleRoot);
  server.on("/TrayInfo", HTTP_GET, handleTrayInfo);
  server.on("/Pause",HTTP_GET, handlePause);
  server.on("/Tray1", HTTP_GET, handleSetTray1);
  server.on("/Tray2", HTTP_GET, handleSetTray1);
  server.on("/Tray3", HTTP_GET, handleSetTray1);
  server.on("/HolidayMode",HTTP_GET,handleHolidayMode);
  server.on("/AutoNutrients",HTTP_GET,handleAutoNutrients);
  server.on("/PauseAllProcesses",HTTP_GET,handlePauseAllProcesses);
  server.on("/Settings",HTTP_GET,handleSettings);
  server.on("/RenameDevice",HTTP_GET,handleRenameDevice);
  server.onNotFound(handleNotFound);
  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
  }
}
bool isconnected = false;


void UpdateServer() {
  server.handleClient();
}

//call at void start
void RecallWifiSettings() {
  int str_len = EEPROM_ESP8266_LEER(wifiSSID_EEPROM, wifiSSID_EEPROM + 32).length() + 1;
  // Prepare the character array (the buffer) 
  char char_array[32];
  // Copy it over 
  EEPROM_ESP8266_LEER(wifiSSID_EEPROM, wifiSSID_EEPROM + 32).toCharArray(char_array, str_len);
  ssid_router = char_array;

  str_len = EEPROM_ESP8266_LEER(wifiPASS_EEPROM, wifiPASS_EEPROM + 32).length() + 1;
  EEPROM_ESP8266_LEER(wifiPASS_EEPROM, wifiPASS_EEPROM + 32).toCharArray(char_array, str_len);
  ssid_router = char_array;




}


void setup() {
  
  Serial.begin(9600);
  EEPROM.begin(512);//number of bytes
  deviceName = EEPROM_ESP8266_LEER(name_EEPROM);
  updateTrayStats();///////update the information from the eeprom
  //tray pins setup
  pinMode(Tray_Depth_Pin[Tray1], INPUT);
  pinMode(Tray_Depth_Pin[Tray2], INPUT);
  pinMode(Tray_Depth_Pin[Tray3], INPUT);

  pinMode(Tray_DrainValve_Pin[Tray1], OUTPUT);
  pinMode(Tray_DrainValve_Pin[Tray2], OUTPUT);
  pinMode(Tray_DrainValve_Pin[Tray3], OUTPUT);


  //devices
  pinMode(LED, OUTPUT);
  pinMode(waterPump, OUTPUT);
  pinMode(airPump, OUTPUT);
  pinMode(nutrientPump, OUTPUT);
  pinMode(conductivitySensor, INPUT);


  ////////////////////////setting up wifi connection
  ConnectionSetUp();
  ////////////////////////////////////////////////////////////////////////////
}

////timer variables
int timer1val;
int timer1delay = 86400;
void loop() {
  ////server////
  UpdateServer();
  //////////////////
  DateTime now = rtc.now();
  int seconds = now.unixtime();
  int days = now.unixtime() / 86400;
  /////////////////////////
  cycleLED(now.hour());
  cycleWater(seconds);
  cycleNutrients(seconds);

  ////timers
  if (timer1val > seconds + timer1delay) {
    updateAge(days);
    timer1val = seconds;
  }
}
