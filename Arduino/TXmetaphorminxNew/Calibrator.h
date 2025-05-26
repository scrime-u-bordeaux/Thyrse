#include "Arduino.h"
#include <EEPROM.h>

class Calibrator {
  bool initialized;
  bool calibrating;
  unsigned int nbCalSamples;
  unsigned int calSamplesCnt;
  float gyroOffsets[3];

public:
  Calibrator(unsigned int n = 1000):
  initialized(false),
  calibrating(false),
  nbCalSamples(n),
  calSamplesCnt(0) {
    gyroOffsets[0] = gyroOffsets[1] = gyroOffsets[2] = 0;
  }

  bool init(bool calibrate) {
    calibrating = calibrate;

    char header[5];
    EEPROM.get(0, header);
    if (!(header[0] == 'i' &&
          header[1] == 'n' &&
          header[2] == 'i' &&
          header[3] == 't' &&
          header[4] == '\0')) {
      EEPROM.put(0, String("init"));
      calibrating = true;
    }

    if (calibrating) {
      calSamplesCnt = 0;
      gyroOffsets[0] = gyroOffsets[1] = gyroOffsets[2] = 0;
    } else {
      for (unsigned int i = 0; i < 3; ++i) {
        EEPROM.get(5 + i * sizeof(float), gyroOffsets[i]);
      }
    }

    initialized = true;
    return calibrating;
  }

  bool processGyro(float& gx, float& gy, float& gz) {
    if (calibrating) {
      gyroOffsets[0] += gx;
      gyroOffsets[1] += gy;
      gyroOffsets[2] += gz;
      calSamplesCnt++;

      if (calSamplesCnt == nbCalSamples) {
        for (unsigned int i = 0; i < 3; ++i) {
          gyroOffsets[i] /= nbCalSamples;  
          EEPROM.put(5 + i * sizeof(float), gyroOffsets[i]);
        }
        calibrating = false;
        return true;
      }
    } else {
      gx -= gyroOffsets[0];
      gy -= gyroOffsets[1];
      gz -= gyroOffsets[2];
    }
    return false;
  }
};
