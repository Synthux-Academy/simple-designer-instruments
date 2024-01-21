#pragma once

#include <stdint.h>

class ISource {
public:
    virtual ~ISource() {};

    virtual void set_frozen(bool frozen) = 0;
    virtual bool is_frozen() = 0;
    virtual void set_antifreeze(bool) = 0;
    
    virtual size_t length() = 0;

    virtual void set_cycle_start(size_t start) = 0;

    virtual void initialize() = 0;
    
    virtual void write(float in0, float in1) = 0;

    virtual size_t read_head() = 0;
    virtual void read(float& out0, float& out1, size_t frameIndex) = 0;
    
    virtual void reset() = 0;
};
