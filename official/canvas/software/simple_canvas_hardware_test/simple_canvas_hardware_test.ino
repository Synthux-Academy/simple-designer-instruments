#include "DaisyDuino.h"
using namespace daisysp;

#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_MCP23X17.h>

#include <Adafruit_NeoPixel.h>

//Comment or uncomment each of those lines to test a specific part of your synth

#define TEST_NEOPIXELS
//#define TEST_SEQUENCER_STEPS_READ
//#define TEST_ANALOGS_READ
//#define TEST_CAPACITIVE_TOUCH_READ
//#define TEST_MCP_PINS_READ
//#define TEST_DIGITAL_PINS_READ

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
//analog pins value
float sequencerStepAnalogIn[5];
float clockRate;
float envelopeGenSig1Decay;
float envelopeGenSlopeShape;
float pulserPeriod;
float modOscFrequency;
float modOscWaveform;
float modOscAttenuator;
float complexOscTimbre;
float complexOscFrequency;
float complexOscAttenuator;
float envelopeGenSig0Decay;

// ditial pins value
uint8_t sequencerStep4 = 0;
uint8_t sequencerStep3 = 0;
uint8_t sequencerStep2 = 0;
uint8_t sequencerStep1 = 0;
uint8_t sequencerStep0 = 0;

// mcp pins value
uint8_t sequencerTrigger = 0;
uint8_t sequencerStage0 = 0;
uint8_t sequencerStage1 = 0;
uint8_t randomTriggerSelect0 = 0;
uint8_t randomTriggerSelect1 = 0;
uint8_t pulserTriggerSelect0 = 0;
uint8_t pulserTriggerSelect1 = 0;
uint8_t modOscAmFm = 0;
uint8_t complexOscWaveform0 = 0;
uint8_t complexOscWaveform1 = 0;
uint8_t envelopeGenSig0Selector0 = 0;
uint8_t envelopeGenSig0Selector1 = 0;
uint8_t envelopeGenSig0LpgVca = 0;
uint8_t envelopeGenSig1Selector0 = 0;
uint8_t envelopeGenSig1Selector1 = 0;
uint8_t envelopeGenSig1LpgVca = 0;




// ----------------- Capacitive sensor ---------------------------
Adafruit_MPR121 cap = Adafruit_MPR121();

// ----------------- Neopixels -----------------------------------

#define NUMPIXELS 13

Adafruit_NeoPixel pixels(NUMPIXELS, DI_LEDS_DIN, NEO_GRBW + NEO_KHZ800);

// --------------------- Gpio expander --------------------------
Adafruit_MCP23X17 mcp;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {

    pinMode(DI_SEQUENCER_STEPSELECT[i], OUTPUT);
    digitalWrite(DI_SEQUENCER_STEPSELECT[i], 0);
  }

#ifdef TEST_CAPACITIVE_TOUCH_READ
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");
#endif

#ifdef TEST_MCP_PINS_READ
  if (!mcp.begin_I2C()) {
    Serial.println("MCP not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MCP found!");
#endif

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

#ifdef TEST_NEOPIXELS
  pixels.begin();
  pixels.clear();  // Set all pixel colors to 'off'
#endif

  // DAISY SETUP
  DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  DAISY.begin(ProcessAudio);
}

void loop() {
  // uncomment a line to do hardware test of specific part

#ifdef TEST_NEOPIXELS
  rgbNeopixels();
#endif
#ifdef TEST_SEQUENCER_STEPS_READ
  sequencerStepsRead();
#endif
#ifdef TEST_ANALOGS_READ
  analogsRead();
#endif
#ifdef TEST_CAPACITIVE_TOUCH_READ
  capacitiveTouchRead();
#endif
#ifdef TEST_MCP_PINS_READ
  mcpPinsRead();
#endif
#ifdef TEST_DIGITAL_PINS_READ
  digitalPinsread();
#endif

  delay(500);
}

void ProcessAudio(float **in, float **out, size_t size) {
}

