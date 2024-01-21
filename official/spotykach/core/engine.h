#pragma once

#include "i.trigger.h"
#include "i.source.h"
#include "i.envelope.h"
#include "i.generator.h"
#include "i.lfo.h"
#include "globals.h"
#include "../fx/pitch.shift.h"

namespace blptls {
namespace spotykach {

struct PlaybackParameters {
    float tempo;
    float sampleRate;
};

struct RawParameters {
    float slicePosition    = -1;
    float sliceLength      = -1;
    
    bool on                 = false;
    bool frozen             = false;
    bool reverse            = false;
};

class Engine {
public:
    Engine(ITrigger&, ISource&, IEnvelope&, IGenerator&, ILFO&);
    ~Engine() {};
    
    RawParameters rawParameters() { return _raw; }
    
    void initialize();
    
    ITrigger& trig() { 
        return _trigger; 
    }

    void set_index(int ndx) { index = ndx; }
    
    void set_slice_position(float start);
    void set_slice_length(float slice);
    
    void set_pitch_shift(float value);
    
    void set_jitter_amount(float value);
    void set_jitter_rate(float value);
    
    void set_reverse(bool reverse);

    void preprocess(PlaybackParameters p);
    
    void set_frozen(bool frozen);
    void set_antifreeze(bool value);

    void set_on_slice(SliceCallback f);

    void process(float in0, float in1, float* out0, float* out1, bool continual, bool reverse);

    void reset(bool hard);
    void clear_buffer();
    
    int index = -1;

private:
    ITrigger& _trigger;
    ISource& _source;
    IEnvelope& _envelope;
    IGenerator& _generator;
    ILFO& _jitterLFO;
    
    RawParameters _raw;
    
    bool _is_playing;
    float _tempo;
    float _start;
    float _slice;
    uint32_t _step;

    bool _invalidate_crossfade;
};
}
}
