#pragma once

#include <algorithm>
#include "hid/led.h"
#include "../core/globals.h"

namespace blptls {
namespace spotykach {

class Leds {
public:
    Leds() = default;
    ~Leds() = default;

    void initialize() {
    _led_a.Init(daisy::seed::D29, false, 31250);
    _led_b.Init(daisy::seed::D27, false, 31250);
    _led_r.Init(daisy::seed::D5, false);
    }

    void blink_a(uint32_t duration, bool reverse) {
        _led_a_duration = _max_duration * static_cast<float>(duration) / (kSliceMaxSeconds * kSampleRate);
        _led_a_duration = std::max(_led_a_duration, _sus_duration);
        _led_a_decay_treshld = _led_a_duration - _sus_duration;
        _led_a_rev = reverse;
    }

    void blink_b(uint32_t duration, bool reverse) {
        _led_b_duration = _max_duration * static_cast<float>(duration) / (kSliceMaxSeconds * kSampleRate);
        _led_b_duration = std::max(_led_b_duration, _sus_duration);
        _led_b_decay_treshld = _led_b_duration - _sus_duration;
        _led_b_rev = reverse;
    }

    void set_led_a_on(bool value) {
        _led_a.Set(value ? 1 : 0);
        _led_a.Update();
    }

    void set_led_b_on(bool value) {
        _led_b.Set(value ? 1 : 0);
        _led_b.Update();
    }

    void set_rec_on(bool value) {
        _led_r.Set(value ? 1 : 0);
        _led_r.Update();
    }

    void tick() {
        if (_led_a_duration > 0) {
            _led_a_duration --;
            _led_a.Set(led_brightness(_led_a_duration, _led_a_decay_treshld, _led_a_rev));
            _led_a.Update();
        }

        if (_led_b_duration > 0) {
            _led_b_duration --;
            _led_b.Set(led_brightness(_led_b_duration, _led_b_decay_treshld, _led_b_rev));
            _led_b.Update();
        }
    }

private:
    inline float led_brightness(const uint32_t duration, const float decay_treshld, const bool inv) {
        if (duration == 0) return 0;
        if (inv) return (duration > _sus_duration) ? 1.f - (duration - _sus_duration) / decay_treshld : 1;
        return duration > decay_treshld ? 1 : duration / decay_treshld;
    }

    daisy::Led _led_a;
    daisy::Led _led_b;
    daisy::Led _led_r;

    uint32_t _led_a_duration = 0;
    uint32_t _led_a_decay_treshld = 0;
    uint32_t _led_b_duration = 0;
    uint32_t _led_b_decay_treshld = 0;

    bool _led_a_rev = false;
    bool _led_b_rev = false;

    uint32_t _max_duration = 2634520;
    uint32_t _sus_duration = 100000;
};

}
}
