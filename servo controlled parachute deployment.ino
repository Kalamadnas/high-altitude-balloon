
#include <SD.h>
#include <Wire.h>
#include "SparkFunMPL3115A2.h"
#include <Adafruit_MPL3115A2.h>

#include <Servo.h>


// ------------------------------- running median

#include "RunningMedian.h"

RunningMedian samples = RunningMedian(5);

long count = 0;

// ------------------------------- running median


Servo myservoA;  // create servo object to control a servo
Servo myservoB;  // create servo object to control a servo

//Create an instance of the object
MPL3115A2 myPressure;
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();


unsigned long previousMillis = 0;          

// variables will change:
int posA = 0;    // variable to store the servo position
int posB = 0;    // variable to store the servo position

// initialize the sensor:
const int chipSelect = 4;         // SPI chip select for SD card
const int cardDetect = 7;          // pin that detects whether the card is there
const int writeLed = 8;           // LED indicator for writing to card
const int errorLed = 13;          // LED indicator for error
long lastWriteTime = 0;           // timestamp for last write attempt
const int interval = 2000;            // time between readings
const int timer = 30000;            // aikaraja jonka jälkeen laskuvarjo voidaan vapauttaa
char fileName[] = "datalog.txt";  // filename to save on SD card

float altm;
float temperature_ulko;

void setup() {

    Wire.begin();        // Join i2c bus
  Serial.begin(9600);  // Start serial for output

  //Configure the sensor
  myPressure.begin(); // Get sensor online
  myPressure.setModeAltimeter(); // Measure altitude above sea level in meters
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags 

  myservoA.attach(12);  // attaches the servo on pin 9 to the servo object
//  myservoB.attach(9);  // attaches the servo on pin 10 to the servo object

    for (posA = 180; posA >= 0; posA -= 1) 
    { // goes from 180 degrees to 0 degree
      myservoA.write(posA);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }

// ----------------- Running median setup
 // Serial.begin(115200);

  if (! baro.begin()) {
    Serial.println("Couldnt find sensor");
    return;
  }

 Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
}



void loop() {
  // initialize LED and cardDetect pins:
//  pinMode(writeLed, OUTPUT);
//  pinMode(errorLed, OUTPUT);  

  unsigned long currentMillis = millis();

  // read sensors every 10 seconds
 if (millis()  - lastWriteTime >=  interval) 
   {
   File logFile = SD.open(fileName, FILE_WRITE);   // open the log file
   if (logFile) {                                  // if you can write to the log file,
      // read sensor:
      temperature_ulko = myPressure.readTemp();
  //    float altitude = myPressure.readAltitude() -65400;
        altm = baro.getAltitude();

   // print to the log file:
   int otaaika = millis();
   logFile.print(otaaika/1000, 1);
   logFile.print(",");
  logFile.print(altm);
  logFile.print(",");
  logFile.println(temperature_ulko);
  logFile.close();                    // close the file
   }
       // update the last attempted save time:
      lastWriteTime = millis(); 



//-------------------------median

  if (count % 20 == 0) Serial.println(F("\nmsec \tAnR \tSize \tCnt \tLow \tAvg \tAvg(3) \tMed \tHigh"));
  count++;

 // long x = myPressure.readAltitude() -65400;
 long x = baro.getAltitude();

  samples.add(x);

//  float l = samples.getLowest();
  float m = samples.getMedian();
//  float a = samples.getAverage();
//  float a3 = samples.getAverage(3);
//  float h = samples.getHighest();
//  int s = samples.getSize();
//  int c = samples.getCount();

  Serial.print(m);
  Serial.print('\t');
  delay(100);

//-------------------------median

  Serial.print(millis());
      Serial.print(" >");
      Serial.print(timer);
            Serial.print(" ");
      Serial.print(altm);
            Serial.print("m ");
      Serial.print(temperature_ulko);
               Serial.println("*C");
     if (millis() >=  timer && altm <= 1000)
     { 
      /*Serial.print(millis());
      Serial.print(" >");
      Serial.print(timer);
            Serial.print(" ");
      Serial.println(altm);*/


       for (posA = 0; posA <= 180; posA += 1) 
        { // goes from 0 degrees to 180 degrees
          // in steps of 1 degree
        myservoA.write(posA);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position
        }
      /* for (posA = 180; posA >= 0; posA -= 1) 
        { // goes from 180 degrees to 0 degrees
        myservoA.write(posA);              // tell servo to go to position in variable 'pos'
        delay(15);                       // waits 15ms for the servo to reach the position
        }
      */
     } 
   }
      
}  


//    digitalWrite(writeLed, LOW);      // turn off the write LED

//  Serial.println();
  
  
