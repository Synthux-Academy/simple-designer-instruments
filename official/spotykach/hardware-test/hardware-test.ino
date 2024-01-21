#include "DaisyDuino.h"
#include "Adafruit_MPR121.h"
#include <Wire.h>

// #define TEST_LEDS
// #define TEST_FADER
// #define TEST_PADS
// #define TEST_TRIGGER
// #define TEST_KNOBS_A
// #define TEST_KNOBS_B
// #define TEST_KNOBS_C
// #define TEST_SWITCHES












void setup() {
  Serial.begin(9600);
  
  #ifdef TEST_SWITCHES
  setupSwitches();
  #endif

  #ifdef TEST_PADS
  setupPads();
  #endif

  #ifdef TEST_TRIGGER
  setupTriggerTest();
  #endif

  #ifdef TEST_LEDS
  testLED_A();
  testLED_B();
  testLED_R();
  #endif
}

void loop() {
  #ifdef TEST_PADS
  testPads();
  #endif

  #ifdef TEST_KNOBS_A
  testKnobsA();
  #endif
  
  #ifdef TEST_KNOBS_B
  testKonbsB();
  #endif

  #ifdef TEST_KNOBS_C
  testCommonKnobs();
  #endif

  #ifdef TEST_FADER
  testFader();
  #endif

  #ifdef TEST_SWITCHES
  testSwitches();
  #endif
  
  #ifdef TEST_TRIGGER
  testTrigger();
  #else
  delay(100);
  #endif
}

////////////////////////////////////////
//// KNOBS /////////////////////////////

enum class Knobs {
    JitterAmountA     = A10,
    SlicePositionA    = A9,
    SliceLengthA      = A8,
    RetriggerA        = A7,
    Tempo             = A6,
    VolumeCrossfade   = A11,
    PatternCrossfade  = A5,
    Pitch             = A4,
    RetriggerB        = A3,
    JitterAmountB     = A2,
    SlicePositionB    = A1,
    SliceLengthB      = A0
};

#ifndef K
#define KR(e) 1023 - analogRead((uint32_t)(Knobs::e))
#endif

void testKnobsA() {
  Serial.print("Slice Position A ");
  Serial.println(KR(SlicePositionA));
  Serial.print("Slice Length A ");
  Serial.println(KR(SliceLengthA));
  Serial.print("Retrigger A ");
  Serial.println(KR(RetriggerA));
  Serial.print("Jitter Amount A ");
  Serial.println(KR(JitterAmountA));
}

void testKonbsB() {
  Serial.print("Slice Position B ");
  Serial.println(KR(SlicePositionB));
  Serial.print("Slice Length B ");
  Serial.println(KR(SliceLengthB));
  Serial.print("Retrigger B ");
  Serial.println(KR(RetriggerB));
  Serial.print("Jitter Amount B ");
  Serial.println(KR(JitterAmountB));
}

void testCommonKnobs() {
  Serial.print("Tempo ");
  Serial.println(KR(Tempo));
  Serial.print("Pattern Crossfade ");
  Serial.println(KR(PatternCrossfade));
  Serial.print("Pitch ");
  Serial.println(KR(Pitch));
}

void testFader() {
  Serial.print("Volume Crossfade ");
  Serial.println(KR(VolumeCrossfade));
}

////////////////////////////////////////
//// SWITCHES //////////////////////////

enum class Switches {
    GridA     = D30,
    GridB     = D9,
    ReverseA  = D4,
    ReverseB  = D8,
    Mutex     = D7,
    Cascade   = D26,
    Split     = D6
};

#ifndef S
#define S(p) (uint32_t)Switches::p
#endif

#ifndef SR
#define SR(p) 1 - digitalRead(S(p))
#endif

void setupSwitches() {
  pinMode(S(GridA), INPUT_PULLUP);
  pinMode(S(GridB), INPUT_PULLUP);
  pinMode(S(ReverseA), INPUT_PULLUP);
  pinMode(S(ReverseB), INPUT_PULLUP);
  pinMode(S(Mutex), INPUT_PULLUP);
  pinMode(S(Cascade), INPUT_PULLUP);
  pinMode(S(Split), INPUT_PULLUP);
}

