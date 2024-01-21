#pragma once

#include <stdint.h>
#include <functional>

class DescreteSensorPad {
public:
    enum class Mode {
        Toggle, //on on the first touch, off on the second
        Immediate //on on the touch, off on the release
    };

    DescreteSensorPad() = default;
    ~DescreteSensorPad() = default;

    std::function<void()> on_touch;
    std::function<void()> on_release;

    Mode mode = Mode::Immediate;

    void initialize(uint16_t mask) {
        _mask = mask;
    }

    void process(uint16_t state) {
        auto new_is_touched = (state & _mask) == _mask;

        // if it *wasn't* touched and now *is*
        if (new_is_touched && !_is_touched) {
            //call back
            if (on_touch) on_touch();
            
            //set state
            _is_on = (mode == Mode::Toggle) ? !_is_on : true;
        }
        // if it *was* touched and now *isnt*
        if (!new_is_touched && _is_touched) {
            //call back
            if (on_release) on_release();

            //set state
            if (mode == Mode::Immediate) _is_on = false;
        }

        _is_touched = new_is_touched;
    }

    bool is_on() {
        return _is_on;
    }

private:
    uint16_t _mask;
    bool _is_touched = false;
    bool _is_on = false; //toggle only
};
