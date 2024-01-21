#pragma once
#ifndef INFS_SMOOTHED_VALUE_H
#define INFS_SMOOTHED_VALUE_H

#include <daisysp.h>
#include "DSPUtils.h"

namespace infrasonic {

class SmoothedValue {

public:

    explicit SmoothedValue(const float init_value, const float coef)
        : coef_(coef)
        , target_(init_value)
        , value_(init_value)
    {
    }

    inline float get()
    {
        if (!initialized_) {
            initialized_ = true;
            value_ = target_;
        } else {
            daisysp::fonepole(value_, target_, coef_);
        }

        return value_;
    }

    inline void set(const float target, const bool immediate = false)
    {
        target_ = target;
        if (immediate)
        {
            value_ = target;
        }
    }

    inline void setCoef(const float coef)
    {
        coef_ = daisysp::fclamp(coef, 0, 1);
    }

private:
    float coef_;
    float target_;
    float value_;
    bool initialized_ = false;
};

}

#endif