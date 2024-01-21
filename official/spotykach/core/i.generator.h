#pragma once

#include "globals.h"
#include "i.envelope.h"
#include <stdint.h>
#include <algorithm>

using SliceCallback = std::function<void(uint32_t, bool)>;

class IGenerator {
public:
    virtual void initialize() = 0;
    virtual void set_frames_per_measure(uint32_t value) = 0;
    virtual void set_pitch_shift(float value) = 0;
    virtual void set_slice_position(float) = 0;
    virtual void set_jitter_amount(float value) = 0;
    virtual void set_slice_length(float) = 0;
    virtual uint32_t frames_per_slice() = 0;
    virtual void set_reverse(bool value) = 0;
    virtual void activate_slice(float onset, int direction) = 0;
    virtual void generate(float* out0, float* out1, bool continual, bool reverse) = 0;
    virtual void set_needs_reset_slices() = 0;
    virtual void set_cycle_start() = 0;
    virtual void set_on_slice(SliceCallback f) = 0;
    virtual void reset() = 0;

    virtual ~IGenerator() {};
};
