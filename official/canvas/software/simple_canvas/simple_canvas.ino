#include "DaisyDuino.h"
using namespace daisysp;

#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_MCP23X17.h>

#include <Adafruit_NeoPixel.h>

#include "MorphingMultiShapeAdsr.h"
#include "AveragedAnalog.h"

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif


#define TRIGGER_DIFF 0.01
#define DEFAULT_VALUE 5          //put a default value that is not possible to be sure all pins are initialized correctly
#define DEFAULT_SIGNED_VALUE -1  //put a default value that is not possible to be sure all pins are initialized correctly

#define AN_SEQUENCER_STEPANALOGIN A0
#define AN_CLOCK_RATE A1
#define AN_PULSER_PERIOD A2
#define AN_MODOSC_FREQUENCY A3
#define AN_MODOSC_WAVEFORM A4
#define AN_MODOSC_ATTENUATOR A5
#define AN_COMPLEXOSC_TIMBRE A6
#define AN_COMPLEXOSC_FREQUENCY A7
#define AN_COMPLEXOSC_ATTENUATOR A8
#define AN_ENVELOPEGEN_SIG0DECAY A9
#define AN_ENVELOPEGEN_SIG1DECAY A10
#define AN_ENVELOPEGEN_SLOPESHAPE A11

AveragedAnalog avAnClockRate((uint32_t)4);

AveragedAnalog avAnPulserPeriod((uint32_t)4);
AveragedAnalog avAnModoscFrequency(true);
AveragedAnalog avAnModoscWaveform;
AveragedAnalog avAnModoscAttenuator;
AveragedAnalog avAnComplexoscTimbre;
AveragedAnalog avAnComplexoscFrequency(true);
AveragedAnalog avAnComplexoscAttenuator;
AveragedAnalog avAnEnvelopegenSig0decay;
AveragedAnalog avAnEnvelopegenSig1decay;
AveragedAnalog avAnEnvelopeShape;

//Digital input gpio definition
#define DI_MIDIIN 1
#define DI_MIDIOUT 2
#define DI_SEQUENCER_STEPSELECT0 4
#define DI_SEQUENCER_STEPSELECT1 3
#define DI_SEQUENCER_STEPSELECT2 29
#define DI_SEQUENCER_STEPSELECT3 30
#define DI_SEQUENCER_STEPSELECT4 0
#define DI_SEQUENCER_STEP4 5
#define DI_SEQUENCER_STEP3 6
#define DI_SEQUENCER_STEP2 7
#define DI_SEQUENCER_STEP1 8
#define DI_SEQUENCER_STEP0 9
#define DI_PATCH_IRQ 10
#define DI_SCL 11
#define DI_SDA 12
#define DI_INTB 13
#define DI_INTA 14
#define DI_LEDS_DIN 26

//Digital input gpio expander gpio definition
#define DI_SEQUENCER_TRIGGER 0
#define DI_SEQUENCER_STAGE0 1
#define DI_SEQUENCER_STAGE1 2
#define DI_RANDOM_TRIGGERSELECT0 3
#define DI_RANDOM_TRIGGERSELECT1 4
#define DI_PULSER_TRIGGERSELECT0 5
#define DI_PULSER_TRIGGERSELECT1 6
#define DI_MODOSC_AMFM 7
#define DI_COMPLEXOSC_WAVEFORM0 8
#define DI_COMPLEXOSC_WAVEFORM1 9
#define DI_ENVELOPEGEN_SIG0SELECTOR0 10
#define DI_ENVELOPEGEN_SIG0SELECTOR1 11
#define DI_ENVELOPEGEN_SIG1SELECTOR0 12
#define DI_ENVELOPEGEN_SIG1SELECTOR1 13
#define DI_ENVELOPEGEN_SIG0LPGVCA 14
#define DI_ENVELOPEGEN_SIG1LPGVCA 15

uint8_t DI_SEQUENCER_STEPSELECT[5] = { DI_SEQUENCER_STEPSELECT0, DI_SEQUENCER_STEPSELECT1, DI_SEQUENCER_STEPSELECT2, DI_SEQUENCER_STEPSELECT3, DI_SEQUENCER_STEPSELECT4 };

#define SEQUENCER_AN_MIN 27
#define SEQUENCER_AN_MAX 920
#define SEQUENCER_AN_RANGE (SEQUENCER_AN_MAX - SEQUENCER_AN_MIN)

//analog pins value
float sequencerStepAnalogIn[5];
float clockRate;
float envelopeGenSig1Decay;
float pulserPeriod = 3.0f;
float modOscFrequency = 440.0f;  //just in case we set frequency with it before it succeed to read the analog value, should never happen
float modOscWaveform;
float modOscAttenuator;
float complexOscTimbre;
float complexOscFrequency;
float complexOscAttenuator;
float envelopeGenSig0Decay;

float modOscFrequencyOld = DEFAULT_VALUE;
float complexOscFrequencyOld = DEFAULT_VALUE;
float modOscWaveformOld = DEFAULT_VALUE;
float pulserPeriodOld = DEFAULT_VALUE;
float clockRateOld = DEFAULT_VALUE;
float envelopeGenSig0DecayOld = DEFAULT_VALUE;
float envelopeGenSig1DecayOld = DEFAULT_VALUE;

float envelopeGenSig0DecayFactor = DEFAULT_VALUE;
float envelopeGenSig1DecayFactor = DEFAULT_VALUE;

float envelopeGenSig0Volume = DEFAULT_VALUE;
float envelopeGenSig1Volume = DEFAULT_VALUE;

bool envelope0Enable = true;
bool envelope1Enable = true;

float slopeFactor = 0.0f;
float slopeMorphFactor = 0.0f;

// ditial pins value
uint8_t sequencerSteps[5] = { 0, 0, 0, 0, 0 };

uint8_t sequencerStepsOld[5] = { DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE };

uint8_t sequencerLenght = 5;
uint8_t sequencerIndex = 0;
float sequencerValue = 0;
bool currentSequencerStepEnable = true;

// mcp pins value
uint8_t sequencerTrigger = 0;
uint8_t sequencerStage = 0;
uint8_t randomTriggerSelect = 0;
uint8_t pulserTriggerSelect = 0;
uint8_t modOscAmFm = 0;
uint8_t complexOscWaveform = 0;
uint8_t envelopeGenSig0Selector = 0;
uint8_t envelopeGenSig0LpgVca = 0;
uint8_t envelopeGenSig1Selector = 0;
uint8_t envelopeGenSig1LpgVca = 0;

uint8_t sequencerTriggerOld = DEFAULT_VALUE;
uint8_t sequencerStageOld = DEFAULT_VALUE;
uint8_t randomTriggerSelectOld = DEFAULT_VALUE;
uint8_t pulserTriggerSelectOld = DEFAULT_VALUE;
uint8_t modOscAmFmOld = DEFAULT_VALUE;
uint8_t complexOscWaveformOld = DEFAULT_VALUE;
uint8_t envelopeGenSig0SelectorOld = DEFAULT_VALUE;
uint8_t envelopeGenSig0LpgVcaOld = DEFAULT_VALUE;
uint8_t envelopeGenSig1SelectorOld = DEFAULT_VALUE;
uint8_t envelopeGenSig1LpgVcaOld = DEFAULT_VALUE;

// --------------------- Gpio expander --------------------------
Adafruit_MCP23X17 mcp;

