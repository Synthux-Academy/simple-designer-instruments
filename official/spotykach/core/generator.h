#pragma once

#include "i.generator.h"
#include "i.source.h"
#include "i.envelope.h"
#include "i.lfo.h"
#include "slice.h"
#include "globals.h"
#include "slice.buffer.h"
#include "globals.h"
#include <array>
#include <memory>

namespace blptls {
namespace spotykach {

class Generator: public IGenerator {
public:
    Generator(ISource&, IEnvelope&, ILFO&);
    
    void initialize() override;

    void set_pitch_shift(float) override;

    void set_frames_per_measure(uint32_t) override;
    void set_slice_position(float) override;
    void set_jitter_amount(float) override;
    void set_slice_length(float) override;
    void set_cycle_start() override;
    void set_reverse(bool) override;
    
    void set_on_update(std::function<void()> on_update);

    void activate_slice(float, int) override;
    void generate(float*, float*, bool, bool) override;
    void reset() override;

    void set_on_slice(SliceCallback) override;

    uint32_t frames_per_slice() override { 
        return _frames_per_slice; 
    }
    
    void set_needs_reset_slices() override;

private:
    ISource& _source;
    IEnvelope& _envelope;
    ILFO& _jitter_lfo;
    PitchShift _continual_pitch;
    std::array<std::shared_ptr<Slice>, kSlicesCount> _slices;
    std::array<SliceBuffer, kSlicesCount> _buffers;

    std::function<void()> _on_update;

    float _slice_position;
    float _jitter_amount;
    float _pitch_shift;

    size_t _slice_position_frames;
    size_t _frames_per_slice;
    size_t _frames_per_beat;
    
    float _raw_onset;
    bool _reverse;

    SliceCallback _on_slice;

    bool _continual;
    bool _continual_rev;
    uint32_t _continual_iterator;
};

}
}
