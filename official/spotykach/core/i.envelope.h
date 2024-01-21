#pragma once

class IEnvelope {
public:
    virtual long attackLength() = 0;
    virtual long decayLength() = 0;
    
    virtual void setFramesPerCrossfade(long inFrames) = 0;
    
    virtual float attackAttenuation(long currentFrame) = 0;
    virtual float decayAttenuation(long currentFrame) = 0;
    
    virtual ~IEnvelope() {};
};
