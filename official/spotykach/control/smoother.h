#pragma once

#include <math.h>

struct Smoother {
public:
    Smoother() = default;
    ~Smoother() = default;

    float smoothing() const { 
        return _smooth_k; 
    }

    void setSmoothing(float s) {
        if (s >= 0.5 && s <= 1.f) _smooth_k = s;
    }

    float smoothed(float rough) {
        if (_val == 0) {
            _val = rough;
        }
        else {
            auto smth = rough * _smooth_k + (1 - _smooth_k) * _val;
            _val = floorf(smth * 1000.f) / 1000.f;
        }

        return _val;
    }

private:
    float _val      = 0;
    float _smooth_k = 0.75;
};
