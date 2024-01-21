#include "FeedbackSynthControls.h"
#include <functional>

using namespace infrasonic;
using namespace infrasonic::FeedbackSynth;
using namespace daisy;

////////////// SIMPLE X DAISY PINOUT CHEATSHEET ///////////////

// 3v3           29  |       |   20    AGND
// D15 / A0      30  |       |   19    OUT 01
// D16 / A1      31  |       |   18    OUT 00
// D17 / A2      32  |       |   17    IN 01
// D18 / A3      33  |       |   16    IN 00
// D19 / A4      34  |       |   15    D14
// D20 / A5      35  |       |   14    D13
// D21 / A6      36  |       |   13    D12
// D22 / A7      37  |       |   12    D11
// D23 / A8      38  |       |   11    D10
// D24 / A9      39  |       |   10    D9
// D25 / A10     40  |       |   09    D8
// D26           41  |       |   08    D7
// D27           42  |       |   07    D6
// D28 / A11     43  |       |   06    D5
// D29           44  |       |   05    D4
// D30           45  |       |   04    D3
// 3v3 Digital   46  |       |   03    D2
// VIN           47  |       |   02    D1
// DGND          48  |       |   01    D0

// TODO: Add footprint numbers to these

static constexpr daisy::Pin kFreqKnobAdcPin             = daisy::seed::A10; // Simple bottom pin 40
static constexpr daisy::Pin kFeedbackGainKnobPin        = daisy::seed::A9;  // Simple bottom pin 39
static constexpr daisy::Pin kFeedbackBodyKnobPin        = daisy::seed::A5;  // Simple bottom pin 35
static constexpr daisy::Pin kFeedbackLowpassKnobAdcPin  = daisy::seed::A8;  // Simple bottom pin 38
static constexpr daisy::Pin kFeedbackHighpassKnobAdcPin = daisy::seed::A4;  // Simple bottom pin 34
static constexpr daisy::Pin kRevMixKnobAdcPin           = daisy::seed::A7;  // Simple bottom pin 37
static constexpr daisy::Pin kRevDecayKnobAdcPin         = daisy::seed::A6;  // Simple bottom pin 36
static constexpr daisy::Pin kEchoSendKnobAdcPin         = daisy::seed::A1;  // Simple bottom pin 31
static constexpr daisy::Pin kEchoTimeKnobAdcPin         = daisy::seed::A0;  // Simple bottom pin 30
static constexpr daisy::Pin kEchoFeedbackKnobAdcPin     = daisy::seed::A3;  // Simple bottom pin 33
static constexpr daisy::Pin kOutputVolumeAdcPin         = daisy::seed::A2;  // Simple bottom pin 32
static constexpr daisy::Pin kDelaySwitchPin             = daisy::seed::D14; // Simple bottom pin 15

void Controls::Init(DaisySeed &hw, Engine &engine) {
    params_.Init(hw.AudioSampleRate() / hw.AudioBlockSize());
    del_sw_.Init(
        static_cast<dsy_gpio_pin>(kDelaySwitchPin),
        1000.0f,
        Switch::TYPE_TOGGLE,
        Switch::POLARITY_INVERTED,
        Switch::PULL_UP
    );
    initADCs(hw);
    registerParams(engine);
}

void Controls::Update(DaisySeed &hw) {
    params_.UpdateNormalized(Parameter::Frequency,          1.0f - hw.adc.GetFloat(0));
    params_.UpdateNormalized(Parameter::FeedbackGain,       1.0f - hw.adc.GetFloat(1));
    params_.UpdateNormalized(Parameter::FeedbackBody,       1.0f - hw.adc.GetFloat(2));
    params_.UpdateNormalized(Parameter::FeedbackLPFCutoff,  1.0f - hw.adc.GetFloat(3));
    params_.UpdateNormalized(Parameter::FeedbackHPFCutoff,  1.0f - hw.adc.GetFloat(4));
    params_.UpdateNormalized(Parameter::ReverbMix,          1.0f - hw.adc.GetFloat(5));
    // Special mapping for reverb feedback/decay (anti-exponential tension curve)
    params_.UpdateNormalized(Parameter::ReverbDecay,        ftension(1.0f - hw.adc.GetFloat(6), -3.0f));
    params_.UpdateNormalized(Parameter::EchoDelaySend,      1.0f - hw.adc.GetFloat(7));
    // Delay switch doubles or halves delay time instantly for doppler warp
    del_sw_.Debounce();
    float delay_norm = 1.0f - hw.adc.GetFloat(8);
    float delay_scale = del_sw_.Pressed() ? 0.5f : 1.0f;
    params_.UpdateNormalized(Parameter::EchoDelayTime, delay_norm * delay_scale);
    params_.UpdateNormalized(Parameter::EchoDelayFeedback,  1.0f - hw.adc.GetFloat(9));
    params_.UpdateNormalized(Parameter::OutputVolume,       1.0f - hw.adc.GetFloat(10));
}