// ----------------- Seed modules --------------------------------

static Oscillator complexOscSinus;
static VariableShapeOscillator complexOscBasis;

static Chorus chorus;

static VariableShapeOscillator pulserOsc;

static VariableShapeOscillator modulationOsc;

static MoogLadder lowPassGateFilter0;  // yes moogladder in a buchla inspired synth, what you gonna do
static MoogLadder lowPassGateFilter1;

float frequency = 440;
float sample_rate;

// ---------------------- Custom modules ------------------------

MorphingMultiShapeAdsr multiShapeAdsr0;
MorphingMultiShapeAdsr multiShapeAdsr1;

// ----------------- Capacitive sensor ---------------------------
#define THRESHOLD_TOUCHED 95
#define INPUT_TOUCH_COUNT 4
#define OUTPUT_TOUCH_COUNT 5
#define TOTAL_TOUCH_COUNT (INPUT_TOUCH_COUNT + OUTPUT_TOUCH_COUNT)

enum TRIGGER_SOURCE { PULSER_TRIGGER = 0,
                      CLOCK_TRIGGER = 1,
                      SEQ_PULSER_TRIGGER = 2,
                      SEQ_CLOCK_TRIGGER = 3 };

uint8_t sequencerTriggerSource = PULSER_TRIGGER;
uint8_t randomVoltageTriggerSource = PULSER_TRIGGER;
uint8_t pulserTriggerSource = PULSER_TRIGGER;
uint8_t envelopeGenSig0TriggerSource = PULSER_TRIGGER;
uint8_t envelopeGenSig1TriggerSource = PULSER_TRIGGER;


enum SOURCE_MODULE { NONE_SOURCE = -1,
                     SEQUENCER = 0,
                     PULSER = 1,
                     RANDOM = 2,
                     ENVELOPE_B = 3 };

enum DESTINATION_MODULE { NONE_DEST = -1,
                          OSC_A_FRQ = 0,
                          OSC_A_TMBR = 1,
                          OSC_B_FRQ = 2,
                          OSC_B_FORM = 3,
                          OSC_B_ATT = 4 };

uint16_t capacitiveSensorRawValue[TOTAL_TOUCH_COUNT];
uint8_t capacitiveSensorTouched[TOTAL_TOUCH_COUNT];
uint8_t capacitiveSensorTouchedOld[TOTAL_TOUCH_COUNT];

int8_t destinationPatches[OUTPUT_TOUCH_COUNT] = {
  NONE_DEST,
  NONE_DEST,
  NONE_DEST,
  NONE_DEST,
  NONE_DEST
};

int8_t highlightedSource = NONE_SOURCE;

Adafruit_MPR121 cap = Adafruit_MPR121();

enum {
  IDLE,
  WAIT,
  INPUT_PRESSED,
  WAIT_OUTPUT,
  OUTPUT_PRESSED,
  INPUT_CANCELLED

} capacitiveState = IDLE;

int lastCapacitiveState = IDLE;

int8_t inputPressedIndex;
int8_t outputPressedIndex;

// ----------------- Neopixels -----------------------------------

#define LED_OFF_COUNT 4
//TOTAL_TOUCH_COUNT = 9
#define ENVELOPE_VOLTAGE_LED TOTAL_TOUCH_COUNT
#define PULSER_VOLTAGE_LED ENVELOPE_VOLTAGE_LED + 1
#define RANDOM_VOLTAGE_LED PULSER_VOLTAGE_LED + 1
#define SEQUENCER_LED RANDOM_VOLTAGE_LED + 1
bool envolpeLedStatus = false;
bool pulserLedStatus = false;
uint32_t envolpeLedCount = 0;
uint32_t pulserLedCount = 0;

#define NUMPIXELS SEQUENCER_LED + 1

Adafruit_NeoPixel pixels(NUMPIXELS, DI_LEDS_DIN, NEO_GRBW + NEO_KHZ800);

//Blue
uint32_t sequencerColorHighlighted = pixels.Color(50, 0, 50);
uint32_t sequencerColor = pixels.Color(200, 0, 255);
//Yellow
uint32_t pulserColorHighlighted = pixels.Color(50, 50, 0);
uint32_t pulserColor = pixels.Color(255, 130, 0);
//White
uint32_t randomColorHighlighted = pixels.Color(25, 25, 25);
uint32_t randomColor = pixels.Color(255, 255, 255);
//Green
uint32_t envelopesColorHighlighted = pixels.Color(0, 50, 0);
uint32_t envelopesColor = pixels.Color(0, 255, 0);

uint32_t noneColor = pixels.Color(0, 0, 0);

uint32_t sourceColor[5] = { sequencerColor, pulserColor, randomColor, envelopesColor };
uint32_t sourceColorHighlighted[5] = { sequencerColorHighlighted, pulserColorHighlighted, randomColorHighlighted, envelopesColorHighlighted };

// ------------------ Timers ------------------------
HardwareTimer timerClock(TIM1);
HardwareTimer timerPulser(TIM2);
float pulserIncrement = 0.0f;
float pulserValue = 0.0f;

float pulserFrequency = 0.0f;
float pulserPeriodSecond = 0.0f;
float clockFrequency = 0.0f;
float clockPeriodSecond = 0.0f;

bool skipNextPulser = false;
bool skipNextClock = false;

float randomVoltageValue = 0.0f;
float adsr0Value = 0.0f;
float adsr1Value = 0.0f;

float clockValue = 0.0f;

void OnTimerClockInterrupt() {
  if (skipNextClock == false) {
    if (clockValue == 0.0f) {
      if (sequencerTriggerSource == SEQ_CLOCK_TRIGGER) {
        sequencerCurrentStepRead();
        if (sequencerSteps[sequencerIndex] == 1)
          currentSequencerStepEnable = false;
        else
          currentSequencerStepEnable = true;
        setSequencerLed(sequencerIndex);
        sequencerIndex = (sequencerIndex + 1) % sequencerLenght;
      }
      clockValue = 1.0f;

      if (pulserTriggerSource == CLOCK_TRIGGER || (pulserTriggerSource == SEQ_CLOCK_TRIGGER && currentSequencerStepEnable == true)) {
        pulserValue = 1.0f;
        setPulserLed(true);
      }
      if (randomVoltageTriggerSource == CLOCK_TRIGGER || (randomVoltageTriggerSource == SEQ_CLOCK_TRIGGER && currentSequencerStepEnable == true)) {
        generateRandomVoltage();
        setRandomLed(randomVoltageValue);
      }
      if (envelopeGenSig0TriggerSource == CLOCK_TRIGGER || (envelopeGenSig0TriggerSource == SEQ_CLOCK_TRIGGER && currentSequencerStepEnable == true)) {

        float fullTimeAttackRelease = clockPeriodSecond * envelopeGenSig0DecayFactor;

        multiShapeAdsr0.setAttackTime(fullTimeAttackRelease * slopeFactor);
        multiShapeAdsr0.setReleaseTime(fullTimeAttackRelease * (1.0f - slopeFactor));
        multiShapeAdsr0.retrigger();
      }
      if (envelopeGenSig1TriggerSource == CLOCK_TRIGGER || (envelopeGenSig1TriggerSource == SEQ_CLOCK_TRIGGER && currentSequencerStepEnable == true)) {

        float fullTimeAttackRelease = clockPeriodSecond * envelopeGenSig1DecayFactor;

        multiShapeAdsr1.setAttackTime(fullTimeAttackRelease * slopeFactor);
        multiShapeAdsr1.setReleaseTime(fullTimeAttackRelease * (1.0f - slopeFactor));
        multiShapeAdsr1.retrigger();
        setEnvelopeLed(true);
      }
    } else {
      clockValue = 0.0f;
    }

    if (avAnClockRate.hasValueUpdated()) {
      clockRate = simpleAnalogNormalize(avAnClockRate.getVal()) * 3.0f;  //0..3Hz = 0..180 bpm
      skipNextClock = true;                                              // when we reset the timer, it will trigger the interrupt directly, so we need to skip it once
      setClockFrequency(clockRate);
    }
  } else {
    skipNextClock = false;
  }
}

