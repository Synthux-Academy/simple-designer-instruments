#ifndef MULTI_SHAPE_ADSR_H
#define MULTI_SHAPE_ADSR_H

#include "DaisyDuino.h"
using namespace daisysp;

enum CURVE_SHAPE {
  LINEAR_SHAPE,
  QUADRATIC_SHAPE,
  QUADRATIC_INVERT_SHAPE,
  LOGISTIC_SHAPE
};

enum ADSR_STATUS {
  MULTISHAPE_ADSR_SEG_IDLE = 0,
  MULTISHAPE_ADSR_SEG_ATTACK = 1,
  MULTISHAPE_ADSR_SEG_DECAY = 2,
  MULTISHAPE_ADSR_SEG_SUSTAIN = 3,
  MULTISHAPE_ADSR_SEG_RELEASE = 4
};

class MultiShapeAdsr {
public:
  MultiShapeAdsr() {}
  ~MultiShapeAdsr() {}
  void Init(float sample_rate, bool adsrOrAr);
  float Process(bool gate);
  void setAttackShape(uint8_t shape);
  void setDecayShape(uint8_t shape);
  void setReleaseShape(uint8_t shape);
  void retrigger();

  void setAttackTime(float timeInS);
  void setDecayTime(float timeInS);
  void setReleaseTime(float timeInS);
  void setSustainLevel(float sustainValue);
  void setAttackStartReleaseEndLevel(float value);
  private:
    bool adsrOrAr;
    float currentOutput = 0.0f;
    uint8_t currentStatus = MULTISHAPE_ADSR_SEG_IDLE;
    uint32_t indexTimer = 0;

    float sampleRate;
    float sampleTime;

    uint8_t attackShape = LINEAR_SHAPE;
    uint8_t decayShape = LINEAR_SHAPE;
    uint8_t releaseShape = LINEAR_SHAPE;
    uint8_t mode = MULTISHAPE_ADSR_SEG_IDLE;

    float attackTime;
    float decayTime;
    float releaseTime;

    float attackStart;
    float attackEnd;
    float attackRange;

    float decayStart;
    float decayEnd;
    float decayRange;

    float releaseStart;
    float releaseEnd;
    float releaseRange;

    uint32_t attackSampleCount;
    uint32_t decaySampleCount;
    uint32_t releaseSampleCount;

    float interpolateRisingSlope(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float startRise);
    float interpolateFallingSlope(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float endFall);
  };

  float squareFloat(float x);
  float cubeFloat(float x);
#endif  // MULTI_SHAPE_ADSR_H