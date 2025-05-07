#include "Arduino.h"

// this code was copy / pasted from https://github.com/jremington/MPU-6050-Fusion
// then snippets were added to attempt to make it compute alternative angular representations (azi / ele)

class Mahony {
  // Free parameters in the Mahony filter and fusion scheme,
  // Kp for proportional feedback, Ki for integral
  float Kp;
  float Ki;

  float q[4];

  float pitch, roll, yaw;
  float azi, ele;

public:
  Mahony(): Kp(30), Ki(0) {
    q[0] = 1;
    q[1] = q[2] = q[3] = 0;
  }

  void update(float ax, float ay, float az, float gx, float gy, float gz, float deltat) {
    gx = gx * M_PI / 180.f;
    gy = gy * M_PI / 180.f;
    gz = gz * M_PI / 180.f;

    float recipNorm;
    float vx, vy, vz;
    float ex, ey, ez;  //error terms
    float qa, qb, qc;
    static float ix = 0.0, iy = 0.0, iz = 0.0;  //integral feedback terms
    float tmp;

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    tmp = ax * ax + ay * ay + az * az;
    if (tmp > 0.0)
    {

      // Normalise accelerometer (assumed to measure the direction of gravity in body frame)
      recipNorm = 1.0 / sqrt(tmp);
      ax *= recipNorm;
      ay *= recipNorm;
      az *= recipNorm;

      // Estimated direction of gravity in the body frame (factor of two divided out)
      vx = q[1] * q[3] - q[0] * q[2];  //to normalize these terms, multiply each by 2.0
      vy = q[0] * q[1] + q[2] * q[3];
      vz = q[0] * q[0] - 0.5f + q[3] * q[3];

      // Error is cross product between estimated and measured direction of gravity in body frame
      // (half the actual magnitude)
      ex = (ay * vz - az * vy);
      ey = (az * vx - ax * vz);
      ez = (ax * vy - ay * vx);

      // Compute and apply to gyro term the integral feedback, if enabled
      if (Ki > 0.0f) {
        ix += Ki * ex * deltat;  // integral error scaled by Ki
        iy += Ki * ey * deltat;
        iz += Ki * ez * deltat;
        gx += ix;  // apply integral feedback
        gy += iy;
        gz += iz;
      }

      // Apply proportional feedback to gyro term
      gx += Kp * ex;
      gy += Kp * ey;
      gz += Kp * ez;
    }

    // Integrate rate of change of quaternion, q cross gyro term
    deltat = 0.5 * deltat;
    gx *= deltat;   // pre-multiply common factors
    gy *= deltat;
    gz *= deltat;
    qa = q[0];
    qb = q[1];
    qc = q[2];
    q[0] += (-qb * gx - qc * gy - q[3] * gz);
    q[1] += (qa * gx + qc * gz - q[3] * gy);
    q[2] += (qa * gy - qb * gz + q[3] * gx);
    q[3] += (qa * gz + qb * gy - qc * gx);

    // renormalise quaternion
    recipNorm = 1.0 / sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    q[0] = q[0] * recipNorm;
    q[1] = q[1] * recipNorm;
    q[2] = q[2] * recipNorm;
    q[3] = q[3] * recipNorm;
  }

  void computeAngles() {
    // original pitch / roll / yaw calculus
    pitch = atan2((q[0] * q[1] + q[2] * q[3]), 0.5 - (q[1] * q[1] + q[2] * q[2]));
    roll = asin(2.0 * (q[0] * q[2] - q[1] * q[3]));
    // conventional yaw increases clockwise from North. Not that the MPU-6050 knows where North is.
    yaw = -atan2((q[1] * q[2] + q[0] * q[3]), 0.5 - (q[2] * q[2] + q[3] * q[3]));

    yaw *= 180.0 / M_PI;
    if (yaw < 0) yaw += 360.0; // compass circle
    pitch *= 180.0 / M_PI;
    roll *= 180.0 / M_PI;

    float vx = 2 * (q[1] * q[3] - q[0] * q[2]);  //to normalize these terms, multiply each by 2.0
    float vy = 2 * (q[0] * q[1] + q[2] * q[3]);
    float vz = 2 * (q[0] * q[0] - 0.5f + q[3] * q[3]);
    // or alternative (see https://forum.arduino.cc/t/math-behind-quaternion-to-gravity-conversion/430237/6)
    // float vz = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];

    azi = atan2(vx, vy);
    ele = asin(vz);

    azi *= 180.0 / M_PI;
    ele *= 180.0 / M_PI;
  }

  float getPitch() {
    return pitch;
  }

  float getRoll() {
    return roll;
  }

  float getYaw() {
    return yaw;
  }

  float getAzi() {
    return azi;
  }

  float getEle() {
    return ele;
  }
};