void OnTimerPulserInterrupt() {
  if (skipNextPulser == false) {
    if (sequencerTriggerSource == SEQ_PULSER_TRIGGER) {
      sequencerCurrentStepRead();
      if (sequencerSteps[sequencerIndex] == 1)
        currentSequencerStepEnable = false;
      else
        currentSequencerStepEnable = true;
      setSequencerLed(sequencerIndex);
      sequencerIndex = (sequencerIndex + 1) % sequencerLenght;
    }

    if (pulserTriggerSource == PULSER_TRIGGER || (pulserTriggerSource == SEQ_PULSER_TRIGGER && currentSequencerStepEnable == true)) {
      pulserValue = 1.0f;
      setPulserLed(true);
    }
    if (randomVoltageTriggerSource == PULSER_TRIGGER || (randomVoltageTriggerSource == SEQ_PULSER_TRIGGER && currentSequencerStepEnable == true)) {
      generateRandomVoltage();
      setRandomLed(randomVoltageValue);
    }
    if (envelopeGenSig0TriggerSource == PULSER_TRIGGER || (envelopeGenSig0TriggerSource == SEQ_PULSER_TRIGGER && currentSequencerStepEnable == true)) {

      float fullTimeAttackRelease = pulserPeriodSecond * envelopeGenSig0DecayFactor;

      multiShapeAdsr0.setAttackTime(fullTimeAttackRelease * slopeFactor);
      multiShapeAdsr0.setReleaseTime(fullTimeAttackRelease * (1.0f - slopeFactor));
      multiShapeAdsr0.retrigger();
    }
    if (envelopeGenSig1TriggerSource == PULSER_TRIGGER || (envelopeGenSig1TriggerSource == SEQ_PULSER_TRIGGER && currentSequencerStepEnable == true)) {

      float fullTimeAttackRelease = pulserPeriodSecond * envelopeGenSig1DecayFactor;

      multiShapeAdsr1.setAttackTime(fullTimeAttackRelease * slopeFactor);
      multiShapeAdsr1.setReleaseTime(fullTimeAttackRelease * (1.0f - slopeFactor));
      multiShapeAdsr1.retrigger();
      setEnvelopeLed(true);
    }
    if (avAnPulserPeriod.hasValueUpdated()) {
      float normalizedPulserVal = simpleAnalogNormalize(avAnPulserPeriod.getVal());
      pulserPeriod = fmap(normalizedPulserVal, 0, 6.0f, Mapping::EXP);  //0..6Hz = 0..360 bpm --> double of clock
      skipNextPulser = true;                                            // when we reset the timer, it will trigger the interrupt directly, so we need to skip it once
      setPulserFrequency(pulserPeriod);
    }
  } else {
    skipNextPulser = false;
  }
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);


  pinMode(DI_PATCH_IRQ, INPUT);

  for (int i = 0; i < 5; i++) {

    pinMode(DI_SEQUENCER_STEPSELECT[i], INPUT);
  }

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");

  if (!mcp.begin_I2C()) {
    Serial.println("MCP not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MCP found!");

  mcp.pinMode(DI_SEQUENCER_TRIGGER, INPUT_PULLUP);
  mcp.pinMode(DI_SEQUENCER_STAGE0, INPUT_PULLUP);
  mcp.pinMode(DI_SEQUENCER_STAGE1, INPUT_PULLUP);
  mcp.pinMode(DI_RANDOM_TRIGGERSELECT0, INPUT_PULLUP);
  mcp.pinMode(DI_RANDOM_TRIGGERSELECT1, INPUT_PULLUP);
  mcp.pinMode(DI_PULSER_TRIGGERSELECT0, INPUT_PULLUP);
  mcp.pinMode(DI_PULSER_TRIGGERSELECT1, INPUT_PULLUP);
  mcp.pinMode(DI_MODOSC_AMFM, INPUT_PULLUP);
  mcp.pinMode(DI_COMPLEXOSC_WAVEFORM0, INPUT_PULLUP);
  mcp.pinMode(DI_COMPLEXOSC_WAVEFORM1, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG0SELECTOR0, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG0SELECTOR1, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG0LPGVCA, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG1SELECTOR0, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG1SELECTOR1, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG1LPGVCA, INPUT_PULLUP);

  pinMode(DI_INTB, INPUT);
  pinMode(DI_INTA, INPUT);

  pinMode(DI_SEQUENCER_STEP4, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP3, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP2, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP1, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP0, INPUT_PULLUP);
  pixels.begin();
  pixels.clear();  // Set all pixel colors to 'off'

  for (int i = 0; i < OUTPUT_TOUCH_COUNT; i++) {
    pixels.setPixelColor(i, sourceColor[i]);
  }

  pixels.show();

  // DAISY SETUP
  DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  sample_rate = DAISY.get_samplerate();

  // init complex oscillators
  complexOscSinus.Init(sample_rate);
  complexOscSinus.SetWaveform(complexOscSinus.WAVE_SIN);
  complexOscSinus.SetAmp(1);

  complexOscBasis.Init(sample_rate);
  setComplexOscillatorFrequency(0.0f);

  modulationOsc.Init(sample_rate);
  setModulationOscillatorFrequency();

  setPulserFrequency(6);
  timerPulser.resume();  // Start
  timerPulser.attachInterrupt(OnTimerPulserInterrupt);

  setClockFrequency(3);
  timerClock.resume();  // Start
  timerClock.attachInterrupt(OnTimerClockInterrupt);

  // init adsr
  multiShapeAdsr0.Init(sample_rate, false);  //true = adsr, false = ar
  multiShapeAdsr0.setAttackTime(0.05);
  multiShapeAdsr0.setReleaseTime(0.05);
  multiShapeAdsr0.setAttackShapes(QUADRATIC_SHAPE, QUADRATIC_INVERT_SHAPE);
  multiShapeAdsr0.setDecayShapes(QUADRATIC_SHAPE, QUADRATIC_INVERT_SHAPE);
  multiShapeAdsr0.setReleaseShapes(QUADRATIC_SHAPE, QUADRATIC_INVERT_SHAPE);
  // multiShapeAdsr0.setDecayTime(0.1); //uncomment if adsr and not just ar
  // multiShapeAdsr0.setSustainLevel(.7); //uncomment if adsr and not just ar

  multiShapeAdsr1.Init(sample_rate, false);  //true = adsr, false = ar
  multiShapeAdsr1.setAttackTime(0.05);
  multiShapeAdsr1.setReleaseTime(0.1);
  multiShapeAdsr1.setAttackShapes(QUADRATIC_SHAPE, QUADRATIC_INVERT_SHAPE);
  multiShapeAdsr1.setDecayShapes(QUADRATIC_SHAPE, QUADRATIC_INVERT_SHAPE);
  multiShapeAdsr1.setReleaseShapes(QUADRATIC_SHAPE, QUADRATIC_INVERT_SHAPE);
  // multiShapeAdsr1.setDecayTime(0.1);//uncomment if adsr and not just ar
  // multiShapeAdsr1.setSustainLevel(.7);//uncomment if adsr and not just ar

  // init lpg
  lowPassGateFilter0.Init(sample_rate);
  lowPassGateFilter0.SetFreq(sample_rate);
  lowPassGateFilter1.Init(sample_rate);
  lowPassGateFilter1.SetFreq(sample_rate);

  chorus.Init(sample_rate);
  chorus.SetDelayMs(30.0f);
  chorus.SetLfoFreq(0.3f);
  chorus.SetLfoDepth(0.3f);
  chorus.SetFeedback(0.55f);

  //read onces all gpios before starting daisy
  digitalPinsread();
  mcpPinsRead();
  analogsRead();

  DAISY.begin(ProcessAudio);
}

