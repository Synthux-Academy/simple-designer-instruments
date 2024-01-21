#include "envelope.h"

static const float kP      = 3.1415926535898;
static const float kP_2    = 1.5707963267949;
static const float kCoef3  = 0.16666666666667;
static const float kCoef5  = 0.00833333333333;
static const float kCoef7  = 0.00019841269841;

inline float normalize(float x) {
    if (x <= kP) return x;
    auto times = int32_t(x / kP);
    return x - (times * kP);
}

inline float sine(float x) {
    x = normalize(x);
    float x3 = x * x * x;
    float x5 = x3 * x * x;
    float x7 = x5 * x * x;
    return x - kCoef3 * x3 + kCoef5 * x5 - kCoef7 * x7;
}

Envelope::Envelope():
    _declick            { true },
    _attackLength       { 0 },
    _decayLength        { 0 },
    _framesPerCrossfade { 0 } 
{
    measure();
}


void Envelope::setFramesPerCrossfade(long inFrames) {
    _framesPerCrossfade = inFrames;
    measure();
}

void Envelope::measure() {
    if (_framesPerCrossfade > 0) {
        _attackLength = _decayLength = _framesPerCrossfade;
    }
    else {
        _attackLength = _decayLength = _declick ? 512 : 0;
    }
}

float Envelope::attackAttenuation(long currentFrame) {
    return _attackLength > 0 ? sine(kP_2 * currentFrame / _attackLength) : 1;
}

float Envelope::decayAttenuation(long currentFrame) {
    return _decayLength > 0 ? sine(kP_2 + kP_2 * currentFrame / _decayLength) : 1;
}
