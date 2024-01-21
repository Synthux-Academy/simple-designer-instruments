#pragma once

#include "daisy_seed.h"
#include "../common/fcomp.h"
#include "mi/pitch_shifter.h"
#include "mi/units.h"
#include "../core/buffers.h"

namespace blptls {
namespace spotykach {

class PitchShift {
public:
    PitchShift() = default;
    ~PitchShift() = default;

    void initialize(float const sr, float const delay) {
        ps_.Init(Buffers::pool().pitch_buf());
        ps_.set_ratio(stmlib::SemitonesToRatio(0));
	    ps_.set_size(1.0);
    }

    void setShift(float s) {
        if (s > 1.0) s = 1.0;
        if (s < 0) s = 0;

        float semitones = 0;
        if (fcomp(s, 0.5)) {
            _bypass = true;
        }
        else if (s < 0.5) {
            semitones = 48.0 * (s - 0.5);
            _bypass = false;
        }
        else {
            semitones = 24.0 * (s - 0.5);
            _bypass = false;
        }
        ps_.set_ratio(stmlib::SemitonesToRatio(semitones));
    }

    void process(float *in_out_0, float *in_out_1) {
        if (_bypass) return;

        clouds::FloatFrame f;
        f.l = *in_out_0;
        f.r = *in_out_1;

        ps_.ProcessFrame(&f);
        *in_out_0 = f.l;
        *in_out_1 = f.r;
    }

private:
    clouds::PitchShifter ps_;
    bool _bypass = true;
};

}
}
