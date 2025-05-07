#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
// #include <MadgwickAHRS.h>
#include <Wire.h>
#include "Calibrator.h"
#include "Mahony.h"
// #include <MPU6050_tockn.h>
// MPU6050 mpu6050(Wire);

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
Adafruit_MPU6050 mpu;
Calibrator calibrator;

// this doesn't seem to have any effect
#undef betaDef
#define betaDef 0.1f

// OnePole filter (thanks EarLevel Engineering !) /////////////////////////////
class OnePole {
  bool hipass;
  double a0, b1, z1;

public:
  OnePole(bool hip = false): hipass(hip), a0(1), b1(0), z1(0) {}
  OnePole(double f, bool hip = false): OnePole(hip) { setFc(f); }
  ~OnePole() {}

  void setFc(double f) {
    b1 = exp(-2.0 * M_PI * f);
    a0 = 1.0 - b1;
  }

  float process(float v) {
    z1 = v * a0 + z1 * b1;
    return hipass ? (v - z1) : z1;
  }
};
///////////////////////////////////////////////////////////////////////////////

unsigned long measureInterval = 1000000 / 500; // microseconds
unsigned long lastMeasureDate = 0; // microseconds
unsigned long midiOutInterval = 20; // milliseconds
unsigned long lastMidiOutDate = 0; // milliseconds

float RAD2DEG = 180 / M_PI;
float sr = 50; // Hz
float fc = 5 / sr; // Hz
OnePole hipass[3];
// Madgwick filter;
Mahony mahony;
float ax, ay, az;
float gx, gy, gz;
float p, r, y;
int accX, accY, accZ;
int gyrX, gyrY, gyrZ;
int pitch, roll, yaw;
bool buttons[7];

RF24 radio(14, 10);            // using pin 14 for the CE pin, and pin 10 for the CSN pin
uint8_t address[6] = "00001";  // Let these addresses be used for the pair
float payload = 0.0;
#define SIZE 32
char buffer[SIZE];
#define sck 8
#define din 7
long tval = 0;
long val = 0;

// #define DEBUG  // !!!!!!!!!!!!!!!!!!!!!!!!DEBUG SERIAL

void setup() {
  hipass[0] = OnePole(fc, true);
  hipass[1] = OnePole(fc, true);
  hipass[2] = OnePole(fc, true);

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
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
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
    Serial.println("radio hardware is not responding");
#endif
  }

  digitalWrite(0, 0);  //ROUGE
  digitalWrite(1, 1);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);  //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MAX);   //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();

  // mpu6050.begin();
  // mpu6050.calcGyroOffsets(true);

  if (!mpu.begin()) {
#ifdef DEBUG
    Serial.println("Failed to find MPU6050 chip");
#endif
    while (1) {
      delay(10);
    }
  }

  // filter.begin(500);

  // valid ranges : 2, 4, 8 and 16 G
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  // valid ranges : 250, 500, 1000 and 2000 DPS
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // disable all filters
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ); // disables the lowpass filter
  mpu.setHighPassFilter(MPU6050_HIGHPASS_DISABLE);
  //mpu.setCycleRate(MPU6050_CYCLE_40_HZ);
  mpu.enableCycle(false);

  readButtons();
  bool c = calibrator.init(buttons[0]);
#ifdef DEBUG
  Serial.println(c ? "starting calibration" : "no calibration");