void Controls::initADCs(DaisySeed &hw) {
    AdcChannelConfig config[kNumAdcChannels];

    config[0].InitSingle(kFreqKnobAdcPin);
    config[1].InitSingle(kFeedbackGainKnobPin);
    config[2].InitSingle(kFeedbackBodyKnobPin);
    config[3].InitSingle(kFeedbackLowpassKnobAdcPin);
    config[4].InitSingle(kFeedbackHighpassKnobAdcPin);
    config[5].InitSingle(kRevMixKnobAdcPin);
    config[6].InitSingle(kRevDecayKnobAdcPin);
    config[7].InitSingle(kEchoSendKnobAdcPin);
    config[8].InitSingle(kEchoTimeKnobAdcPin);
    config[9].InitSingle(kEchoFeedbackKnobAdcPin);
    config[10].InitSingle(kOutputVolumeAdcPin);

    hw.adc.Init(config, kNumAdcChannels);
    hw.adc.Start();
}

void Controls::registerParams(Engine &engine) {
    using namespace std::placeholders;

    // String freq/pitch as note number
    params_.Register(Parameter::Frequency, 40.0f, 16.0f, 72.0f,
        std::bind(&Engine::SetStringPitch, &engine, _1), 0.2f);

    // Feedback Gain in dbFS
    params_.Register(Parameter::FeedbackGain, -60.0f, -60.0f, 12.0f,
        std::bind(&Engine::SetFeedbackGain, &engine, _1));

    // Feedback body/delay in seconds
    params_.Register(Parameter::FeedbackBody, 0.001f, 0.001f, 0.1f,
        std::bind(&Engine::SetFeedbackDelay, &engine, _1), 1.0f, daisysp::Mapping::EXP);

    // Feedback filter cutoffs in hz
    params_.Register(Parameter::FeedbackLPFCutoff, 18000.0f, 100.0f, 18000.0f,
        std::bind(&Engine::SetFeedbackLPFCutoff, &engine, _1), 0.05f, daisysp::Mapping::LOG);
    params_.Register(Parameter::FeedbackHPFCutoff, 250.0f, 10.0f, 4000.0f,
        std::bind(&Engine::SetFeedbackHPFCutoff, &engine, _1), 0.05f, daisysp::Mapping::LOG);

    // Reverb Mix
    params_.Register(Parameter::ReverbMix, 0.0f, 0.0f, 1.0f,
        std::bind(&Engine::SetReverbMix, &engine, _1));

    // Reverb Feedback (input is mapped to anti-exponential on ADC read)
    params_.Register(Parameter::ReverbDecay, 0.2f, 0.2f, 1.0f,
        std::bind(&Engine::SetReverbFeedback, &engine, _1));

    // Echo Delay send
    params_.Register(Parameter::EchoDelaySend, 0.0f, 0.0f, 1.0f,
        std::bind(&Engine::SetEchoDelaySendAmount, &engine, _1), 0.05f, daisysp::Mapping::EXP);

    // Echo Delay time in s
    params_.Register(Parameter::EchoDelayTime, 0.5f, 0.05f, 5.0f,
        std::bind(&Engine::SetEchoDelayTime, &engine, _1), 0.1f, daisysp::Mapping::EXP);

    // Echo Delay feedback
    params_.Register(Parameter::EchoDelayFeedback, 0.0f, 0.0f, 1.5f,
        std::bind(&Engine::SetEchoDelayFeedback, &engine, _1));

    // Output level
    params_.Register(Parameter::OutputVolume, 0.5f, 0.0f, 1.0f,
        std::bind(&Engine::SetOutputLevel, &engine, _1), 0.05f, daisysp::Mapping::EXP);
}
