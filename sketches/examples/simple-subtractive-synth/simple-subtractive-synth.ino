/**
 * Title: simple subtractive synth
 * Description: oscillators! filters! lfos! reverb!
 * Hardware: Daisy Seed
 * Author: Chris Maniewski

 * Analog port mapping:
 * A0 filter freq
 * A1 resonance
 * A2 lfo freq
 * A3 osc freq
 * A4 lfo amt -> filter
 * A5 room size
 * A6 wet + dry
 * A7 master volume
 */

// SIMPLE X DAISY PINOUT CHEATSHEET
//                    _____
//                  ---------
// 3v3          29  |       |   20    AGND
// D15 / A0     30  |       |   19    OUT 01
// D16 / A1     31  |       |   18    OUT 00
// D17 / A2     32  |       |   17    IN 01
// D18 / A3     33  |       |   16    IN 00
// D19 / A4     34  |       |   15    D14
// D20 / A5     35  |       |   14    D13
// D21 / A6     36  |       |   13    D12
// D22 / A7     37  |       |   12    D11
// D23 / A8     38  |       |   11    D10
// D24 / A9     39  |       |   10    D9
// D25 / A10    40  |       |   09    D8
// D26          41  |       |   08    D7
// D27          42  |       |   07    D6
// D28 / A11    43  |       |   06    D5
// D29          44  |       |   05    D4
// D30          45  |       |   04    D3
// 3v3 Digital  46  |       |   03    D2
// VIN          47  |       |   02    D1
// DGND         48  |       |   01    D0
//                  ---------

#include "DaisyDuino.h"

DaisyHardware hw;

float filter_freq;
float filter_res;
float verb_fback;
float wet_amt;
float lfo_freq;
float lfo_amt;
float osc_freq;
float master_vol;

static MoogLadder flt;
static Oscillator osc, lfo;
static ReverbSc verb;

void AudioCallback(float **in, float **out, size_t size) {
  float osc_sample, output, lfo_sample;

  for (size_t i = 0; i < size; i++) {
    float rev_tail0, rev_tail1, flt_freq;

    // lfo_sample is between -0.5 and 0.5, so we add 0.5 to bring it between 0 and 1 first
    lfo_sample = fmap(lfo.Process() + 0.5, 0, 5000);

    // Here we mix two signals: the filter_freq that was set by the knob and the lfo_sample that we received earlier.
    // The mix ratios are determined by the lfo_amt
    flt_freq = filter_freq * (1 - lfo_amt) + lfo_sample * lfo_amt;
    flt.SetFreq(flt_freq);

    osc_sample = osc.Process();

    output = flt.Process(osc_sample);

    verb.Process(output, output, &rev_tail0, &rev_tail1);

    // Here we mix two signals again: the output that comes from the filter and the reverb tails.
    // The mix ratios are determined by the wet_amt.
    // After that everything is attenuated by multiplying it with the master volume
    out[0][i] = ((1 - wet_amt) * output + wet_amt * rev_tail0) * master_vol;
    out[1][i] = ((1 - wet_amt) * output + wet_amt * rev_tail1) * master_vol;
  }
}

void setup() {
  float sample_rate;
  // Initialize Daisy at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  sample_rate = DAISY.get_samplerate();

  // Initialize the Moogladder filter
  flt.Init(sample_rate);
  flt.SetRes(0.7);

  // Set parameters for the LFO
  lfo.Init(sample_rate);
  lfo.SetWaveform(Oscillator::WAVE_TRI);
  lfo.SetAmp(0.5);
  lfo.SetFreq(.4);

  // Set parameters for the oscillator
  osc.Init(sample_rate);
  osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
  osc.SetFreq(110);
  osc.SetAmp(0.25);

  // Set parameters for the reverb
  verb.Init(sample_rate);
  verb.SetFeedback(0.2);
  verb.SetLpFreq(16000);

  DAISY.begin(AudioCallback);
}

void loop() {
  // Map the LFO frequency knob to values between 0 and 64 Hertz
  lfo_freq = fmap(simpleAnalogRead(A2), 0, 64);
  lfo.SetFreq(lfo_freq);

  // We are using the 4th fmap parameter here, which determines the mapping curve
  // Mapping::EXP means that the higher the frequency rises, the faster it rises
  // This is analogous to how we percieve sound, so the knob will feel more natural
  filter_freq = fmap(simpleAnalogRead(A0), 0, 5000, Mapping::EXP);

  // Tame the filter a bit by dividing the resonance by more than 1
  filter_res = simpleAnalogRead(A1) / 1.25;
  flt.SetRes(filter_res);

  // Here we eliminate the dead-zone of the reverb knob.
  // By adding 0.4 the reverb feedback will always be at least 0.4.
  // Then we multiply the read value by 0.6 so the resulting feedback value will never by greater than 1
  // So we skip the boring part between 0 and 0.4 where nothing really happens when turning the knob
  verb_fback = 0.4 + simpleAnalogRead(A5) * 0.6;
  verb.SetFeedback(verb_fback);

  // Set the wet amount for the effect. This is between 0 and 1 (100%)
  wet_amt = simpleAnalogRead(A6);

  // Map the oscillator frequency knob to values between 220 and 440 Hertz (one octave)
  osc_freq = fmap(simpleAnalogRead(A3), 220, 440);
  // Uncomment this if you want your oscillator frequency to jump in semitone steps instead
  // osc_freq = semitone_to_hertz(simpleAnalogReadAndMap(A3, 0, 12));
  osc.SetFreq(osc_freq);

  // Set the amount for the LFO VCA. This is between 0 and 1 (100%)
  lfo_amt = simpleAnalogRead(A4);

  // Set the master volume. This is between 0 and 1 (100%)
  master_vol = simpleAnalogRead(A7);
}

/**
 * Reads a Simple board pot (inverted) and normalizes it to values between 0 and 1
 *
 * This is a helper function to make our lives easier, as we are using this pattern a lot
 */
float simpleAnalogRead(uint32_t pin) {
  return (1023.0 - (float)analogRead(pin)) / 1023.0;
}

/**
 * Reads a simple pot and maps it to a value bewtween to integer values
 *
 * This is a helper function to make our lives easier, as we are using this pattern a lot
 */
float simpleAnalogReadAndMap(uint32_t pin, long min, long max) {
  return map(1023 - analogRead(pin), 0, 1023, min, max);
}

/**
 * Convert a note number to Hertz
 *
 * Uses the base frequency of 220Hz, i.e. a note number of 0 equals 220Hz
 */
float semitone_to_hertz(uint8_t note_number) {
  return 220 * pow(2, ((float)note_number - 0) / 12);
}