#endif
  sread(27);

  for (int i = 0; i < 100; i++) {  // Calibration pression souffle
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

void readButtons() {
  buttons[0] = !digitalRead(21);
  buttons[1] = !digitalRead(20);
  buttons[2] = !digitalRead(17);
  buttons[3] = !digitalRead(16);
  buttons[4] = !digitalRead(15);
  buttons[5] = !digitalRead(3);
  buttons[6] = !digitalRead(2);
}

void loop() {
  unsigned long nowMicros = micros();

  float deltat = (nowMicros - lastMeasureDate) * 1e-6f;
  if (nowMicros - lastMeasureDate < measureInterval) return;
  lastMeasureDate = nowMicros;

  readButtons();
  buffer[0] = 0;
  for (unsigned int i = 0; i < 7; ++i) {
    buffer[0] += buttons[i] << i;
  }

  // buffer[0] = !digitalRead(21) + 2 * !digitalRead(15) + 4 * !digitalRead(16) + 8 * !digitalRead(17) + 16 * !digitalRead(20);

  // contrôleur souffle
  if (digitalRead(din) == LOW) {
    val = ((sread(27) - tval) / 1024) + 8192;
    val = constrain(val, 0, 16383);
  }
  uint16_t souffle = val;
  buffer[1] = souffle >> 7;
  buffer[2] = souffle & 127;

  // potentiomètre gros
  uint16_t potargros = analogRead(A8);
  buffer[3] = potargros >> 7;
  buffer[4] = potargros & 127;

  // potentiomètre petit
  uint16_t potarpetit = analogRead(A14);
  buffer[5] = potarpetit >> 7;
  buffer[6] = potarpetit & 127;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  ax = a.acceleration.x / 9.81;
  ay = a.acceleration.y / 9.81;
  az = a.acceleration.z / 9.81;
  gx = g.gyro.x * RAD2DEG;
  gy = g.gyro.y * RAD2DEG;
  gz = g.gyro.z * RAD2DEG;
  /*
  gx = hipass[0].process(g.gyro.x * RAD2DEG);
  gy = hipass[1].process(g.gyro.y * RAD2DEG);
  gz = hipass[2].process(g.gyro.z * RAD2DEG);
  //*/

  // mpu6050.update();
  // ax = mpu6050.getAccX();
  // ay = mpu6050.getAccY();
  // az = mpu6050.getAccZ();
  // gx = mpu6050.getGyroX();
  // gy = mpu6050.getGyroY();
  // gz = mpu6050.getGyroZ();

  if (calibrator.processGyro(gx, gy, gz)) {
#ifdef DEBUG
    Serial.println("calibration performed");
#endif
  }

  // filter.updateIMU(gz, gx, gy, az, ax, ay);
  mahony.update(az, ax, ay, gz, gx, gy, deltat);

  unsigned now = millis();
  if (now - lastMidiOutDate >= midiOutInterval) {
    lastMidiOutDate = now;

    // Serial.print(filter.getRoll());
    // Serial.print("\t");
    // Serial.println(filter.getPitch());

    accX = constrain((ax + 2) * 4096, 0, 16383);
    accY = constrain((ay + 2) * 4096, 0, 16383);
    accZ = constrain((az + 2) * 4096, 0, 16383);
    buffer[7] = accX >> 7;
    buffer[8] = accX & 127;
    buffer[9] = accY >> 7;
    buffer[10] = accY & 127;
    buffer[11] = accZ >> 7;
    buffer[12] = accZ & 127;

    gyrX = constrain((gx / 500 + 1) * 8192, 0, 16383);
    gyrY = constrain((gy / 500 + 1) * 8192, 0, 16383);
    gyrZ = constrain((gz / 500 + 1) * 8192, 0, 16383);
    buffer[13] = gyrX >> 7;
    buffer[14] = gyrX & 127;
    buffer[15] = gyrY >> 7;
    buffer[16] = gyrY & 127;
    buffer[17] = gyrZ >> 7;
    buffer[18] = gyrZ & 127;

    // p = filter.getPitch();
    // r = filter.getRoll();
    // y = filter.getYaw();
    mahony.computeAngles();
    p = mahony.getAzi(); //mahony.getPitch();
    r = mahony.getEle(); //mahony.getRoll();
    y = 0;
    pitch = constrain((p / 90 + 1) * 8192, 0, 16383);
    roll = constrain((r / 180 + 1) * 8192, 0, 16383);
    yaw = constrain((y / 180 + 1) * 8192, 0, 16383);
    buffer[19] = pitch >> 7;
    buffer[20] = pitch & 127;
    buffer[21] = roll >> 7;
    buffer[22] = roll & 127;
    buffer[23] = yaw >> 7;
    buffer[24] = yaw & 127;

  #ifdef DEBUG
    Serial.print(p);
    Serial.print("\t");
    Serial.print(r);
    Serial.print("\t");
    Serial.println(y);

    // Serial.println("souffle :");
    // Serial.print(souffle);
    // Serial.println("accel X/Y/Z :");
    // Serial.print(accX);
    // Serial.print(",");
    // Serial.print(accY);
    // Serial.print(",");
    // Serial.println(accZ);
    // Serial.println("buttons :");
    // Serial.println(buffer[0], BIN);
    // Serial.print(digitalRead(15));
    // Serial.print(digitalRead(16));
    // Serial.print(digitalRead(17));
    // Serial.print(digitalRead(20));
    // Serial.print(digitalRead(21));//Serial.print(",");
    // Serial.println("****************");
  #endif

    bool report = radio.write(&buffer, SIZE);
    if (report) { // payload was delivered
  #ifdef DEBUG
      // Serial.println("successful! ");
  #endif
      digitalWrite(0, 1); // VERT
      digitalWrite(1, 0);
    } else { // payload was not delivered
  #ifdef DEBUG
      // Serial.println("Transmission failed or timed out");
  #endif
      digitalWrite(0, 0); // ROUGE
      digitalWrite(1, 1);
    }
  }
}
