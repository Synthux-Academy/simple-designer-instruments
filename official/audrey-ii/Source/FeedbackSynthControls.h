#pragma once
#ifndef INFS_FEEDBACKSYNTHCONTROLS_H
#define INFS_FEEDBACKSYNTHCONTROLS_H

#include <daisy.h>
#include <daisy_seed.h>
#include "FeedbackSynthEngine.h"
#include "ParameterRegistry.h"

namespace infrasonic {
namespace FeedbackSynth {

class Controls {

public:

    Controls() = default;
    ~Controls() = default;

    void Init(daisy::DaisySeed &hw, Engine &engine);

    void Update(daisy::DaisySeed &hw);

    void Process() {
        params_.Process();
    }

private:

    static const size_t kNumAdcChannels = 11;

    /// Identifies a parameter of the synth engine
    /// The order here is the same order as the ADC pin configs in the cpp file
    enum class Parameter : uint8_t {
        Frequency           = 0,
        FeedbackGain,       // 1
        FeedbackBody,       // 2
        FeedbackLPFCutoff,  // 3
        FeedbackHPFCutoff,  // 4
        ReverbMix,          // 5
        ReverbDecay,        // 6
        EchoDelaySend,      // 7
        EchoDelayTime,      // 8
        EchoDelayFeedback,  // 9
        OutputVolume        // 10
    };

    using Parameters = ParameterRegistry<Parameter>;

    Parameters params_;
    daisy::Switch del_sw_;

    void initADCs(daisy::DaisySeed &hw);
    void registerParams(Engine &engine);
};

}
}

#endif