void loop() {
  capacitiveStateMachine();
  digitalPinsread();
  mcpPinsRead();
  analogsRead();
  updateTimedLeds();
}


void ProcessAudio(float **in, float **out, size_t size) {
  float offsetSig0Adsr;
  float factorSig0Adsr;

  float offsetSig1Adsr;
  float factorSig1Adsr;

  // computation to do only once per processAudio pass
  if (envelopeGenSig0DecayFactor > 0.875f) {
    // envelopeGenSig0DecayFactor 0.875f .. 1
    // offsetSig0Adsr 0 .. 1
    // factorSig0Adsr 1 .. 0
    offsetSig0Adsr = 1.0f - (8.0f * (1.0f - envelopeGenSig0DecayFactor));
    factorSig0Adsr = 1.0f - offsetSig0Adsr;

  } else {
    offsetSig0Adsr = 0.0f;
    factorSig0Adsr = 1.0f;
  }

  if (envelopeGenSig1DecayFactor > 0.875f) {
    // envelopeGenSig1DecayFactor 0.875f .. 1
    // offsetSig1Adsr 0 .. 1
    // factorSig1Adsr 1 .. 0
    offsetSig1Adsr = 1.0f - (8.0f * (1.0f - envelopeGenSig1DecayFactor));
    factorSig1Adsr = 1.0f - offsetSig1Adsr;
  } else {
    offsetSig1Adsr = 0.0f;
    factorSig1Adsr = 1.0f;
  }

  for (size_t i = 0; i < size; i++) {

    pulserProcess();

    adsr0Value = multiShapeAdsr0.Process(false);  // for now there is no gate so we will only have adr
    adsr1Value = multiShapeAdsr1.Process(false);  // for now there is no gate so we will only have adr

    // we have attenuated Adsr Value to have a smooth transition between adsr to no adsr
    // those attenuated adsr are only for the lpg and vca, for the patch the non attenuated adsr value is used
    float attenuatedAdsr0Value = offsetSig0Adsr + adsr0Value * factorSig0Adsr;
    float attenuatedAdsr1Value = offsetSig1Adsr + adsr1Value * factorSig1Adsr;


    lowPassGateFilter0.SetFreq((sample_rate / 2) * (attenuatedAdsr0Value * attenuatedAdsr0Value));
    lowPassGateFilter1.SetFreq((sample_rate / 2) * (attenuatedAdsr1Value * attenuatedAdsr1Value));

    float modulatedComplexOscTimbre = computeModulatedComplexOscTimbre();
    float modulatedComplexOscAttenuator = computeModulatedComplexOscAttenuator();

    computeModulationOscWaveform();
    setModulationOscillatorFrequency();

    float modulationOscSample = modulationOsc.Process();

    float attenuatedModulationOscSample = modulationOscSample * modulatedComplexOscAttenuator;
    float attenuatedComplexMixed;


    if (modulatedComplexOscAttenuator > 0.1f) {
      if (modOscAmFm == 0) {  //0 = AM

        // si attenuator = 0 --> signal sans modulation  signal = attenuatedComplexMixed
        // si attenuator = 1 --> signal modulation max  signal = attenuatedComplexMixed * 100% of (attenuatedModulationOscSample)

        //si attenuator = 0 --> signal

        // modulationOscSample oscillate -1..1
        // modulatedComplexOscAttenuator 0..1 gain

        float gain = 1.0f - (((modulationOscSample + 1.0f) / 2) * modulatedComplexOscAttenuator);


        setComplexOscillatorFrequency(0.0f);
        computeModulationOscWaveform();
        float complexSinusSample = complexOscSinus.Process();
        float complexBasisSample = complexOscBasis.Process();

        float complexMixed = (1.0 - modulatedComplexOscTimbre) * complexSinusSample + modulatedComplexOscTimbre * complexBasisSample;
        attenuatedComplexMixed = complexMixed * gain;
      } else {  // 1 = FM

        // modulationOscSample oscillate -1..1
        //(modulationOscSample * 0.5f) -0.5 .. 0.5
        // modulatedComplexOscAttenuator 0..1 gain
        // fmFrequencyRamp --> example complexOscFrequency = 400hz --> 400 * 0.1 * ((-0.5..0.5)*0..1)

        float fmModulationFactor = 0.5 * ((modulationOscSample)*modulatedComplexOscAttenuator);
        setComplexOscillatorFrequency(fmModulationFactor);
        float complexSinusSample = complexOscSinus.Process();
        float complexBasisSample = complexOscBasis.Process();

        float complexMixed = (1.0 - modulatedComplexOscTimbre) * complexSinusSample + modulatedComplexOscTimbre * complexBasisSample;

        attenuatedComplexMixed = complexMixed;
      }
    } else {
      setComplexOscillatorFrequency(0.0f);
      float complexSinusSample = complexOscSinus.Process();
      float complexBasisSample = complexOscBasis.Process();

      float complexMixed = (1.0 - modulatedComplexOscTimbre) * complexSinusSample + modulatedComplexOscTimbre * complexBasisSample;
      attenuatedComplexMixed = complexMixed;
    }

    //apply filter even if not used to keep it up to date
    float lpgComplexMixed = lowPassGateFilter0.Process(attenuatedComplexMixed * 2.0f);
    //TODO apply envelopeGenSig0DecayFactor
    if (envelope0Enable) {

      if (envelopeGenSig0LpgVca == 0)  //LPG
      {
        attenuatedComplexMixed = lpgComplexMixed;
      } else  //VCA
      {
        attenuatedComplexMixed = attenuatedComplexMixed * attenuatedAdsr0Value;
      }
    }
    attenuatedComplexMixed = attenuatedComplexMixed * envelopeGenSig0Volume;

    float lpgModulationMixed = lowPassGateFilter1.Process(modulationOscSample * 2.0f);
    // add modulation osc
    if (envelopeGenSig1Volume > 0.02f) {
      if (envelope1Enable) {
        if (envelopeGenSig1LpgVca == 0)  //LPG
        {
          attenuatedComplexMixed = attenuatedComplexMixed + (lpgModulationMixed * envelopeGenSig1Volume) * modOscAttenuator;
        } else  //VCA
        {
          attenuatedComplexMixed = attenuatedComplexMixed + ((modulationOscSample * envelopeGenSig1Volume * attenuatedAdsr1Value)) * modOscAttenuator;
        }

      } else
        attenuatedComplexMixed = attenuatedComplexMixed + ((modulationOscSample * envelopeGenSig1Volume)) * modOscAttenuator;
    }

    float chorused = chorus.Process(attenuatedComplexMixed);

    out[0][i] = attenuatedComplexMixed;
    out[1][i] = chorused;
  }
}

