/*
  Reading Lat/Long from the Qwiic GPS module over I2C
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 12th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows date, time and location.

  You will need two libraries:
  1) Use library manager to search for and install: 'SparkFun I2C GPS Arduino Library'
  2) Download and install the zip library from https://github.com/mikalhart/TinyGPSPlus

  The TitanX1 has a buffer of approximately 1,000 NMEA characters. If you don't poll the module
  for a few seconds this buffer will fill up. The first time you read from the GPS module you will
  receive this large buffer and may take up to 800ms to clear out the contents. After that it will
  take approximately 20ms to read the contents of the I2C buffer from the module.

  Hardware Connections:
  Attach a Qwiic shield to your RedBoard or Uno.
  Plug the Qwiic sensor into any port.
  Serial.print it out at 115200 baud to serial monitor.
*/

#define CE_PIN 6
#define CSN_PIN 9

#include "RF24.h"
#include <SPI.h>
#include <SD.h>

#include <Adafruit_SleepyDog.h>

#include <SparkFun_I2C_GPS_Arduino_Library.h> //Use Library Manager or download here: https://github.com/sparkfun/SparkFun_I2C_GPS_Arduino_Library
I2CGPS myI2CGPS; //Hook object to the library

#include <TinyGPS++.h> //From: https://github.com/mikalhart/TinyGPSPlus
TinyGPSPlus gps; //Declare gps object

// kolme seuraavaa altitudea varten
#include <Wire.h>
#include <Adafruit_MPL3115A2.h>
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

RF24 radio(CE_PIN, CSN_PIN);

byte address[11] = "SimpleNode";
char payload[32] = "ping";

long lastWriteTime = 0;           // timestamp for last write attempt
long interval = 1000;            // time between readings
char fileName[] = "datalog.txt";  // filename to save on SD card
String configString;

void setup()
{
  delay (10000);
  Serial.begin(115200);
  Serial.println("GTOP Read Example");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("GTOP Read Example2");

  if (myI2CGPS.begin() == false)
  {
    Serial.println("Module failed to respond. Please check wiring.");
    while (1); //Freeze!
  }
  Serial.println("GPS module found!");

    configString = myI2CGPS.createMTKpacket(886, ",3");
        Serial.print("sending: "); Serial.println(configString);
      myI2CGPS.sendMTKpacket(configString);
      
// startSDCard() blocks everything until the card is present
  // and writable:
    if (startSDCard() == true) {
    Serial.println("card initialized.");
    delay(100);
    // open the log file:
    File logFile = SD.open(fileName, FILE_WRITE);
    // write header columns to file:
    if (logFile) {
      logFile.println("-----");
      logFile.close();
    }
  }
      
 Watchdog.enable(5000);
 Serial.println("watchdog enabled");
}

void loop() {
 Watchdog.reset();
  Serial.println("watchdog reset");
  if (! baro.begin()) {
    Serial.println("Couldnt find sensor");
    return;
  }

float altm = baro.getAltitude();
  Serial.print(altm); Serial.println(" meters");

  float tempC = baro.getTemperature();
  Serial.print(tempC); Serial.println("*C");

  delay(250);
// gps puoli alkaa

  while (myI2CGPS.available()) //available() returns the number of new bytes available from the GPS module
  {
    gps.encode(myI2CGPS.read()); //Feed the GPS parser
  }

  if (gps.time.isUpdated()) //Check to see if new GPS info is available
  {
    displayInfo();
  }
}

//Display new GPS info
void displayInfo()
{

     float vbat = analogRead(A7);
      vbat *= 2;  // 200ohm vastus jakaa, tuplataan takaisin
      Serial.println(vbat);
      vbat *= 3.3;  //  referenssijännite x3.3V
      Serial.println(vbat);
      vbat /= 1024;  // refe antaa aina x1k
      Serial.println(vbat);
  
  //We have new GPS data to deal with!
  Serial.println();

  if (gps.time.isValid())
  {
    Serial.print(F("Date: "));
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());

    Serial.print((" Time: "));
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());

    Serial.println(); //Done printing time
  }
  else
  {
    Serial.println(F("Time not yet valid"));
  }

  if (gps.location.isValid())
  {
    Serial.print("Location: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(", "));
    Serial.print(gps.location.lng(), 6);
    Serial.println();
    Serial.println(gps.altitude.meters(), 6); // Altitude in meters (double)
  }
  else
  {
    Serial.println(F("Location not yet valid"));
  }
  
 if (millis()  - lastWriteTime >=  interval) {
    Serial.println("filewrite");
    File logFile = SD.open(fileName, FILE_WRITE);
    // write header columns to file:
    if (logFile) {
      logFile.print(millis());
            logFile.print(", N");
      logFile.print(gps.location.lat(), 6);
            logFile.print(", E");
      logFile.print(gps.location.lng(), 6);
            logFile.print(", ");
      logFile.print(gps.altitude.meters(), 3);
            logFile.print(", ");
      logFile.print(baro.getAltitude(), 3);
            logFile.print(", ");
      logFile.print(baro.getTemperature(), 3);
            logFile.print(", ");
      logFile.println(vbat);
      Serial.print("v: " ); Serial.println(vbat);

      logFile.close();

  Serial.println("radio begin");
  radio.begin(); // Start up the radio
  radio.setAutoAck(1); // Ensure autoACK is enabled
  radio.setRetries(15,15); // Max delay between retries & number of retries
  Serial.println("radio running");  
  radio.openWritingPipe(address);
  radio.write(payload, 32); // nrf print here
  Serial.println("radio send" );

      //------------------- vilkutus
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(100);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(100);                       // wait for a second
    }
      lastWriteTime = millis();
   }
}


boolean startSDCard() {
   // wait until the card initialized successfully:
  while (!SD.begin(4)) { // chip select 4
    digitalWrite(13, HIGH);   // turn on error LED
    Serial.println("Card failed");
    delay(750);
  }
  return true;
 
}
