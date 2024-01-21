#pragma once

#include "i.source.h"
#include "i.slice.buffer.h"
#include "i.envelope.h"
#include "../fx/pitch.shift.h"

namespace blptls {
namespace spotykach {

class Slice {
public:
    Slice(ISource& inSource, ISliceBuffer& inBuffer, IEnvelope& inEnvelope);
    ~Slice() = default;
    
    
    bool isActive() { return _active; };
    bool isInactive() { return !_active; };
    void initialize();
    void activate(size_t offset, size_t length, bool reverse, float pitch, float volume);
    void synthesize(float *out0, float* out1);
    void setNeedsReset();
    
private :
    ISource& _source;
    IEnvelope& _envelope;
    ISliceBuffer& _buffer;
    PitchShift _pitch;

    bool _active;
    
    size_t _length;
    size_t _offset;
    size_t _iterator;
    bool _reverse;
    
    bool _needsReset;
    
    float *_declickIn;
    float *_declickOut;
    float _volume;
    
    void next();
};

}
}
