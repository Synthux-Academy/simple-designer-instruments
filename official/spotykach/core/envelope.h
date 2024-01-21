#pragma once

#include <stdio.h>
#include "i.envelope.h"

class Envelope: public IEnvelope {
    
public:
    Envelope();
    ~Envelope() = default;
    
    long attackLength() override { return _attackLength; };
    long decayLength() override { return _decayLength; };
    
    void setFramesPerCrossfade(long inFrames) override;
    
    float attackAttenuation(long currentFrame) override;
    float decayAttenuation(long currentFrame) override;

private:
    bool _declick;
    long _attackLength;
    long _decayLength;
    long _framesPerCrossfade;
    
    void measure();
};
