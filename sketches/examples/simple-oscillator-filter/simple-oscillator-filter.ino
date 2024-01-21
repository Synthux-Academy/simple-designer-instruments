////////////////////////////////////////////////////////////////
///////////////////// VARIABLES & LIBRARIES ////////////////////

#include "DaisyDuino.h"

static Oscillator oscillator01;
static MoogLadder filter;

////////////////////////////////////////////////////////////////
///////////////////// START SYNTH SETUP ////////////////////////


void setup() {
  // DAISY SETUP
  DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  float sample_rate = DAISY.get_samplerate();

  // OSCILLATOR SETUP
  oscillator01.Init(sample_rate);
  oscillator01.SetFreq(200);
  oscillator01.SetWaveform(oscillator01.WAVE_SAW);

  // FILTER SETUP
  filter.Init(sample_rate);

  // DAISY SETUP
  DAISY.begin(ProcessAudio);
}


///////////////////// END SYNTH SETUP //////////////////////////
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
///////////////////// PROCESSING AUDIO SAMPLES (LOOP) //////////


void ProcessAudio(float **in, float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    float sample = filter.Process(oscillator01.Process());
    out[0][i] = sample;
    out[1][i] = sample;
  }
}


////////////////////////////////////////////////////////////////
///////////////////// START CONTROLS LOOP //////////////////////


void loop() {

  // FILTER CONTROL
  filter.SetFreq(1023 - analogRead(A0));

}
