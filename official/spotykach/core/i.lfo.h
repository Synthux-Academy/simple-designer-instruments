#pragma once

class ILFO {
public:
    virtual void setPeriod(float period) = 0;
    virtual void setFramesPerMeasure(long frames) = 0;
    virtual float triangleValue() = 0;
    virtual void advance() = 0;
};
