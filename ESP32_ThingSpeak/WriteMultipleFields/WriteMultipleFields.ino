/*
  WriteMultipleFields
  
  Description: Writes values to fields 1,2,3,4 and status in a single ThingSpeak update every 20 seconds.
  
  Hardware: ESP32 based boards
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires installation of EPS32 core. See https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md for details. 
  - Select the target hardware from the Tools->Board menu
  - This example is written for a network using WPA encryption. For WEP or WPA, change the WiFi.begin() call accordingly.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2020, The MathWorks, Inc.
*/

#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int numberRandom;
String nombreRedes[15];
signed int idRedes[15];
int nRedes;
int nRed = 0;

void setup() {
  Serial.begin(115200);  //Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    }
    Serial.println("\nConnected.");
  }
  
  if (nRed == 0) {
    WiFiScan();
  }
  
  long rssi = WiFi.RSSI();
  Serial.print("My RSSI: ");
  Serial.println(rssi);
  
  numberRandom = random(0,100);
  Serial.print("Número Random: ");
  Serial.println(numberRandom);

  Serial.print("Red ");
  Serial.print(nRed + 1);
  Serial.print(": ");
  Serial.print(nombreRedes[nRed]);
  Serial.print(" (");
  Serial.print(idRedes[nRed]);
  Serial.println(")");
  
  // set the fields with the values
  ThingSpeak.setField(1, rssi);
  ThingSpeak.setField(2, numberRandom);
  ThingSpeak.setField(3, idRedes[nRed]);
  
  if (nRed == nRedes-1) {
    nRed = 0;
  } else {
    nRed++;
  }
  
  // write to the ThingSpeak channel
  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(httpCode == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(httpCode));
  }
  
  Serial.println("");
  delay(20000); // Wait 20 seconds to update the channel again
}

void WiFiScan()
{
  Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  nRedes = WiFi.scanNetworks();
  Serial.println("scan done");
  if (nRedes == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(nRedes);
    Serial.println(" networks found");
    for (int i = 0; i < nRedes; ++i) {Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      nombreRedes[i] = WiFi.SSID(i);
      idRedes[i] = WiFi.RSSI(i);
      delay(10);
    }
  }
  Serial.println("");
  delay(100);
}