void rgbNeopixels()
{
  Serial.print("Neopixels on PIN ");
  Serial.println(DI_LEDS_DIN);
  Serial.println("Set all leds to Red");
  pixels.fill(pixels.Color(255, 0, 0));
  pixels.show();
  delay(500);
  Serial.println("Set all leds to Green");
  pixels.fill(pixels.Color(0, 255, 0));
  pixels.show();
  delay(500);
  Serial.println("Set all leds to Blue");
  pixels.fill(pixels.Color(0, 0, 255));
  pixels.show();
  Serial.println();
}

void digitalPinsread() {
  sequencerStep0 = digitalRead(DI_SEQUENCER_STEP0);
  sequencerStep1 = digitalRead(DI_SEQUENCER_STEP1);
  sequencerStep2 = digitalRead(DI_SEQUENCER_STEP2);
  sequencerStep3 = digitalRead(DI_SEQUENCER_STEP3);
  sequencerStep4 = digitalRead(DI_SEQUENCER_STEP4);

  Serial.print("PIN ");
  Serial.print(DI_SEQUENCER_STEP0);
  Serial.print(" | Sequencer step 1 | ");
  Serial.println(sequencerStep0);
  Serial.print("PIN ");
  Serial.print(DI_SEQUENCER_STEP1);
  Serial.print(" | Sequencer step 2 | ");
  Serial.println(sequencerStep1);
  Serial.print("PIN ");
  Serial.print(DI_SEQUENCER_STEP2);
  Serial.print(" | Sequencer step 3 | ");
  Serial.println(sequencerStep2);
  Serial.print("PIN ");
  Serial.print(DI_SEQUENCER_STEP3);
  Serial.print(" | Sequencer step 4 | ");
  Serial.println(sequencerStep3);
  Serial.print("PIN ");
  Serial.print(DI_SEQUENCER_STEP4);
  Serial.print(" | Sequencer step 5 | ");
  Serial.println(sequencerStep4);
  Serial.println("");
}

