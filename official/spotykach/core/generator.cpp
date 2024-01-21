#include "generator.h"
#include "globals.h"
#include "buffers.h"
#include "globals.h"
#include "../common/fcomp.h"

using namespace blptls;
using namespace spotykach;

struct Modulations {
    float position;
    float pitch;
};

Modulations modulations(float x) {
    if (fcomp(x, 0, 3)) return { 0, 0 };
    if (fcomp(x, 1, 3)) return { 0, 0 };

    if (x < 0.33)  return { 2.27f * x, 0 };
    if (x < 0.66)  return { 1.5f - 2.27f * x, 2.f * (x - 0.33f) };
    if (x < 1.0)   return { 0, 1.32f - 2.f * (x - 0.33f) };
};

Generator::Generator(ISource& in_source, IEnvelope& in_envelope, ILFO& in_jitter_lfo) :
    _source             { in_source },
    _envelope           { in_envelope },
    _jitter_lfo         { in_jitter_lfo },
    _raw_onset          { 0 },
    _reverse            { false },
    _slice_position     { -1 },
    _continual          { false },
    _continual_rev      { false },
    _continual_iterator { 0 } {
    for (auto i = 0; i < kSlicesCount; i++) {
        _slices[i] = std::make_shared<Slice>(_source, _buffers[i] ,_envelope);
    }
    reset();
}

void Generator::set_pitch_shift(float value) {
    _pitch_shift = value;
    _continual_pitch.setShift(value);
}

void Generator::set_slice_position(float value) {
    auto init_sycle_start = _slice_position < 0;
    _slice_position = value;
    _slice_position_frames = _source.length() * _slice_position;
    if (init_sycle_start) set_cycle_start();
}

void Generator::set_jitter_amount(float value) {
    _jitter_amount = value;
}

void Generator::set_slice_length(float value) {
    _frames_per_slice = value * kSliceBufferLength;
}

void Generator::set_reverse(bool value) {
    if (value != _reverse) {
        set_needs_reset_slices();
    } 
    _reverse = value;
}

void Generator::set_cycle_start() {
    _source.set_cycle_start(_slice_position_frames);
}

void Generator::initialize() {
    for (auto s: _slices) s->initialize();
    _continual_pitch.initialize(kSampleRate, 4096);
}

void Generator::set_frames_per_measure(uint32_t value) {
    _frames_per_beat = value / kBeatsPerMeasure;
}

void Generator::generate(float* out0, float* out1, bool continual, bool reverse) {
    float out_0_val = 0;
    float out_1_val = 0;
    float slice_out_0 = 0;
    float slice_out_1 = 0;
    
    for (auto& s: _slices) {
        if (s->isInactive()) continue;

        s->synthesize(&slice_out_0, &slice_out_1);
        out_0_val += slice_out_0;
        out_1_val += slice_out_1;
    }

    if (continual) {
        if (!_continual) {
            _continual_iterator = 0;
            _continual = true;
        }
        auto frame = _source.is_frozen() ? _slice_position_frames + _continual_iterator : _source.read_head();
        _source.read(slice_out_0, slice_out_1, frame);
        _continual_pitch.process(&slice_out_0, &slice_out_1);
        out_0_val += slice_out_0;
        out_1_val += slice_out_1;
        if (reverse) {
            if (_continual_iterator == 0) {
                _continual_iterator = _source.length();
            }
            else {
                _continual_iterator --;
            }
        } 
        else {
            _continual_iterator ++;
        }
    }

    *out0 = out_0_val;
    *out1 = out_1_val;
}

void Generator::set_on_slice(SliceCallback f) {
    _on_slice = f;
}

void Generator::activate_slice(float in_raw_onset, int direction) {
    auto reset = !fcomp(in_raw_onset, _raw_onset) || !_source.is_frozen();
    auto offset = _slice_position_frames;
    auto lfo_value = _jitter_lfo.triangleValue();
    auto m = modulations(_jitter_amount);
    if (m.position > 0.05) {
        auto length = _source.length() - 1;
        offset += lfo_value * m.position * length;
        offset = std::min(std::max(offset, 0u), length);
        reset = true;
    }
    auto pitch_shift = _pitch_shift;
    auto frames_per_slice = _frames_per_slice;
    auto volume = 1.f;
    auto reverse = _reverse;
    if (m.pitch > 0.05) {
        auto shift = lfo_value * m.pitch;
        pitch_shift += 0.25 * shift;
        pitch_shift = std::min(std::max(pitch_shift, 0.0f), 1.0f);

        volume += 2.f * shift;
        volume = std::min(std::max(volume, 0.f), 1.0f);

        reverse = shift < 0;
    }
    
    if (direction != 0) reverse = (direction == -1);

    if (reset) {
        set_needs_reset_slices();
        _raw_onset = in_raw_onset;
    }
    
    auto onset = _frames_per_beat * _raw_onset;
    auto slice_start = onset + offset;
    
    for (auto& s: _slices) {
        if (s->isActive()) continue;
        s->activate(slice_start, frames_per_slice, reverse, pitch_shift, volume);
        if (_on_slice) _on_slice(frames_per_slice, reverse);
        break;
    }
}

void Generator::reset() {
    set_needs_reset_slices();
}

void Generator::set_needs_reset_slices() {
    for (auto s: _slices) s->setNeedsReset();
    if (_on_update) _on_update();
}

void Generator::set_on_update(std::function<void()> on_update) {
    _on_update = on_update;
}