void testSwitches() {
  Serial.print("Grid A ");
  Serial.println(SR(GridA));
  Serial.print("Grid B ");
  Serial.println(SR(GridB));
  Serial.print("Reverse A ");
  Serial.println(SR(ReverseA));
  Serial.print("Reverse B ");
  Serial.println(SR(ReverseB));
  Serial.print("Mutex ");
  Serial.println(SR(Mutex));
  Serial.print("Cascade ");
  Serial.println(SR(Cascade));
  Serial.print("Split ");
  Serial.println(SR(Split));
}

////////////////////////////////////////
//// PADS //////////////////////////////

#ifndef _pin
#define _pin(shift) (1 << shift)
#endif

#ifndef _together
#define _together(a, b) (a | b)
#endif

enum class Pad {
    PlayStop      = _pin(5),
    OneShotFwdA   = _pin(4),
    OneShotRevA   = _pin(3),
    RecordA       = _together(_pin(3), _pin(4)),
    PatternMinusA = _pin(2),
    PatternPlusA  = _pin(1),
    OneShotFwdB   = _pin(7),
    OneShotRevB   = _pin(6),
    RecordB       = _together(_pin(6), _pin(7)),
    PatternMinusB = _pin(8),
    PatternPlusB  = _pin(9)
};

Adafruit_MPR121 cap = Adafruit_MPR121();

void setupPads() {
   if (!cap.begin(0x5A)) {
      Serial.println("MPR121 not found, check wiring?");
    while (1) {
      Serial.println("PLEASE CONNECT MPR121 TO CONTINUE TESTING");
      delay(200);
    }
  }
}

Pad pads[11] = { 
  Pad::PlayStop, 
  Pad::OneShotFwdA, 
  Pad::OneShotRevA, 
  Pad::RecordA, 
  Pad::PatternMinusA, 
  Pad::PatternPlusA, 
  Pad::OneShotFwdB, 
  Pad::OneShotRevB, 
  Pad::RecordB, 
  Pad::PatternMinusB, 
  Pad::PatternPlusB 
};

void testPads() {
  auto state = cap.touched();
  for (auto p: pads) {
    if ((state & (int)p) == (int)p) {
      switch (p) {
        case Pad::PlayStop: Serial.println("Play / Stop"); break;
        case Pad::OneShotFwdA: Serial.println("Fwd A"); break;
        case Pad::OneShotRevA: Serial.println("Rev A"); break;
        case Pad::RecordA: Serial.println("Rec A"); break;
        case Pad::PatternMinusA: Serial.println("Pattern minus A"); break;
        case Pad::PatternPlusA: Serial.println("Pattern plus A"); break;
        case Pad::OneShotFwdB: Serial.println("Fwd B"); break;
        case Pad::OneShotRevB: Serial.println("Rev B"); break;
        case Pad::RecordB: Serial.println("Rec B"); break;
        case Pad::PatternMinusB: Serial.println("Pattern minus B"); break;
        case Pad::PatternPlusB: Serial.println("Pattern plus B"); break;
      }
    }
  }  
}

////////////////////////////////////////
//// LEDS //////////////////////////////

void testLED_A() {
  pinMode(D29, OUTPUT);
  digitalWrite(D29, 1);
}

void testLED_B() {
  pinMode(D27, OUTPUT);
  digitalWrite(D27, 1);
}

void testLED_R() {
  pinMode(D5, OUTPUT);
  digitalWrite(D5, 1);
}

////////////////////////////////////////
//// TRIGGER //////////////////////////////

void setupTriggerTest() {
  pinMode(D10, INPUT_PULLDOWN);
  pinMode(LED_BUILTIN, OUTPUT);
}

void testTrigger() {
  auto trigger = digitalRead(D10);
  digitalWrite(LED_BUILTIN, !trigger);
}
