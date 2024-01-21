#include "MultiShapeAdsr.h"

class MorphingMultiShapeAdsr {

public:
  MorphingMultiShapeAdsr();
  ~MorphingMultiShapeAdsr();

  void Init(float sample_rate, bool adsrOrAr);
  float Process(bool gate);
  void setShapeFactor(float shapeFactorA, float shapeFactorB);
  void setAttackShapes(uint8_t shapeA, uint8_t shapeB);
  void setDecayShapes(uint8_t shapeA, uint8_t shapeB);
  void setReleaseShapes(uint8_t shapeA, uint8_t shapeB);
  void retrigger();

  void setAttackTime(float timeInS);
  void setDecayTime(float timeInS);
  void setReleaseTime(float timeInS);
  void setSustainLevel(float sustainValue);
  void setAttackStartReleaseEndLevel(float value);
private:
  MultiShapeAdsr multiShapeAdsrA;
  MultiShapeAdsr multiShapeAdsrB;

  float shapeFactorA = 0.0f;
  float shapeFactorB = 1.0f;
};