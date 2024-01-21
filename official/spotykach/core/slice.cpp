#include "slice.h"
#include "globals.h"

using namespace blptls;
using namespace spotykach;

Slice::Slice(ISource& inSource, ISliceBuffer& inBuffer, IEnvelope& inEnvelope) :
    _source     { inSource },
    _envelope   { inEnvelope },
    _buffer     { inBuffer },
    _active     { false },
    _length     { 0 },
    _offset     { 0 },
    _iterator   { 0 },
    _reverse    { false },
    _volume     { 1.0 }
    {}

void Slice::initialize() {
    _buffer.initialize();
    _pitch.initialize(kSampleRate, 4096);
	_pitch.setShift(0.5);
}

void Slice::activate(size_t offset, size_t length, bool reverse, float pitch, float volume) {
    if (_needsReset || offset != _offset) {
        _buffer.reset();
        _needsReset = false;
    }
    _offset = offset;
    _length = length;
    _reverse = reverse;
    _iterator = 0;
    _pitch.setShift(pitch);
    _active = true;
    _volume = volume;
}

void Slice::synthesize(float *out0, float* out1) {
    if (!_buffer.isFull()) {
        auto readPosition = 0;
        if (_reverse) {
            readPosition = _offset + _length - _buffer.writeHead();
        }
        else {
            readPosition = _offset + _buffer.writeHead();
        }
        float s0 = 0;
        float s1 = 0;
        _source.read(s0, s1, readPosition);
        _buffer.write(s0, s1);
    }
    
    auto out0Val = _buffer.read(0, _iterator);
    auto out1Val = _buffer.read(1, _iterator);
    
    auto attenuation = 1.f;
    if (_iterator < _envelope.attackLength()) {
        attenuation = _envelope.attackAttenuation(_iterator);
    }
    else if (_iterator > _length - _envelope.decayLength()) {
        attenuation = _envelope.decayAttenuation(_iterator - _length + _envelope.decayLength());
    }
    
    *out0 = out0Val * attenuation * _volume;
    *out1 = out1Val * attenuation * _volume;
    
    _pitch.process(out0, out1);

    next();
}

void Slice::next() {
    _iterator ++;
    if (_iterator == _length) {
        _active = false;
    }
}

void Slice::setNeedsReset() {
    _needsReset = true;
}
