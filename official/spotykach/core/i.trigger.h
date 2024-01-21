#pragma once

#include <functional>
#include "globals.h"

namespace blptls {
namespace spotykach {

class ITrigger {
public:
    virtual uint32_t points_count() = 0;
    virtual uint32_t beats_per_pattern() = 0;

    virtual std::array<uint32_t, kGrid_Count> pattern_indexes() = 0;
    virtual void init_pattern_indexes(std::array<uint32_t, kGrid_Count> indexes) = 0;
    virtual uint32_t next_pattern() = 0;
    virtual uint32_t prev_pattern() = 0;

    virtual void prepare_meter_pattern(uint32_t step, uint32_t shift) = 0;
    virtual void prepare_cword_pattern(uint32_t onsets, uint32_t shift) = 0;

    virtual void on_pattern_changed(std::function<void(uint32_t)> on_changed) = 0;

    virtual Grid grid() = 0;
    virtual void set_grid(float grid) = 0;
    virtual void set_shift(float shift) = 0;
    virtual void set_repeats(float repeats) = 0;
    virtual void set_retrigger(float retrigger) = 0;

    virtual void next(bool engaged) = 0;

    virtual bool is_locking() = 0;

    virtual void reset() = 0;

    virtual ~ITrigger() {};
};

}
}
