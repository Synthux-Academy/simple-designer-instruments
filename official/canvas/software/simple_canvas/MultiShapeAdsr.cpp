#include "MultiShapeAdsr.h"
#include "Arduino.h"

void MultiShapeAdsr::Init(float sampleRate, bool adsrOrAr) {
  this->adsrOrAr = adsrOrAr;
  this->sampleRate = sampleRate;
  this->sampleTime = 1 / sampleRate;
  if (this->adsrOrAr)  //true = adsr, false = ar
    this->setSustainLevel(0.7f);
  else
    this->setSustainLevel(1.0f);
}
float MultiShapeAdsr::Process(bool gate) {

  this->indexTimer++;
  if (this->currentStatus == MULTISHAPE_ADSR_SEG_IDLE && gate == true) {
    this->currentStatus = MULTISHAPE_ADSR_SEG_ATTACK;
    this->indexTimer = 0;
  }

  switch (this->currentStatus) {
    case MULTISHAPE_ADSR_SEG_ATTACK:
      {
        //(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float startRise
        this->currentOutput = this->interpolateRisingSlope(this->attackShape, this->indexTimer, this->attackSampleCount, this->attackRange, this->attackStart);
        if (this->indexTimer > this->attackSampleCount - 1) {
          if (this->adsrOrAr)  //true = adsr, false = ar
            this->currentStatus = MULTISHAPE_ADSR_SEG_DECAY;
          else
            this->currentStatus = MULTISHAPE_ADSR_SEG_RELEASE;
          this->indexTimer = 0;
        }
        break;
      }
    case MULTISHAPE_ADSR_SEG_DECAY:
      {
        //(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float endFall)
        this->currentOutput = this->interpolateFallingSlope(this->decayShape, this->indexTimer, this->decaySampleCount, this->decayRange, this->decayEnd);
        if (this->indexTimer > this->decaySampleCount - 1) {
          this->currentStatus = MULTISHAPE_ADSR_SEG_SUSTAIN;
          this->indexTimer = 0;
        }
        break;
      }
    case MULTISHAPE_ADSR_SEG_SUSTAIN:
      {
        //do nothing about current output;
        if (gate == false) {
          this->currentStatus = MULTISHAPE_ADSR_SEG_RELEASE;
          this->indexTimer = 0;
        }
        break;
      }
    case MULTISHAPE_ADSR_SEG_RELEASE:
      {
        //(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float endFall)
        this->currentOutput = this->interpolateFallingSlope(this->releaseShape, this->indexTimer, this->releaseSampleCount, this->releaseRange, this->releaseEnd);
        if (this->indexTimer > this->releaseSampleCount - 1) {
          this->currentStatus = MULTISHAPE_ADSR_SEG_IDLE;
          this->indexTimer = 0;
        }
        break;
      }
  }

  /*Serial.print("this->indexTimer ");
  Serial.println(this->indexTimer);*/

  if (this->currentOutput < 0.0f)
    this->currentOutput = 0.0f;
  if (this->currentOutput > 1.0f)
    this->currentOutput = 1.0f;

  return this->currentOutput;
}
void MultiShapeAdsr::setAttackShape(uint8_t shape) {
  this->attackShape = shape;
}
void MultiShapeAdsr::setDecayShape(uint8_t shape) {
  this->decayShape = shape;
}
void MultiShapeAdsr::setReleaseShape(uint8_t shape) {
  this->releaseShape = shape;
}
void MultiShapeAdsr::retrigger() {
  this->currentStatus = MULTISHAPE_ADSR_SEG_ATTACK;
  this->indexTimer = 0;
  this->currentOutput = 0.0f;
}

