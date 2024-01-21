#pragma once

#include "daisy_seed.h"

class HW {
public:
    static HW& hw() {
        static HW instance;
        return instance;
    }
    HW(HW const&) = delete;
    void operator=(HW const&)  = delete;

    void setHW(daisy::DaisySeed* hw) {
        _hw = hw;
        
    }

    void setLed(bool led) {
        _hw->SetLed(led);
    }

    void startLog() {
        _hw->StartLog();
    }

    template <typename... VA>
    void print(const char* format, VA... va) {
        _hw->PrintLine(format, va...);
    }


private:
    HW() = default;

    daisy::DaisySeed* _hw;
};

#define LOGFLOAT(v) HW::hw().print("%d", int(v * 1000));
#define LOGBOOL(v) HW::hw().print("%d", v);
#define LOGINT(v) HW::hw().print("%d", v);
