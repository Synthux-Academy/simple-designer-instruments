#pragma once

#include "i.generator.h"
#include "i.trigger.h"

static inline void adjustNextIndex(uint32_t* points, uint32_t pointsCount, uint32_t iterator, uint32_t& nextIndex) {
    int newNextIndex = 0;
    float nextDiff = INT32_MAX;
    for (uint32_t i = 0; i < pointsCount; i++) {
        auto point = points[i];
        if (point >= iterator && point - iterator < nextDiff) {
            nextDiff = point - iterator;
            newNextIndex = i;
        }
    }
    nextIndex = newNextIndex;
}

namespace blptls {
namespace spotykach {

class Trigger: public ITrigger {
public:
    Trigger(IGenerator& inGenerator);

    uint32_t beats_per_pattern() override { return _beats_per_pattern; };

    std::array<uint32_t, kGrid_Count> pattern_indexes() override { return _pattern_indexes; }
    void init_pattern_indexes(std::array<uint32_t, kGrid_Count> indexes) override;
    uint32_t next_pattern() override;
    uint32_t prev_pattern() override;

    void prepare_meter_pattern(uint32_t step, uint32_t shift) override;
    void prepare_cword_pattern(uint32_t onsets, uint32_t shift) override;

    void on_pattern_changed(std::function<void(uint32_t)> on_changed) override;

    Grid grid() override { return _grid; }
    void set_grid(float grid) override;
    void set_shift(float shift) override;
    void set_repeats(float repeats) override;
    void set_retrigger(float retrigger) override;
    uint32_t points_count() override { return _points_count; }

    void next(bool engaged) override;

    bool is_locking() override { return _ticks_till_unlock > 0; };

    void reset() override;

    int index = -1;

private:
    struct
    {
        float repeats = 1.f;
    } _raw;

    uint32_t set_pattern_index(uint32_t index);
    void prepare_pattern();
    void adjust_iterator();
    void adjust_repeats();

    IGenerator& _generator;

    std::function<void(uint32_t)> _on_pattern_changed;

    Grid _grid;
    std::array<uint32_t, kGrid_Count> _pattern_indexes;
    std::array<uint32_t, 64> _trigger_points;
    uint32_t _points_count;
    uint32_t _beats_per_pattern;
    uint32_t _next_point_index;
    uint32_t _iterator;

    uint32_t _onsets;
    uint32_t _repeats;
    uint32_t _step;
    uint32_t _shift;

    uint32_t _ticks_till_unlock;

    uint32_t _retrigger;
    uint32_t _repeats_to_retrigger;
    uint32_t _retrigger_distance;
    float _onset;
};

}
}