void MultiShapeAdsr::setAttackTime(float timeInS) {
  this->attackTime = timeInS;
  this->attackSampleCount = this->attackTime * this->sampleRate;
  /*Serial.print("attackTime ");
  Serial.println(attackTime);
  Serial.print(" attackSampleCount ");
  Serial.println(attackSampleCount);*/
}
void MultiShapeAdsr::setDecayTime(float timeInS) {
  this->decayTime = timeInS;
  this->decaySampleCount = this->decayTime * this->sampleRate;
  /*Serial.print("decayTime ");
  Serial.println(decayTime);
  Serial.print(" decaySampleCount ");
  Serial.println(decaySampleCount);*/
}
void MultiShapeAdsr::setReleaseTime(float timeInS) {
  this->releaseTime = timeInS;
  this->releaseSampleCount = this->releaseTime * this->sampleRate;
  /*
  Serial.print("releaseTime ");
  Serial.println(releaseTime);
  Serial.print(" releaseSampleCount ");
  Serial.println(releaseSampleCount);*/
}

void MultiShapeAdsr::setAttackStartReleaseEndLevel(float value) {
  if (value > 1.0f)
    value = 1.0f;
    
  this->attackStart = value;
  this->releaseEnd = value;

  this->attackRange = this->attackEnd - this->attackStart;
  this->decayRange = this->decayStart - this->decayEnd;
  this->releaseRange = this->releaseStart - this->releaseEnd;
}

void MultiShapeAdsr::setSustainLevel(float sustainValue) {
  this->attackStart = 0.0f;
  this->attackEnd = 1.0f;
  this->decayStart = 1.0f;
  this->decayEnd = sustainValue;
  this->releaseStart = sustainValue;
  this->releaseEnd = 0.0f;

  this->attackRange = this->attackEnd - this->attackStart;
  this->decayRange = this->decayStart - this->decayEnd;
  this->releaseRange = this->releaseStart - this->releaseEnd;
  /*
  Serial.print(" attackStart ");
  Serial.print(attackStart);
  Serial.print(" attackEnd ");
  Serial.print(attackEnd);
  Serial.print(" decayStart ");
  Serial.print(decayStart);
  Serial.print(" decayEnd ");
  Serial.print(decayEnd);
  Serial.print(" releaseStart ");
  Serial.print(releaseStart);
  Serial.print(" releaseEnd ");
  Serial.print(releaseEnd);
  Serial.print(" attackRange ");
  Serial.print(attackRange);
  Serial.print(" decayRange ");
  Serial.print(decayRange);
  Serial.print(" releaseRange ");
  Serial.println(releaseRange);*/
}

float MultiShapeAdsr::interpolateRisingSlope(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float startRise) {
  switch (slopeShape) {
    case LINEAR_SHAPE:
      return (indexTimer * (1.0f / nSample)) * range + startRise;
    case QUADRATIC_SHAPE:
      return ((cubeFloat(((float)indexTimer / (float)nSample))) * range + startRise);
    case QUADRATIC_INVERT_SHAPE:
      return ((1.0f - cubeFloat(((float)(nSample - indexTimer) / (float)(nSample))))) * range + startRise;
    case LOGISTIC_SHAPE:
      return (1.0f - 1.0f / (1.0f + expf(-5.0f + (indexTimer / (nSample / 10.0f))))) * range + startRise;
    default:
      return 0.0f;
  }
}

float MultiShapeAdsr::interpolateFallingSlope(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float endFall) {
  switch (slopeShape) {
    case LINEAR_SHAPE:
      return (1.0f - indexTimer * (1.0f / nSample)) * range + endFall;
    case QUADRATIC_SHAPE:
      return ((cubeFloat((float)(nSample - indexTimer) / (float)(nSample))) * range + endFall);
    case QUADRATIC_INVERT_SHAPE:
      return ((1.0f - cubeFloat(((float)(indexTimer) / (float)(nSample)))) * range + endFall);
    case LOGISTIC_SHAPE:
      return (1.0f / (1.0f + expf(-5.0f + (indexTimer / (nSample / 10.0f))))) * range + endFall;
    default:
      return 0.0f;
  }
}
float squareFloat(float x) {
  return x * x;
}
float cubeFloat(float x) {
  return x * x * x;
}