void digitalPinsread() {
  sequencerSteps[0] = digitalRead(DI_SEQUENCER_STEP0);
  sequencerSteps[1] = digitalRead(DI_SEQUENCER_STEP1);
  sequencerSteps[2] = digitalRead(DI_SEQUENCER_STEP2);
  sequencerSteps[3] = digitalRead(DI_SEQUENCER_STEP3);
  sequencerSteps[4] = digitalRead(DI_SEQUENCER_STEP4);

  for (int i = 0; i < 5; i++) {
    if (sequencerStepsOld[i] != sequencerSteps[i]) {  //1 disabled 0 enabled
      sequencerStepsOld[0] = sequencerSteps[i];
    };
  }
}

void mcpPinsRead() {
  sequencerTrigger = mcp.digitalRead(DI_SEQUENCER_TRIGGER);
  sequencerStage = mcp.digitalRead(DI_SEQUENCER_STAGE0) + 2 * mcp.digitalRead(DI_SEQUENCER_STAGE1);
  randomTriggerSelect = mcp.digitalRead(DI_RANDOM_TRIGGERSELECT0) + 2 * mcp.digitalRead(DI_RANDOM_TRIGGERSELECT1);
  pulserTriggerSelect = mcp.digitalRead(DI_PULSER_TRIGGERSELECT0) + 2 * mcp.digitalRead(DI_PULSER_TRIGGERSELECT1);
  modOscAmFm = mcp.digitalRead(DI_MODOSC_AMFM);
  complexOscWaveform = mcp.digitalRead(DI_COMPLEXOSC_WAVEFORM0) + 2 * mcp.digitalRead(DI_COMPLEXOSC_WAVEFORM1);
  envelopeGenSig0Selector = mcp.digitalRead(DI_ENVELOPEGEN_SIG0SELECTOR0) + 2 * mcp.digitalRead(DI_ENVELOPEGEN_SIG0SELECTOR1);
  envelopeGenSig0LpgVca = mcp.digitalRead(DI_ENVELOPEGEN_SIG0LPGVCA);
  envelopeGenSig1Selector = mcp.digitalRead(DI_ENVELOPEGEN_SIG1SELECTOR0) + 2 * mcp.digitalRead(DI_ENVELOPEGEN_SIG1SELECTOR1);
  envelopeGenSig1LpgVca = mcp.digitalRead(DI_ENVELOPEGEN_SIG1LPGVCA);

  if (sequencerTriggerOld != sequencerTrigger) {  // 0 clock, 1 pulser
    if (sequencerTrigger == 0)
      sequencerTriggerSource = SEQ_CLOCK_TRIGGER;
    else
      sequencerTriggerSource = SEQ_PULSER_TRIGGER;

    if (pulserTriggerSelect == 1)
      pulserTriggerSource = sequencerTriggerSource;
    if (randomTriggerSelect == 1)
      randomVoltageTriggerSource = sequencerTriggerSource;
    if (envelopeGenSig0Selector == 1)
      envelopeGenSig0TriggerSource = sequencerTriggerSource;
    if (envelopeGenSig1Selector == 1)
      envelopeGenSig1TriggerSource = sequencerTriggerSource;
  }
  if (sequencerStageOld != sequencerStage) {  // 2 = 3steps, 3 = 4steps, 1 = 5steps
    switch (sequencerStage) {
      case 2:
        sequencerLenght = 3;
        break;
      case 3:
        sequencerLenght = 4;
        break;
      case 1:
        sequencerLenght = 5;
        break;
    }
  }
  if (randomTriggerSelectOld != randomTriggerSelect) {  //2 = pulser, 3 = clock, 1 = sequencer
    switch (randomTriggerSelect) {
      case 2:
        randomVoltageTriggerSource = PULSER_TRIGGER;
        break;
      case 3:
        randomVoltageTriggerSource = CLOCK_TRIGGER;
        break;
      case 1:
        randomVoltageTriggerSource = sequencerTriggerSource;
        break;
    }
  }

  if (pulserTriggerSelectOld != pulserTriggerSelect) {  //2 = pulser, 3 = clock, 1 = sequencer
    switch (pulserTriggerSelect) {
      case 2:
        pulserTriggerSource = PULSER_TRIGGER;
        break;
      case 3:
        pulserTriggerSource = CLOCK_TRIGGER;
        break;
      case 1:
        pulserTriggerSource = sequencerTriggerSource;
        break;
    }
  }

  if (complexOscWaveformOld != complexOscWaveform) {  // 2 = small pulse, 3 = square, 1 = triangle

    if (complexOscWaveform == 2) {
      complexOscBasis.SetPW(0.95f);
      complexOscBasis.SetWaveshape(1.0f);  // 1 = square
    } else if (complexOscWaveform == 3) {
      complexOscBasis.SetPW(0.5f);
      complexOscBasis.SetWaveshape(1.0f);
    } else if (complexOscWaveform == 1) {
      complexOscBasis.SetPW(0.5f);
      complexOscBasis.SetWaveshape(0.0f);  //0 = triangle
    }
  }

  if (envelopeGenSig0SelectorOld != envelopeGenSig0Selector) {  // 2 = pulser, 3 = clock, 1 = sequencer
    switch (envelopeGenSig0Selector) {
      case 2:
        envelopeGenSig0TriggerSource = PULSER_TRIGGER;
        break;
      case 3:
        envelopeGenSig0TriggerSource = CLOCK_TRIGGER;
        break;
      case 1:
        envelopeGenSig0TriggerSource = sequencerTriggerSource;
        break;
    }
  }

  if (envelopeGenSig1SelectorOld != envelopeGenSig1Selector) {  // 2 = pulser, 3 = clock, 1 = sequencer
    switch (envelopeGenSig1Selector) {
      case 2:
        envelopeGenSig1TriggerSource = PULSER_TRIGGER;
        break;
      case 3:
        envelopeGenSig1TriggerSource = CLOCK_TRIGGER;
        break;
      case 1:
        envelopeGenSig1TriggerSource = sequencerTriggerSource;
        break;
    }
  }

  sequencerTriggerOld = sequencerTrigger;
  sequencerStageOld = sequencerStage;
  randomTriggerSelectOld = randomTriggerSelect;
  pulserTriggerSelectOld = pulserTriggerSelect;
  modOscAmFmOld = modOscAmFm;
  complexOscWaveformOld = complexOscWaveform;
  envelopeGenSig0SelectorOld = envelopeGenSig0Selector;
  envelopeGenSig0LpgVcaOld = envelopeGenSig0LpgVca;
  envelopeGenSig1SelectorOld = envelopeGenSig1Selector;
  envelopeGenSig1LpgVcaOld = envelopeGenSig1LpgVca;
}