void mcpPinsRead() {

  sequencerTrigger = mcp.digitalRead(DI_SEQUENCER_TRIGGER);
  sequencerStage0 = mcp.digitalRead(DI_SEQUENCER_STAGE0);
  sequencerStage1 = mcp.digitalRead(DI_SEQUENCER_STAGE1);
  randomTriggerSelect0 = mcp.digitalRead(DI_RANDOM_TRIGGERSELECT0);
  randomTriggerSelect1 = mcp.digitalRead(DI_RANDOM_TRIGGERSELECT1);
  pulserTriggerSelect0 = mcp.digitalRead(DI_PULSER_TRIGGERSELECT0);
  pulserTriggerSelect1 = mcp.digitalRead(DI_PULSER_TRIGGERSELECT1);
  modOscAmFm = mcp.digitalRead(DI_MODOSC_AMFM);
  complexOscWaveform0 = mcp.digitalRead(DI_COMPLEXOSC_WAVEFORM0);
  complexOscWaveform1 = mcp.digitalRead(DI_COMPLEXOSC_WAVEFORM1);
  envelopeGenSig0Selector0 = mcp.digitalRead(DI_ENVELOPEGEN_SIG0SELECTOR0);
  envelopeGenSig0Selector1 = mcp.digitalRead(DI_ENVELOPEGEN_SIG0SELECTOR1);
  envelopeGenSig0LpgVca = mcp.digitalRead(DI_ENVELOPEGEN_SIG0LPGVCA);
  envelopeGenSig1Selector0 = mcp.digitalRead(DI_ENVELOPEGEN_SIG1SELECTOR0);
  envelopeGenSig1Selector1 = mcp.digitalRead(DI_ENVELOPEGEN_SIG1SELECTOR1);
  envelopeGenSig1LpgVca = mcp.digitalRead(DI_ENVELOPEGEN_SIG1LPGVCA);
 
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_SEQUENCER_TRIGGER);
  Serial.print(" | Sequencer Trigger | ");
  Serial.println(sequencerTrigger); 

  Serial.print("MCP2307 PIN ");
  Serial.print(DI_SEQUENCER_STAGE0);
  Serial.print(" | Sequencer Stages 1/2 | ");
  Serial.println(sequencerStage0); 

  Serial.print("MCP2307 PIN ");
  Serial.print(DI_SEQUENCER_STAGE1);
  Serial.print(" | Sequencer Stages 2/2 | ");
  Serial.println(sequencerStage1); 

  Serial.print("MCP2307 PIN ");
  Serial.print(DI_RANDOM_TRIGGERSELECT0);
  Serial.print(" | Random Trigger Select 1/2 | ");
  Serial.println(randomTriggerSelect0); 

  Serial.print("MCP2307 PIN ");
  Serial.print(DI_RANDOM_TRIGGERSELECT1);
  Serial.print(" | Random Trigger Select 2/2 | ");
  Serial.println(randomTriggerSelect1); 

  Serial.print("MCP2307 PIN ");
  Serial.print(DI_PULSER_TRIGGERSELECT0);
  Serial.print(" | Pulser Trigger Select 1/2 | ");
  Serial.println(pulserTriggerSelect0);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_PULSER_TRIGGERSELECT1);
  Serial.print(" | Pulser Trigger Select 2/2 | ");
  Serial.println(pulserTriggerSelect1); 

  Serial.print("MCP2307 PIN ");
  Serial.print(DI_MODOSC_AMFM);
  Serial.print(" | Modulation Osc AM/FM | ");
  Serial.println(modOscAmFm);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_COMPLEXOSC_WAVEFORM0);
  Serial.print(" | Complex Osc Waveform 1/2 | ");
  Serial.println(complexOscWaveform0);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_COMPLEXOSC_WAVEFORM1);
  Serial.print(" | Complex Osc Waveform 2/2 | ");
  Serial.println(complexOscWaveform1);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_ENVELOPEGEN_SIG0SELECTOR0);
  Serial.print(" | Envelope A Source 1/2 | ");
  Serial.println(envelopeGenSig0Selector0);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_ENVELOPEGEN_SIG0SELECTOR1);
  Serial.print(" | Envelope A Source 2/2 | ");
  Serial.println(envelopeGenSig0Selector1);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_ENVELOPEGEN_SIG0LPGVCA);
  Serial.print(" | Envelope A VCA/LPG | ");
  Serial.println(envelopeGenSig0LpgVca);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_ENVELOPEGEN_SIG1SELECTOR0);
  Serial.print(" | Envelope B Source 1/2 | ");
  Serial.println(envelopeGenSig1Selector0);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_ENVELOPEGEN_SIG1SELECTOR1);
  Serial.print(" | Envelope B Source 2/2 | ");
  Serial.println(envelopeGenSig1Selector1);
   
  Serial.print("MCP2307 PIN ");
  Serial.print(DI_ENVELOPEGEN_SIG1LPGVCA);
  Serial.print(" | Envelope B VCA/LPG | ");
  Serial.println(envelopeGenSig1LpgVca);
  Serial.println("");
}

void capacitiveTouchRead() {
  uint16_t mpr121Touched = cap.touched();
  Serial.println("Capacitives value are arround 100-200 when not touched and below 80 when touched");
  for (int i = 0; i < 9; i++) {
    Serial.print("Capacitive pad n°");
    Serial.print(i);
    Serial.print(" value : ");
    Serial.print(cap.filteredData(i));
    Serial.println("");
  }
  Serial.println("");
}

