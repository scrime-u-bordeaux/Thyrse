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
RF24 radio(14, 10);  // using pin 14 for the CE pin, and pin 10 for the CSN pin
uint8_t address[6] = "00001";
float payload = 0.0;
#define SIZE 32
byte buffer[SIZE];
bool flagNote[8] = { false };
// #define DEBUG ///////////////////////////////////////////// DEBUG !!!!!
unsigned long lastRX;

void setup() {
  delay(555);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(15, 0);  //ROUGE
  digitalWrite(16, 1);
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
  }
  //Serial.print("startsetupbegin");
#endif
  while (!radio.begin()) {  // initialize the transceiver on the SPI bus
#ifdef DEBUG
    Serial.println("radio hardware is not responding!!");
#endif
    digitalWrite(15, 0);  //ROUGE
    digitalWrite(16, 1);
    delay(100);
    digitalWrite(15, 1);  //vert
    digitalWrite(16, 0);
    delay(100);
  }
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, address);  //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MAX);      //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();
  digitalWrite(15, 0);  //ROUGE
  digitalWrite(16, 1);
}

void loop() {
  if (millis() - lastRX > 666) {
    digitalWrite(15, 0);  //ROUGE
    digitalWrite(16, 1);
  }
  if (radio.available()) {  // is there a payload? get the pipe number that recieved it
    //uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
    radio.read(buffer, SIZE);  // fetch payload from FIFO
    for (uint8_t i = 0; i < 8; ++i) {
      if (bitRead(buffer[0], i) && !flagNote[i]) {
        usbMIDI.sendNoteOn(i + 1, 127, 7);
        flagNote[i] = 1;
      }
      if (!bitRead(buffer[0], i) && flagNote[i]) {
        usbMIDI.sendNoteOff(i + 1, 0, 7);
        flagNote[i] = 0;
      }
    }
    int souffle = (128 * buffer[1] + buffer[2]) - 8192;
    usbMIDI.sendPitchBend(souffle, 1);
    int potar = (128 * buffer[3] + buffer[4]) - 8192;
    usbMIDI.sendPitchBend(potar, 2);
    int accX = (128 * buffer[5] + buffer[6]) - 8192;
    usbMIDI.sendPitchBend(accX, 3);
    int accY = (128 * buffer[7] + buffer[8]) - 8192;
    usbMIDI.sendPitchBend(accY, 4);
    int accZ = (128 * buffer[9] + buffer[10]) - 8192;
    usbMIDI.sendPitchBend(accZ, 5);
    // int gyrX = (128 * buffer[11] + buffer[12]) - 8192;
    // usbMIDI.sendPitchBend(gyrX, 6);
    // int gyrY = (128 * buffer[13] + buffer[14]) - 8192;
    // usbMIDI.sendPitchBend(gyrY, 7);
    // int gyrZ = (128 * buffer[15] + buffer[16]) - 8192;
    // usbMIDI.sendPitchBend(gyrZ, 8);
    // int pitch = (128 * buffer[17] + buffer[18]) - 8192;
    // usbMIDI.sendPitchBend(pitch, 9);
    // int roll = (128 * buffer[19] + buffer[20]) - 8192;
    // usbMIDI.sendPitchBend(roll, 10);
    // int yaw = (128 * buffer[21] + buffer[22]) - 8192;
    // usbMIDI.sendPitchBend(yaw, 11);

    //usbMIDI.sendNoteOn(note, velocity, channel);
    digitalWrite(15, 1);  //vert
    digitalWrite(16, 0);
    lastRX = millis();
#ifdef DEBUG
    Serial.print("Received ");
    for (uint8_t j = 0; j < SIZE; ++j) {
      Serial.print(buffer[j], HEX);
      Serial.print("_");
    }
    Serial.println();
#endif
  }
}  // loop