void analogsRead() {


  avAnClockRate.updateValue(analogRead(AN_CLOCK_RATE));
  avAnPulserPeriod.updateValue(analogRead(AN_PULSER_PERIOD));
  avAnModoscFrequency.updateValue(analogRead(AN_MODOSC_FREQUENCY));
  avAnModoscWaveform.updateValue(analogRead(AN_MODOSC_WAVEFORM));
  avAnModoscAttenuator.updateValue(analogRead(AN_MODOSC_ATTENUATOR));
  avAnComplexoscTimbre.updateValue(analogRead(AN_COMPLEXOSC_TIMBRE));
  avAnComplexoscFrequency.updateValue(analogRead(AN_COMPLEXOSC_FREQUENCY));
  avAnComplexoscAttenuator.updateValue(analogRead(AN_COMPLEXOSC_ATTENUATOR));
  avAnEnvelopegenSig0decay.updateValue(analogRead(AN_ENVELOPEGEN_SIG0DECAY));
  avAnEnvelopegenSig1decay.updateValue(analogRead(AN_ENVELOPEGEN_SIG1DECAY));

  avAnEnvelopeShape.updateValue(analogRead(AN_ENVELOPEGEN_SLOPESHAPE));

  if (avAnEnvelopeShape.hasValueUpdated()) {
    //shapeALowerBound = 1023
    uint16_t shapeAHigherBound = 723;
    uint16_t shapeBLowerBound = 300;
    //shapeBHigherBound = 0
    float lowerSlopeFactor = 0.01f;
    float higherSLopeFactor = 0.99f;

    uint32_t rawVal = avAnEnvelopeShape.getVal();

    if (rawVal > shapeAHigherBound) {  // from 1023 to 723, goes from 0.1 to 0.9 using shape A
      float tmpVal = map(rawVal, 1023, shapeAHigherBound, 0, 1023) / 1023.0f;
      slopeFactor = fmap(tmpVal, lowerSlopeFactor, higherSLopeFactor, Mapping::LINEAR);
      slopeMorphFactor = 0.0f;
    } else if (rawVal > shapeBLowerBound) {  // from 723 to 300 stay at 0.9 but pass from shape A, to shape B
      slopeFactor = higherSLopeFactor;

      float tmpVal = map(rawVal, shapeAHigherBound, shapeBLowerBound, 0, 1023) / 1023.0f;
      slopeMorphFactor = fmap(tmpVal, 0.0f, 1.0f, Mapping::LINEAR);

    } else {  // from 0 t0 300, goes from 0.9 to 0. using shape B
      float tmpVal = map(rawVal, shapeBLowerBound, 0, 1023, 0) / 1023.0f;
      slopeFactor = fmap(tmpVal, lowerSlopeFactor, higherSLopeFactor, Mapping::LINEAR);
      slopeMorphFactor = 1.0f;
    }

    multiShapeAdsr0.setShapeFactor(1.0f - slopeMorphFactor, slopeMorphFactor);
    multiShapeAdsr1.setShapeFactor(1.0f - slopeMorphFactor, slopeMorphFactor);
  }

  modOscWaveform = simpleAnalogNormalize(avAnModoscWaveform.getVal());
  modOscAttenuator = simpleAnalogNormalize(avAnModoscAttenuator.getVal());
  complexOscTimbre = simpleAnalogNormalize(avAnComplexoscTimbre.getVal());
  complexOscAttenuator = simpleAnalogNormalize(avAnComplexoscAttenuator.getVal());

  if (avAnComplexoscFrequency.hasValueUpdated()) {  //TODO make this better
    float complexOscVal = simpleAnalogNormalize(avAnComplexoscFrequency.getFVal());
    if (complexOscVal < 0.5)
      complexOscFrequency = fmap(complexOscVal * 2.0f, 0, 3000, Mapping::EXP);
    else
      complexOscFrequency = fmap((complexOscVal - 0.5f) * 2.0f, 3000, 8000, Mapping::LINEAR);
  }

  if (avAnModoscFrequency.hasValueUpdated()) {  //TODO make this better
    float normalizedModOscVal = simpleAnalogNormalize(avAnModoscFrequency.getFVal());
    if (normalizedModOscVal < 0.5)
      modOscFrequency = fmap(normalizedModOscVal * 2.0f, 0, 3000, Mapping::EXP);
    else
      modOscFrequency = fmap((normalizedModOscVal - 0.5f) * 2.0f, 3000, 8000, Mapping::LINEAR);
  }

  if (avAnModoscWaveform.hasValueUpdated()) {
    modOscWaveform = simpleAnalogNormalize(avAnModoscWaveform.getVal());
  }



  if (avAnEnvelopegenSig0decay.hasValueUpdated()) {
    envelopeGenSig0Decay = simpleAnalogNormalize(avAnEnvelopegenSig0decay.getVal());
    if (envelopeGenSig0Decay < 0.5f) {

      envelopeGenSig0Volume = 1.0f;
      // start to play with envelope, volume is 100%
      // if envelopeGenSig0Decay is at its minimum, disable envelope
      envelopeGenSig0DecayFactor = 1.0f-fmap(1.0f-(envelopeGenSig0Decay * 2.0f),0.0f, 1.0f, Mapping::EXP);
      if (envelope0Enable == false) {
        envelope0Enable = true;
      }
    } else {
      if (envelope0Enable == true) {
        envelope0Enable = false;
      }
      // mean decay is at its max, start to play with volume
      envelopeGenSig0DecayFactor = 1.0f;
      // envelopeGenSig0Decay from 0.5 to 1
      // envelopeGenSig0Volume from 1 to 0
      envelopeGenSig0Volume = 2.0f * (1.0f - envelopeGenSig0Decay);
    }
    envelopeGenSig0DecayOld = envelopeGenSig0Decay;
  }

  if (avAnEnvelopegenSig1decay.hasValueUpdated()) {
    
    envelopeGenSig1Decay = simpleAnalogNormalize(avAnEnvelopegenSig1decay.getVal());
    if (envelopeGenSig1Decay < 0.5f) {

      envelopeGenSig1Volume = 1.0f;
      // start to play with envelope, volume is 100%
      envelopeGenSig1DecayFactor = 1.0f-fmap(1.0f-(envelopeGenSig1Decay * 2.0f),0.0f, 1.0f, Mapping::EXP);
      if (envelope1Enable == false) {
        envelope1Enable = true;
      }
    } else {
      if (envelope1Enable == true) {
        envelope1Enable = false;
      }
      // mean decay is at its max, start to play with volume
      envelopeGenSig1DecayFactor = 1.0f;
      // envelopeGenSig1Decay from 0.5 to 1
      // envelopeGenSig1Volume from 1 to 0
      envelopeGenSig1Volume = 2.0f * (1.0f - envelopeGenSig1Decay);
    }
    envelopeGenSig1DecayOld = envelopeGenSig1Decay;
  }
}

void sequencerCurrentStepRead() {
  uint8_t nextSequencerIndex = (sequencerIndex + 1) % sequencerLenght;

  float sequencerAnalogValue = analogRead(AN_SEQUENCER_STEPANALOGIN);  //analog value goes from ~27 to ~920 because of 0.7V loss of the diode

  sequencerValue = (sequencerAnalogValue - (float)SEQUENCER_AN_MIN) / (float)SEQUENCER_AN_RANGE;
  if (sequencerValue > 1.0f)
    sequencerValue = 1.0f;
  if (sequencerValue < 0.0f)
    sequencerValue = 0.0f;

  //start with all pins to 0
  for (int i = 0; i < 5; i++) {

    pinMode(DI_SEQUENCER_STEPSELECT[i], INPUT);
  }

  //Prepare for next sequential voltage source
  pinMode(DI_SEQUENCER_STEPSELECT[nextSequencerIndex], OUTPUT);
  digitalWrite(DI_SEQUENCER_STEPSELECT[nextSequencerIndex], 1);
}