void analogsRead() {
  clockRate = simpleAnalogRead(AN_CLOCK_RATE);
  pulserPeriod = simpleAnalogRead(AN_PULSER_PERIOD);
  modOscFrequency = simpleAnalogRead(AN_MODOSC_FREQUENCY);
  modOscWaveform = simpleAnalogRead(AN_MODOSC_WAVEFORM);
  modOscAttenuator = simpleAnalogRead(AN_MODOSC_ATTENUATOR);
  complexOscTimbre = simpleAnalogRead(AN_COMPLEXOSC_TIMBRE);
  complexOscFrequency = simpleAnalogRead(AN_COMPLEXOSC_FREQUENCY);
  complexOscAttenuator = simpleAnalogRead(AN_COMPLEXOSC_ATTENUATOR);
  envelopeGenSig0Decay = simpleAnalogRead(AN_ENVELOPEGEN_SIG0DECAY);
  envelopeGenSig1Decay = simpleAnalogRead(AN_ENVELOPEGEN_SIG1DECAY);
  envelopeGenSlopeShape = simpleAnalogRead(AN_ENVELOPEGEN_SLOPESHAPE);
  
  Serial.print("PIN 16");
  Serial.print(" or A1");
  Serial.print(" | Clock Rate | ");
  Serial.println(clockRate);

  Serial.print("PIN 17");
  Serial.print(" or A2");
  Serial.print(" | Pulser Period | ");
  Serial.println(pulserPeriod);

  
  Serial.print("PIN 18");
  Serial.print(" or A3");
  Serial.print(" | Modulation Osc Frequency | ");
  Serial.println(modOscFrequency);
  
  Serial.print("PIN 19");
  Serial.print(" or A4");
  Serial.print(" | Modulation Osc Waveform| ");
  Serial.println(modOscWaveform);
  
  Serial.print("PIN 20");
  Serial.print(" or A5");
  Serial.print(" | Modulation Osc Attenuator | ");
  Serial.println(modOscAttenuator);
  
  Serial.print("PIN 21");
  Serial.print(" or A6");
  Serial.print(" | Complex Osc timbre | ");
  Serial.println(complexOscTimbre);
  
  Serial.print("PIN 22");
  Serial.print(" or A7");
  Serial.print(" | Complex Osc Frequency | ");
  Serial.println(complexOscFrequency);
  
  Serial.print("PIN 23");
  Serial.print(" or A8");
  Serial.print(" | Complex Osc Attenuator | ");
  Serial.println(complexOscAttenuator);
  
  Serial.print("PIN 24");
  Serial.print(" or A9");
  Serial.print(" | Envelope A decay | ");
  Serial.println(envelopeGenSig0Decay);
  
  Serial.print("PIN 25");
  Serial.print(" or A10");
  Serial.print(" | Envelope B decy | ");
  Serial.println(envelopeGenSig1Decay);
  
  Serial.print("PIN 28");
  Serial.print(" or A11");
  Serial.print(" | Envelope Slope | ");
  Serial.println(envelopeGenSlopeShape);
  Serial.println("");
}

void sequencerStepsRead() {
  Serial.print("PIN 15");
  Serial.print(" or A0");
  Serial.println(" | Sequencer");
  Serial.println("Sequencer values goes from ~20 to ~900 due to diode voltage drop");
  //start with all pins to 0
  for (int i = 0; i < 5; i++) {

    pinMode(DI_SEQUENCER_STEPSELECT[i], INPUT);
  }


  for (int indexRead = 0; indexRead < 5; indexRead++) {
    for (int i = 0; i < 5; i++) {
      pinMode(DI_SEQUENCER_STEPSELECT[i], INPUT);
    }

    delay(10);

    pinMode(DI_SEQUENCER_STEPSELECT[indexRead], OUTPUT);
    digitalWrite(DI_SEQUENCER_STEPSELECT[indexRead], 1);
    sequencerStepAnalogIn[indexRead] = analogRead(AN_SEQUENCER_STEPANALOGIN);

    Serial.print("Setting PIN ");
    Serial.print(DI_SEQUENCER_STEPSELECT[indexRead]);
    Serial.print(" to 1");
    Serial.print(" | Sequencer Step ");
    Serial.print(indexRead+1);
    Serial.print(" | ");
    Serial.print(sequencerStepAnalogIn[indexRead]);
    Serial.println("");
  }
  Serial.println();
}

float semitone_to_hertz(int8_t note_number) {
  return 220 * pow(2, ((float)note_number - 0) / 12);
}

float simpleAnalogRead(uint32_t pin) {
  return (1023.0 - (float)analogRead(pin)) / 1023.0;
}

// Reads a simple pot and maps it to a value bewtween to integer values
float simpleAnalogReadAndMap(uint32_t pin, long min, long max) {
  return map(1023 - analogRead(pin), 0, 1023, min, max);
}
