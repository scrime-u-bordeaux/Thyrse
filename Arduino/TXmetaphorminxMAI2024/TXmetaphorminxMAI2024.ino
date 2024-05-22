#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <Wire.h>
#include <MPU6050_tockn.h>
MPU6050 mpu6050(Wire);
int accX, accY, accZ;
int gyrX, gyrY, gyrZ;
RF24 radio(14, 10);            // using pin 14 for the CE pin, and pin 10 for the CSN pin
uint8_t address[6] = "00001";  // Let these addresses be used for the pair
float payload = 0.0;
#define SIZE 32
char buffer[SIZE];
#define sck 8
#define din 7
long tval = 0;
long val = 0;
//#define DEBUG  // !!!!!!!!!!!!!!!!!!!!!!!!DEBUG SERIAL

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
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
    digitalWrite(0, 0);  //ROUGE
    digitalWrite(1, 1);
    delay(100);
    digitalWrite(0, 1);  //vert
    digitalWrite(1, 0);
    delay(100);
#ifdef DEBUG
    Serial.println("radio hardware is not responding!!");
#endif
  }
  digitalWrite(0, 0);  //ROUGE
  digitalWrite(1, 1);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);  //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MAX);   //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();

  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  sread(27);
  for (int i = 0; i < 100; i++) {  //Calibration pression souffle
    while (digitalRead(din) == HIGH) {
      delayMicroseconds(1);
    }
    tval = tval + sread(27);
  }
  tval = tval / 100;
  digitalWrite(0, 1);  //vert
  digitalWrite(1, 0);
  delay(50);
}
long sread(int p) {
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
  buffer[0] = !digitalRead(21) + 2 * !digitalRead(15) + 4 * !digitalRead(16) + 8 * !digitalRead(17) + 16 * !digitalRead(20);
  uint16_t potar = analogRead(A8);  //joyX == breathControler
  if (digitalRead(din) == LOW) {
    val = ((sread(27) - tval) / 1024) + 8192;
    val = constrain(val, 0, 16383);
  }
  uint16_t souffle = val;  /// controlleu souffle
  buffer[1] = souffle >> 7;
  buffer[2] = souffle & 127;

  buffer[3] = potar >> 7;
  buffer[4] = potar & 127;
  mpu6050.update();
  accX = (mpu6050.getAccX() + 2) * 4096;
  accY = (mpu6050.getAccY() + 2) * 4096;
  accZ = (mpu6050.getAccZ() + 2) * 4096;
  buffer[5] = accX >> 7;
  buffer[6] = accX & 127;
  buffer[7] = accY >> 7;
  buffer[8] = accY & 127;
  buffer[9] = accZ >> 7;
  buffer[10] = accZ & 127;
  gyrX = (mpu6050.getGyroX() + 500) * 16;
  gyrY = (mpu6050.getGyroY() + 500) * 16;
  gyrZ = (mpu6050.getGyroZ() + 500) * 16;
  gyrX = constrain(gyrX, 0, 16383);
  gyrY = constrain(gyrY, 0, 16383);
  gyrZ = constrain(gyrZ, 0, 16383);
  buffer[11] = gyrX >> 7;
  buffer[12] = gyrX & 127;
  buffer[13] = gyrY >> 7;
  buffer[14] = gyrY & 127;
  buffer[15] = gyrZ >> 7;
  buffer[16] = gyrZ & 127;
#ifdef DEBUG
  Serial.println("souffle :");
  Serial.print(souffle);
  Serial.println("accel X/Y/Z :");
  Serial.print(accX);
  Serial.print(",");
  Serial.print(accY);
  Serial.print(",");
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

  bool report = radio.write(&buffer, SIZE);
  if (report) {  // payload was delivered
#ifdef DEBUG
    Serial.println("successful! ");
#endif
    digitalWrite(0, 1);  ////VERT
    digitalWrite(1, 0);
  } else {
#ifdef DEBUG
    //Serial.println("Transmission failed or timed out");
#endif
    digitalWrite(0, 0);  //ROUGE
    digitalWrite(1, 1);  // payload was not delivered
  }
  delay(20);  // slow transmissions down
}
