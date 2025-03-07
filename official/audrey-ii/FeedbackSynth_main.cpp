#include <daisy_seed.h>
#include "FeedbackSynthEngine.h"
#include "FeedbackSynthControls.h"

using namespace infrasonic;
using namespace daisy;
using namespace daisysp;

static const auto kSampleRate = SaiHandle::Config::SampleRate::SAI_48KHZ;
static const size_t kBlockSize = 4;

static DaisySeed hw;
static FeedbackSynth::Engine engine;
static FeedbackSynth::Controls controls;
static Limiter limiter[2];

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    controls.Update(hw);
    controls.Process();
    for (size_t i=0; i<size; i++) {
        engine.Process(IN_L[i], OUT_L[i], OUT_R[i]);
    }
    limiter[0].ProcessBlock(OUT_L, size, 0.7f);
    limiter[1].ProcessBlock(OUT_R, size, 0.7f);
}

int main(void)
{
    hw.Init();
    hw.SetAudioSampleRate(kSampleRate);
    hw.SetAudioBlockSize(kBlockSize);

    engine.Init(hw.AudioSampleRate());
    controls.Init(hw, engine);

    for(auto& lim : limiter)
    {
        lim.Init();
    }

    hw.StartAudio(AudioCallback);

    while(1) {}
}
