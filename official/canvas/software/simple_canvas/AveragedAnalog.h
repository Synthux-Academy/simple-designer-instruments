#ifndef AVERAGED_ANALOG_H
#define AVERAGED_ANALOG_H

#include "Arduino.h"

//AVERAGE_LEN should be a multiple of 2 (+2 because we always remove min and max) to optimize the division
#define AVERAGE_LEN (16 + 2)

class AveragedAnalog {
public:
  AveragedAnalog(bool useFloatAverage = false) {
    this->initArray();
    thersholdValue = 2;
    this->useFloatAverage = useFloatAverage;
  }
  AveragedAnalog(uint32_t thersholdValue, bool useFloatAverage = false) {
    this->initArray();
    this->thersholdValue = thersholdValue;
    this->useFloatAverage = useFloatAverage;
  }
  ~AveragedAnalog() {}

  void updateValue(uint32_t newValue);
  bool hasValueUpdated();
  uint32_t getVal();
  float getFVal();
private:
  void initArray();
  uint32_t potValues[AVERAGE_LEN];
  uint8_t potValueIndex = 0;
  uint32_t potVal = 0;
  uint32_t potValOld = 0;

  bool useFloatAverage;
  float fPotVal = 0.0f;
  float fPotValOld = 0.0f;

  uint32_t thersholdValue;
  bool initialized = false;
};


#endif  // AVERAGED_ANALOG_H