#include <cmath>
#include "KarplusString.h"
#include <stdlib.h>

using namespace infrasonic;

void KarplusString::Init(float sample_rate)
{
    sample_rate_ = sample_rate;

    SetFreq(440.f);
    brightness_           = .5f;
    damping_              = .5f;

    string_.Init();
    Reset();

    SetFreq(440.f);
    SetDamping(.8f);
    SetBrightness(.5f);

    crossfade_.Init();
}

void KarplusString::Reset()
{
    string_.Reset();
    iir_damping_filter_.Init(sample_rate_);
    iir_damping_filter_.SetFreq(8000.0f);

    dc_blocker_.Init(sample_rate_);

    out_sample_[0] = out_sample_[1] = 0.0f;
    src_phase_                      = 0.0f;
}

float KarplusString::Process(const float in)
{
    return ProcessInternal(in);
}

void KarplusString::SetFreq(float freq)
{
    freq /= sample_rate_;
    frequency_ = daisysp::fclamp(freq, 0.f, .25f);
}

void KarplusString::SetBrightness(float brightness)
{
    brightness_ = daisysp::fclamp(brightness, 0.f, 1.f);
}

void KarplusString::SetDamping(float damping)
{
    damping_ = daisysp::fclamp(damping, 0.f, 1.f);
}

float KarplusString::ProcessInternal(const float in)
{
    // float brightness = brightness_;

    float delay = 1.0f / frequency_;
    delay       = daisysp::fclamp(delay, 4.f, kDelayLineSize - 4.0f);

    // If there is not enough delay time in the delay line, we play at the
    // lowest possible note and we upsample on the fly with a shitty linear
    // interpolator. We don't care because it's a corner case (frequency_ < 11.7Hz)
    float src_ratio = delay * frequency_;
    if(src_ratio >= 0.9999f)
    {
        // When we are above 11.7 Hz, we make sure that the linear interpolator
        // does not get in the way.
        src_phase_ = 1.0f;
        src_ratio  = 1.0f;
    }

    // float damping_cutoff
        // = fmin(12.0f + damping_ * damping_ * 60.0f + brightness * 24.0f, 84.0f);
    // float damping_f
        // = fmin(frequency_ * powf(2.f, damping_cutoff * daisysp::kOneTwelfth), 0.499f);

    // Crossfade to infinite decay.
    // if(damping_ >= 0.95f)
    // {
    //     float to_infinite = 20.0f * (damping_ - 0.95f);
    //     brightness += to_infinite * (1.0f - brightness);
    //     damping_f += to_infinite * (0.4999f - damping_f);
    //     damping_cutoff += to_infinite * (128.0f - damping_cutoff);
    // }

    // float temp_f = damping_f * sample_rate_;
    // iir_damping_filter_.SetFreq(temp_f);

    // float ratio                = powf(2.f, damping_cutoff * daisysp::kOneTwelfth);
    // float damping_compensation = 1.f - 2.f * atanf(1.f / ratio) / (TWOPI_F);

    src_phase_ += src_ratio;
    if(src_phase_ > 1.0f)
    {
        float s = 0.0f;
        src_phase_ -= 1.0f;
        // delay   = delay * damping_compensation;
        s = string_.ReadHermite(delay);
        s += in;
        s = daisysp::fclamp(s, -20.f, +20.f);

        s = dc_blocker_.Process(s);
        s *= 0.8f;

        s = iir_damping_filter_.Process(s);
        string_.Write(s);

        out_sample_[1] = out_sample_[0];
        out_sample_[0] = s;
    }

    crossfade_.SetPos(src_phase_);
    return crossfade_.Process(out_sample_[1], out_sample_[0]);
}
