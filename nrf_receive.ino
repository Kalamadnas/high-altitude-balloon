#define CE_PIN 6
#define CSN_PIN 9

#include <SPI.h>
#include "RF24.h"
#include <Wire.h>

RF24 radio(CE_PIN, CSN_PIN);

byte address[11] = "SimpleNode";
// unsigned long 
char payload[32];
int radiostat = 0;
int laskuri = 0;



void setup() {
  // Serial.begin(9600);
 //  while (!Serial)
  Serial.begin(115200);
  Serial.println("setup start");
  radio.begin(); // Start up the radio
  radio.setAutoAck(1); // Ensure autoACK is enabled
  radio.setRetries(15,15); // Max delay between retries & number of retries

  Serial.println("wait for 1s");
    delay(1000);   // wait a second for nrf write
  radio.openReadingPipe(1, address); // Write to device address 'SimpleNode'
  radio.startListening();
  Serial.println("setup end");
}


void loop(){
  Serial.println("loop start");
  radio.read( &payload, 32);
    Serial.print("Got Payload: ");
       String ass = String(payload);
  if (ass != ""){ Serial.println(ass);
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(100);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(100);                       // wait for a second
  } else     Serial.println("nothing");

  
delay(100); // receive every 1/3 second
}
