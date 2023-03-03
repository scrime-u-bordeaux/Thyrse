#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <Wire.h>
#include "SparkFun_MMA8452Q.h"
MMA8452Q accel;
int accX, accY, accZ;
RF24 radio(14, 10); // using pin 14 for the CE pin, and pin 10 for the CSN pin
uint8_t address[6] = "00001";// Let these addresses be used for the pair
float payload = 0.0;
#define sck 8
#define din 7

long tval = 0;
long val = 0;
#define SIZE 16
char buffer[SIZE];
//#define DEBUG // !!!!!!!!!!!!!!!!!!!!!!!!DEBUG SERIAL

void setup() {
#ifdef DEBUG
  unsigned long ulngStart = millis();
  Serial.begin(115200);
  while ( !Serial && ((millis() - ulngStart) <= 5000) ) {
    ;
  }
#endif
  delay(100);
  analogReadRes(14);
  Wire.begin();
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  pinMode(sck, OUTPUT);
  pinMode(din, INPUT_PULLUP);
  digitalWrite(sck, HIGH);
  while (!radio.begin()) {
    digitalWrite(0, 0); //ROUGE
    digitalWrite(1, 1);
    delay(100);
    digitalWrite(0, 1); //vert
    digitalWrite(1, 0);
    delay(100);
#ifdef DEBUG
    Serial.println("radio hardware is not responding!!");
#endif
  }
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MAX);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();
  accel.begin();
  while (accel.begin() == false) { //l'addresse a changée???? accel.begin(Wire, 0x1C) ==> PULLUP!!
    digitalWrite(0, 0); //NOIR
    digitalWrite(1, 0);
    delay(100);
    digitalWrite(0, 0); //ROUGE
    digitalWrite(1, 1);
    delay(100);
  }
  accel.setScale(SCALE_2G);
  digitalWrite(0, 0); //ROUGE
  digitalWrite(1, 1);
  sread(27);
  for (int i = 0; i < 100; i++) { //Calibration pression souffle
    while (digitalRead(din) == HIGH)
    {
      delayMicroseconds(1);
    }
    tval = tval + sread(27);
  }
  tval = tval / 100;
  digitalWrite(0, 1); //vert
  digitalWrite(1, 0);
  delay(50);
}

long sread(int p)
{
  long rval = 0;
  for (int x = 0; x < p; x++) {
    digitalWrite(sck, HIGH);
    delayMicroseconds(1);
    if (x < 24) {
      rval <<= 1;
      rval = rval + (digitalRead(din));
    }
    digitalWrite(sck, LOW);
    delayMicroseconds(1);
  }
  if (rval > 8388607) {
    rval = rval - 16777216;
  }
  return rval;
}

void loop() {
  buffer[0] = !digitalRead(21) + 2 * !digitalRead(20) + 4 * !digitalRead(17) + 8 * !digitalRead(16) + 16 * !digitalRead(15);
  uint16_t joyX = analogRead(A8);
  if (digitalRead(din) == LOW)
  {
    val = ((sread(27) - tval) / 1024) + 8192;
    val = constrain(val, 0, 16383);
  }
  uint16_t souffle = val;/// controlleu souffle
  buffer[1] = souffle >> 7;
  buffer[2] = souffle & 127;
  buffer[3] = joyX >> 7;
  buffer[4] = joyX & 127;
  if (accel.available()) {// Wait for new data from accelerometer
    accX = (accel.getX() + 2048) * 4;
    accY = (accel.getY() + 2048) * 4;
    accZ = (accel.getZ() + 2048) * 4;
    buffer[5] = accX >> 7;
    buffer[6] = accX & 127;
    buffer[7] = accY >> 7;
    buffer[8] = accY & 127;
    buffer[9] = accZ >> 7;
    buffer[10] = accZ & 127;
#ifdef DEBUG
    //Serial.println("accel X/Y/Z :");
    Serial.print(accX); Serial.print(",");
    Serial.print(accY); Serial.print(",");
    Serial.println(accZ);
    //    Serial.println("joy :");
    //    Serial.println(analogRead(A8));
    //    Serial.println(analogRead(A9));
    //    Serial.println("buttons :");
    //    Serial.println(buffer[0], BIN);
    //    Serial.print(digitalRead(15));
    //    Serial.print(digitalRead(16));
    //    Serial.print(digitalRead(17));
    //    Serial.print(digitalRead(20));
    //    Serial.print(digitalRead(21));//Serial.print(",");
    //Serial.println("****************");
#endif
  }
  bool report = radio.write(&buffer, SIZE);
  if (report) {// payload was delivered
#ifdef DEBUG
    Serial.println("successful! ");
#endif
    digitalWrite(0, 1); ////VERT
    digitalWrite(1, 0);
  }
  else {
#ifdef DEBUG
    //Serial.println("Transmission failed or timed out");
#endif
    digitalWrite(0, 0); //ROUGE
    digitalWrite(1, 1); // payload was not delivered
  }
  delay(20);  // slow transmissions down
}
