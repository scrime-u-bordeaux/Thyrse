#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <Wire.h>
#include <TTP229.h>
TTP229 ttp229;
#include "SparkFun_MMA8452Q.h"
MMA8452Q accel;
byte accX, accY, accZ;
RF24 radio(14, 10); // using pin 14 for the CE pin, and pin 10 for the CSN pin
#define SIZE 32 
// Let these addresses be used for the pair
uint8_t address[6] = "00001";
float payload = 0.0;
char buffer[SIZE]; 

void setup() {
  delay(555);
  Wire.begin();
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  Serial.begin(115200);

while (!radio.begin()) {
    digitalWriteFast(8, 0); //ROUGE
  digitalWriteFast(9, 1);
    Serial.println("radio hardware is not responding!!");
  delay(100);
    digitalWriteFast(8, 1); //vert
  digitalWriteFast(9, 0);
  delay(100);
  }
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();


  while (accel.begin() == false) {
    digitalWrite(LED_BUILTIN, 0);
    delay(50);
    digitalWrite(LED_BUILTIN, 1);
    delay(100);
  }
  accel.setScale(SCALE_4G);
  digitalWriteFast(8, 1); //vert
  digitalWriteFast(9, 0);
  delay(222);
   //Serial.print("startsetupend");
} // setup

void loop() {
  ttp229.readKeys();
    for (int i = 0; i < 16; ++i) {
        buffer[i]=ttp229.isKeyPress(i);
    }
  if (accel.available()) {      // Wait for new data from accelerometer
    // Acceleration of x, y, and z directions in g units
    accX = (accel.getCalculatedX()+2)*32;
    accY = (accel.getCalculatedY()+2)*32;
    accZ = (accel.getCalculatedZ()+2)*32;
   buffer[16]=accX;
   buffer[17]=accY;
   buffer[18]=accZ;
  }
  bool report = radio.write(&buffer, SIZE);
//  unsigned long start_timer = micros();                    // start the timer
//  bool report = radio.write(&payload, sizeof(float));      // transmit & save the report
//  unsigned long end_timer = micros();                      // end the timer

  if (report) {
    Serial.println("successful! ");          // payload was delivered
//    Serial.print("Time to transmit = ");
//    Serial.print(end_timer - start_timer);                 // print the timer result
//    Serial.print(" us. Sent: ");
//    Serial.println(payload);                               // print payload sent
    //payload += 0.01; // increment float payload
    digitalWrite(8, 1); ////VERT
    digitalWrite(9, 0);
  }
  else {
    Serial.println("Transmission failed or timed out");
    digitalWrite(8, 0); //ROUGE
    digitalWrite(9, 1); // payload was not delivered
  }
  // to make this example readable in the serial monitor
  delay(20);  // slow transmissions down by 1 second
 //Serial.println("top"); 
} // loop
