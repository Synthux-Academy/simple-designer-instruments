#include "MorphingMultiShapeAdsr.h"

MorphingMultiShapeAdsr::MorphingMultiShapeAdsr() {}
MorphingMultiShapeAdsr::~MorphingMultiShapeAdsr() {}

void MorphingMultiShapeAdsr::Init(float sample_rate, bool adsrOrAr) {
  this->multiShapeAdsrA.Init(sample_rate, adsrOrAr);
  this->multiShapeAdsrB.Init(sample_rate, adsrOrAr);
}

void MorphingMultiShapeAdsr::setShapeFactor(float shapeFactorA, float shapeFactorB) {
  this->shapeFactorA = shapeFactorA;
  this->shapeFactorB = shapeFactorB;
}
float MorphingMultiShapeAdsr::Process(bool gate) {
  return this->multiShapeAdsrA.Process(gate)*this->shapeFactorA + this->multiShapeAdsrB.Process(gate)*this->shapeFactorB;
}

void MorphingMultiShapeAdsr::setAttackShapes(uint8_t shapeA, uint8_t shapeB) {
  this->multiShapeAdsrA.setAttackShape(shapeA);
  this->multiShapeAdsrB.setAttackShape(shapeB);
}

void MorphingMultiShapeAdsr::setDecayShapes(uint8_t shapeA, uint8_t shapeB) {
  this->multiShapeAdsrA.setDecayShape(shapeA);
  this->multiShapeAdsrB.setDecayShape(shapeB);
}
void MorphingMultiShapeAdsr::setReleaseShapes(uint8_t shapeA, uint8_t shapeB) {
  this->multiShapeAdsrA.setReleaseShape(shapeA);
  this->multiShapeAdsrB.setReleaseShape(shapeB);
}

void MorphingMultiShapeAdsr::retrigger() {
  this->multiShapeAdsrA.retrigger();
  this->multiShapeAdsrB.retrigger();
}

void MorphingMultiShapeAdsr::setAttackTime(float timeInS) {
  this->multiShapeAdsrA.setAttackTime(timeInS);
  this->multiShapeAdsrB.setAttackTime(timeInS);
}
void MorphingMultiShapeAdsr::setDecayTime(float timeInS) {
  this->multiShapeAdsrA.setDecayTime(timeInS);
  this->multiShapeAdsrB.setDecayTime(timeInS);
}
void MorphingMultiShapeAdsr::setReleaseTime(float timeInS) {
  this->multiShapeAdsrA.setReleaseTime(timeInS);
  this->multiShapeAdsrB.setReleaseTime(timeInS);
}
void MorphingMultiShapeAdsr::setSustainLevel(float sustainValue) {
  this->multiShapeAdsrA.setSustainLevel(sustainValue);
  this->multiShapeAdsrB.setSustainLevel(sustainValue);
}
void MorphingMultiShapeAdsr::setAttackStartReleaseEndLevel(float value) {
  this->multiShapeAdsrA.setAttackStartReleaseEndLevel(value);
  this->multiShapeAdsrB.setAttackStartReleaseEndLevel(value);
}