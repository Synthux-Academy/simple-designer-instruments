#pragma once

#include "i.lfo.h"

class LFO: public ILFO {
public:
    LFO();
    
    void setPeriod(float) override;
    void setFramesPerMeasure(long) override;
    void advance() override;
    float triangleValue() override;
    
private:
    long _frame;
    long _framesPerMeasure;
    float _period;
 };