float semitone_to_hertz(int8_t note_number) {
  return 220 * pow(2, ((float)note_number - 0) / 12);
}

float simpleAnalogRead(uint32_t pin) {
  return (1023.0 - (float)analogRead(pin)) / 1023.0;
}

float simpleAnalogNormalize(uint32_t value) {
  return (1023.0 - (float)value) / 1023.0;
}

float simpleAnalogNormalize(float value) {
  return (1023.0 - value) / 1023.0;
}

// Reads a simple pot and maps it to a value bewtween to integer values
float simpleAnalogReadAndMap(uint32_t pin, long min, long max) {
  return map(1023 - analogRead(pin), 0, 1023, min, max);
}

float simpleAnalogMap(uint32_t value, long min, long max) {
  return map(1023 - value, 0, 1023, min, max);
}

int8_t capacitiveSensorTouch() {

  int8_t touchedIndex = -1;

  // get sensor value
  for (int i = 0; i < TOTAL_TOUCH_COUNT; i++) {
    capacitiveSensorRawValue[i] = cap.filteredData(i);
    if (capacitiveSensorRawValue[i] > THRESHOLD_TOUCHED) {
      capacitiveSensorTouched[i] = 0;
    } else {
      capacitiveSensorTouched[i] = 1;
    }
  }

  //detect if we have a touch
  for (int i = 0; i < TOTAL_TOUCH_COUNT; i++) {
    if (capacitiveSensorTouchedOld[i] != capacitiveSensorTouched[i] && capacitiveSensorTouched[i] == 1) {
      DEBUG_PRINT("Sensor n°");
      DEBUG_PRINT(i);
      DEBUG_PRINTLN(" touched");
      touchedIndex = i;
    }
  }

  //stor sensor value
  for (int i = 0; i < TOTAL_TOUCH_COUNT; i++) {
    capacitiveSensorTouchedOld[i] = capacitiveSensorTouched[i];
  }
  return touchedIndex;
}

void capacitiveStateMachine() {
  static int counter = 0;
  //TRANSITION
  switch (lastCapacitiveState) {
    case IDLE:
      capacitiveState = WAIT;
      break;

    case WAIT:
      {
        int8_t sensorTouchIndex = capacitiveSensorTouch();
        if (sensorTouchIndex > -1 && sensorTouchIndex < INPUT_TOUCH_COUNT) {
          capacitiveState = INPUT_PRESSED;
          inputPressedIndex = sensorTouchIndex;  //goes from 0 to 4
        }
        break;
      }

    case INPUT_PRESSED:
      capacitiveState = WAIT_OUTPUT;
      break;

    case WAIT_OUTPUT:
      {
        counter++;
        if (counter % 64 == 0) {
          pixels.setPixelColor(inputPressedIndex, sourceColorHighlighted[inputPressedIndex]);
          highlightedSource = SOURCE_MODULE(inputPressedIndex);
          pixels.show();
        } else if (counter % 32 == 0) {
          pixels.setPixelColor(inputPressedIndex, sourceColor[inputPressedIndex]);
          highlightedSource = SOURCE_MODULE(inputPressedIndex);
          pixels.show();
        }

        int8_t sensorTouchIndex = capacitiveSensorTouch();
        if (sensorTouchIndex >= INPUT_TOUCH_COUNT && sensorTouchIndex < (INPUT_TOUCH_COUNT + OUTPUT_TOUCH_COUNT)) {
          capacitiveState = OUTPUT_PRESSED;
          outputPressedIndex = sensorTouchIndex - INPUT_TOUCH_COUNT;  //so it goes from 0 to 5

        } else if (sensorTouchIndex == inputPressedIndex) {  //can only cancel if we repress the same input
          capacitiveState = INPUT_CANCELLED;
        }
        break;
      }

    case OUTPUT_PRESSED:
      capacitiveState = WAIT;
      break;

    case INPUT_CANCELLED:
      capacitiveState = WAIT;
      break;

    default:
      //Error
      break;
  };

  //CODE
  if (lastCapacitiveState != capacitiveState) {
    switch (capacitiveState) {
      case IDLE:
        DEBUG_PRINTLN("Enter IDLE State");
        break;
      case WAIT:
        DEBUG_PRINTLN("Enter WAIT State");
        break;
      case INPUT_PRESSED:
        DEBUG_PRINTLN("Enter INPUT_PRESSED State");

        pixels.setPixelColor(inputPressedIndex, sourceColorHighlighted[inputPressedIndex]);
        highlightedSource = SOURCE_MODULE(inputPressedIndex);
        pixels.show();

        break;
      case WAIT_OUTPUT:
        DEBUG_PRINTLN("Enter WAIT_OUTPUT State");
        counter = 0;
        break;
      case OUTPUT_PRESSED:
        DEBUG_PRINTLN("Enter OUTPUT_PRESSED State");
        if (destinationPatches[outputPressedIndex] == SOURCE_MODULE(inputPressedIndex)) {
          // repressed an existing pass --> cancel the patch
          destinationPatches[outputPressedIndex] = NONE_SOURCE;
          pixels.setPixelColor(outputPressedIndex + INPUT_TOUCH_COUNT, noneColor);
          pixels.show();
        } else {
          destinationPatches[outputPressedIndex] = SOURCE_MODULE(inputPressedIndex);

          pixels.setPixelColor(outputPressedIndex + INPUT_TOUCH_COUNT, sourceColor[inputPressedIndex]);
          pixels.show();
        }

        pixels.setPixelColor(inputPressedIndex, sourceColor[inputPressedIndex]);
        pixels.show();
        highlightedSource = NONE_SOURCE;

        break;
      case INPUT_CANCELLED:
        DEBUG_PRINTLN("Enter INPUT_CANCELLED State");

        pixels.setPixelColor(inputPressedIndex, sourceColor[inputPressedIndex]);
        pixels.show();
        highlightedSource = NONE_SOURCE;
        break;

      default:
        DEBUG_PRINTLN("Enter default State");
        //Error
        break;
    };
  }
  lastCapacitiveState = capacitiveState;
}

void setComplexOscillatorFrequency(float fmModulationFactor) {  //TODO
  float finalFrequency;

  if (destinationPatches[OSC_A_FRQ] != NONE_SOURCE) {
    float modulationFactor = getModulationFactorFromPatch(destinationPatches[OSC_A_FRQ]);
    finalFrequency = modulationFactor * complexOscFrequency;
  }

  else {
    finalFrequency = complexOscFrequency;
  }

  if (fmModulationFactor != 0.0f) {
    finalFrequency = finalFrequency + finalFrequency * fmModulationFactor;
  }

  complexOscBasis.SetFreq(finalFrequency);
  complexOscBasis.SetSyncFreq(finalFrequency);
  complexOscSinus.SetFreq(finalFrequency);
}

