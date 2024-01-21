#pragma once

#include "daisy_seed.h"
#include "smoother.h"

const int kKnobsCount { 8 };

struct MuxKnob {
public:
    enum class Target {
        Position,
        Slice,
        Retrigger,
        Jitter,
        Step,
        Level,
        Repeats,
        Shift
    };

    MuxKnob() = default;
    ~MuxKnob() = default;

    using MuxChannel = uint8_t;
    using ADCCHannel = int;
    void initialize(ADCCHannel adc_ch, MuxChannel mux_ch, daisy::DaisySeed& hw) {
        const auto flip = true;
        const auto sample_rate = hw.AudioCallbackRate();
        _target = assignedTarget(mux_ch);
        _control.Init(hw.adc.GetMuxPtr(adc_ch, mux_ch), sample_rate, flip);
    }

    float smoothing() const { 
        return _smoother.smoothing();
    }

    void setSmoothing(float s) {
        _smoother.setSmoothing(s);
    }

    Target target() {
        return _target;
    }

    float value() {
        _control.Process();
        return _smoother.smoothed(_control.Value());
    }

private: 
    Smoother _smoother;
    daisy::AnalogControl _control;
    Target _target;

    Target assignedTarget(MuxChannel mux_ch) {
        static std::array<Target, kKnobsCount> targets = {
            Target::Position,
            Target::Slice,
            Target::Retrigger,
            Target::Jitter,
            Target::Step,
            Target::Level,
            Target::Shift,
            Target::Repeats
        };
        return targets[mux_ch];
    }
};
