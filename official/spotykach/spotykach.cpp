#include "daisy_seed.h"
#include "core/globals.h"
#include "core/core.h"
#include "control/controller.h"
#include "control/clock.h"
#include "control/leds.h"
#include "common/deb.h"
#include "control/clock.h"

using namespace daisy;
using namespace blptls;
using namespace spotykach;

DaisySeed hw;
Controller controller;
Core core;
PlaybackParameters p;
Clock clck;
Leds leds;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
	static int cnfg_cnt { 0 };
	if (++cnfg_cnt == 40) {
		p.tempo = clck.tempo();
		p.sampleRate = kSampleRate;
		cnfg_cnt = 0;
	}
	clck.tick();
	core.preprocess(p);
	core.process(in, out, size);
}

int main(void) {
	hw.Configure();
	hw.Init();

	//HW::hw().setHW(&hw);
	//HW::hw().startLog();

	core.initialize();
	clck.run(core);
	controller.initialize(hw, core, clck);

	leds.initialize();
	core.engineAt(0).set_on_slice([](uint32_t sl, bool rev){ leds.blink_a(sl, rev); });
	core.engineAt(1).set_on_slice([](uint32_t sl, bool rev){ leds.blink_b(sl, rev); });

	hw.SetAudioBlockSize(kBufferSize);
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);

	uint32_t count_limit = 10e2;
	while(1) {
		clck.pull(hw);
		leds.tick();
		static uint32_t counter = 0;
		if (++counter == count_limit ) {
			counter = 0;
			controller.set_parameters(core, leds, clck);
		}
	}
}
