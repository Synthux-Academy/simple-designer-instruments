#pragma once

#include "daisy_seed.h"
#include "muxknob.h"

struct Mux8 {
public: 
    Mux8() = default;
    ~Mux8() = default;
    
    using ADCChannel = int16_t;
    using Index = int16_t;
    void initialize(daisy::DaisySeed& hw, daisy::AdcChannelConfig& conf, Index i, ADCChannel in_adc_ch) {
        using namespace daisy;
        using namespace seed;
        _adc_channel = in_adc_ch;
        conf.InitMux(outPin(i), _knobs.size(), D12, D11, D10);
    }

    void initKnobs(daisy::DaisySeed& hw) {
        for (size_t i = 0; i < _knobs.size(); i++) {
            _knobs[i].initialize(_adc_channel, i, hw);
        }
    }

    std::tuple<MuxKnob::Target, float> paramAt(int index) {
        auto knob = _knobs[index];
        return { knob.target(), knob.value() };
    };

    size_t knobsCount() const { return _knobs.size(); }

private:
    int16_t _adc_channel;
    std::array<MuxKnob, kKnobsCount> _knobs;
    
    daisy::Pin outPin(int ch) const {
        return ch == 0 ? daisy::seed::A0 : daisy::seed::A2;
    }
};
