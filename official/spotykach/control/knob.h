#pragma once

#include "daisy_seed.h"
#include "smoother.h"

class Knob {
public:
    enum class Target {
        SlicePositionA,
        SliceLengthA,
        RetriggerA,
        JitterAmountA,
        Tempo,
        VolumeCrossfade,
        PatternCrossfade,
        Pitch,
        SlicePositionB,
        SliceLengthB,
        RetriggerB,
        JitterAmountB
    };

    Knob() = default;
    ~Knob() = default;

    void initialize(daisy::AdcChannelConfig& conf, int channel);
    void configure(daisy::DaisySeed& hw);
    Knob::Target target() const { return _targets[_channel]; };
    float value();

private:
    daisy::AnalogControl _ctrl;
    Smoother _smoother;
    int _channel;
    constexpr static  std::array<Target, 12> _targets = { 
        Knob::Target::SlicePositionA,
        Knob::Target::SliceLengthA,
        Knob::Target::RetriggerA,
        Knob::Target::JitterAmountA,
        Knob::Target::Tempo,
        Knob::Target::VolumeCrossfade,
        Knob::Target::PatternCrossfade,
        Knob::Target::Pitch,
        Knob::Target::SlicePositionB,
        Knob::Target::SliceLengthB,
        Knob::Target::RetriggerB,
        Knob::Target::JitterAmountB
    };
    
    daisy::Pin pin(Knob::Target usage) const {
        using namespace daisy;
        using namespace seed;
        using KT = Knob::Target;
        switch (usage) {
            case KT::JitterAmountA:     return A10;
            case KT::SlicePositionA:    return A9;
            case KT::SliceLengthA:      return A8;
            case KT::RetriggerA:        return A7;
            case KT::Tempo:             return A6;
            case KT::VolumeCrossfade:   return A11;
            case KT::PatternCrossfade:  return A5;
            case KT::Pitch:             return A4;
            case KT::RetriggerB:        return A3;
            case KT::JitterAmountB:     return A2;
            case KT::SlicePositionB:    return A1;
            case KT::SliceLengthB:      return A0;
            default: return {};
        }
    };
};
