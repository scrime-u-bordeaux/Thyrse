/*
   See documentation at https://nRF24.github.io/RF24
   See License information at root directory of this library
   Author: Brendan Doherty (2bndy5)
*/

/**
   A simple example of sending data from 1 nRF24L01 transceiver to another.

   This example was written to be used on 2 devices acting as "nodes".
   Use the Serial Monitor to change each node's behavior.
*/
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
RF24 radio(14, 10); // using pin 14 for the CE pin, and pin 10 for the CSN pin
uint8_t address[6] = "00001";
float payload = 0.0;
#define SIZE 32
byte buffer[SIZE];
bool flagNote[16];
//#define DEBUG 0

void setup() {
  delay(555);
  Serial.begin(115200);
  //Serial.print("startsetupbegin");
  // initialize the transceiver on the SPI bus
  while (!radio.begin()) {
    Serial.println("radio hardware is not responding!!");
    delay(100);
  }
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();
  //Serial.print("startsetupend");
} // setup

void loop() {
  if (radio.available()) {             // is there a payload? get the pipe number that recieved it
    //uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
    radio.read(buffer, SIZE );           // fetch payload from FIFO

#ifdef DEBUG
    Serial.print("Received ");
    for (uint8_t j = 0; j < SIZE; ++j) {
      Serial.print(buffer[j]); Serial.print("_");
    }
    Serial.println();
#endif
    for (uint8_t i = 0; i < 19; ++i) {
      usbMIDI.sendControlChange(i, buffer[i], 1);
    }
    //usbMIDI.sendNoteOn(note, velocity, channel);
    //       if(buffer[i] && !flagNote[i]){
    //        usbMIDI.sendNoteOn(i, 127, 1);
    //        flagNote[i]=1;
    //      }
    //      if(!buffer[i] && flagNote[i]){
    //        usbMIDI.sendNoteOff(i, 0, 1);
    //        flagNote[i]=0;
  }
  //     usbMIDI.sendControlChange(10, buffer[16], 1);
  //     usbMIDI.sendControlChange(11, buffer[17], 1);
  //     usbMIDI.sendControlChange(12, buffer[18], 1);


} // loop
