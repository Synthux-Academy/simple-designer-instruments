#include "knob.h"

using namespace daisy;

void Knob::initialize(AdcChannelConfig& conf, int channel) {
    _channel = channel;
    auto target = _targets[channel];
    conf.InitSingle(pin(target));
}

void Knob::configure(DaisySeed& hw) {
    _ctrl.Init(hw.adc.GetPtr(_channel), hw.AudioCallbackRate(), true);
}

float Knob::value() { 
    _ctrl.Process();
    return _smoother.smoothed(_ctrl.Value()); 
}
