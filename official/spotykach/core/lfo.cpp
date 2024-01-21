//
//  LFO.cpp
//  spotykach_core
//
//  Created by Vladyslav Lytvynenko on 21.06.22.
//

#include "lfo.h"
#include <cmath>

//"Note that in many programming languages, the % operator
//is a remainder operator (with result the same sign as the dividend),
//not a modulo operator; the modulo operation can be obtained by
//using ((x % p) + p) % p in place of x % p."
//https://en.wikipedia.org/wiki/Triangle_wave
template<typename T>
inline float mod(T a, T b) {
    return ((a % b) + b) % b;
}

LFO::LFO(): 
    _frame              { 0 },
    _framesPerMeasure   { 0 },
    _period             { 0.25 } // 1/4 
    {}

void LFO::setPeriod(float value) {
    _period = std::fmin(std::fmax(value, 0.03125), 1.); // 1/32...1
}

void LFO::setFramesPerMeasure(long framesPerMeasure) {
    _framesPerMeasure = framesPerMeasure;
}

void LFO::advance() {
    _frame = (_frame + 1) % _framesPerMeasure;
}

float LFO::triangleValue() {
    long fp = _framesPerMeasure * _period / 2;
    return (2.0 / fp) * (fp - std::abs(mod(_frame - fp, 2 * fp) - fp)) - 1.0;
}