void setModulationOscillatorFrequency() {

  float finalFrequency;

  if (destinationPatches[OSC_B_FRQ] != NONE_SOURCE) {
    float modulationFactor = getModulationFactorFromPatch(destinationPatches[OSC_B_FRQ]);
    finalFrequency = modulationFactor * modOscFrequency;
  }

  else {
    finalFrequency = modOscFrequency;
  }

  modulationOsc.SetSyncFreq(finalFrequency);
  modulationOsc.SetFreq(finalFrequency);
}

void setPulserFrequency(float frequency) {

  if (frequency < 0.08f)
    frequency = 0.08;  //put 0.02 so overflow = 50000 --> which is 16 bits

  pulserFrequency = frequency;
  pulserPeriodSecond = 1 / pulserFrequency;
  //prescaler 16 bits, overflow 16bits
  timerPulser.setPrescaleFactor(40000);            // = Set prescaler to 4800 => timer frequency = 200MHz / 40000  = 5000 Hz
  timerPulser.setOverflow(int(5000 / frequency));  // Set overflow to 50000 => timer frequency = 10'000 Hz / frequency
  timerPulser.refresh();                           // Make register changes take effect

  pulserIncrement = frequency / sample_rate;
}

void setClockFrequency(float frequency) {

  if (frequency < 0.04f)
    frequency = 0.04;  //put 0.01 so overflow = 50000 --> which is 16 bits

  clockFrequency = frequency;
  clockPeriodSecond = 1 / clockFrequency;
  //prescaler 16 bits, overflow 16bits

  //frequency *2 to make a 50% duty cycle square signal

  timerClock.setPrescaleFactor(40000);                  // = Set prescaler to 4800 => timer frequency = 200MHz / 40000  = 5000 Hz
  timerClock.setOverflow(int(5000 / (frequency * 2)));  // Set overflow to 50000 => timer frequency = 10'000 Hz / frequency
  timerClock.refresh();                                 // Make register changes take effect
}

void pulserProcess() {
  pulserValue -= pulserIncrement;
  if (pulserValue < 0.0f)
    pulserValue = 0.0f;
}

void generateRandomVoltage() {
  randomVoltageValue = random(0, 1000) / 1000.0f;
  /*DEBUG_PRINT("random Voltage value ");
  DEBUG_PRINTLN(randomVoltageValue);*/
}


/*
//Blue
uint32_t sequencerColor = pixels.Color(0, 0, 120);
uint32_t sequencerColorHighlighted = pixels.Color(0, 0, 255);
//Yellow
uint32_t pulserColor = pixels.Color(100, 100, 0);
uint32_t pulserColorHighlighted = pixels.Color(200, 200, 0);
//White
uint32_t randomColor = pixels.Color(100, 100, 100);
uint32_t randomColorHighlighted = pixels.Color(200, 200, 200);
//Green
uint32_t envelopesColor = pixels.Color(0, 120, 0);
uint32_t envelopesColorHighlighted = pixels.Color(0, 255, 0);

uint32_t noneColor = pixels.Color(0, 0, 0);

#define SEQUENCER_LED TOTAL_TOUCH_COUNT + 1
#define RANDOM_VOLTAGE_LED SEQUENCER_LED + 1
#define PULSER_VOLTAGE_LED RANDOM_VOLTAGE_LED + 1
#define ENVELOPE_VOLTAGE_LED PULSER_VOLTAGE_LED + 1
*/
void setSequencerLed(uint8_t ledIndex) {
  uint8_t colorFactor = map(ledIndex, 0, 4, 20, 255);
  uint32_t color;
  if (currentSequencerStepEnable) {
    color = pixels.Color(colorFactor, 5, colorFactor);
  } else {
    color = noneColor;
  }

  pixels.setPixelColor(SEQUENCER_LED, color);
  pixels.show();
}
void setRandomLed(float randomVoltage) {
  uint8_t colorFactor = fmap(randomVoltage, 5, 200);
  pixels.setPixelColor(RANDOM_VOLTAGE_LED, pixels.Color(colorFactor, colorFactor, colorFactor));
  pixels.show();
}


void setPulserLed(bool ledStatus) {
  pulserLedStatus = ledStatus;
  if (ledStatus)
    pixels.setPixelColor(PULSER_VOLTAGE_LED, pulserColor);
  else
    pixels.setPixelColor(PULSER_VOLTAGE_LED, noneColor);
  pixels.show();
}
void setEnvelopeLed(bool ledStatus) {
  envolpeLedStatus = ledStatus;
  if (ledStatus)
    pixels.setPixelColor(ENVELOPE_VOLTAGE_LED, envelopesColorHighlighted);
  else
    pixels.setPixelColor(ENVELOPE_VOLTAGE_LED, noneColor);
  pixels.show();
}

void updateTimedLeds() {
  if (envolpeLedStatus)
    envolpeLedCount++;
  if (envolpeLedCount > LED_OFF_COUNT) {
    envolpeLedCount = 0;
    setEnvelopeLed(false);
  }

  if (pulserLedStatus) {
    pulserLedCount++;
  }
  if (pulserLedCount > LED_OFF_COUNT) {
    pulserLedCount = 0;
    setPulserLed(false);
  }
}

float computeModulatedComplexOscTimbre() {
  if (destinationPatches[OSC_A_TMBR] != NONE_SOURCE) {
    float modulationFactor = getModulationFactorFromPatch(destinationPatches[OSC_A_TMBR]);
    return modulationFactor * complexOscTimbre;
  } else {
    return complexOscTimbre;
  }
}

float computeModulatedComplexOscAttenuator() {
  if (destinationPatches[OSC_B_ATT] != NONE_SOURCE) {
    float modulationFactor = getModulationFactorFromPatch(destinationPatches[OSC_B_ATT]);
    return modulationFactor * complexOscAttenuator;
  } else {
    return complexOscAttenuator;
  }
}

void computeModulationOscWaveform() {
  float computedModOscWaveform;

  if (destinationPatches[OSC_B_FORM] != NONE_SOURCE) {  //TODO, maybe make the computedModOscWaveform goes from modOscWaveform up to 1.0 accoarding to modulationFactor
    computedModOscWaveform = getModulationFactorFromPatch(destinationPatches[OSC_B_FORM]) * modOscWaveform;
  } else {
    computedModOscWaveform = modOscWaveform;
  }

  if (computedModOscWaveform < 0.5) {
    //from tirangle to sawtooth 0 = triangle 0.5 = sawtooth
    // tirangle shape = 0, pw = 0.5
    //sawtooth shape =  0, pw 1
    modulationOsc.SetWaveshape(0.0f);  // 1 = square
    modulationOsc.SetPW(1.0f - (0.5f - computedModOscWaveform));

  } else {
    //from sawtooth to square: 0.5 = sawtooth 1 = square
    //sawtooth shape =  0, pw 1
    //square shape = 1, pw = 0.5
    modulationOsc.SetWaveshape((computedModOscWaveform - 0.5f) * 2);  // 1 = square
    modulationOsc.SetPW(1.0f - (computedModOscWaveform - 0.5f));
  }
}

float getModulationFactorFromPatch(int8_t patchValue) {
  switch (patchValue) {
    case NONE_SOURCE:
      return 0.0f;
    case SEQUENCER:
      return sequencerValue;
    case PULSER:
      return pulserValue;
    case RANDOM:
      return randomVoltageValue;
    case ENVELOPE_B:
      return adsr1Value;
  }
}
