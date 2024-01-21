#include "trigger.h"
#include <cmath>
#include <algorithm>
#include "globals.h"

namespace blptls {
namespace spotykach {

Trigger::Trigger(IGenerator& inGenerator) :
    _generator              { inGenerator },
    _grid                   { Grid::c_word },
    _pattern_indexes        { 6, 4 },
    _points_count           { 9 },
    _beats_per_pattern      { 4 },
    _next_point_index       { 0 },
    _iterator               { 0 },
    _onsets                 { 9 },
    _repeats                { 9 },
    _ticks_till_unlock      { 0 },
    _retrigger              { 0 },
    _repeats_to_retrigger   { 0 },
    _retrigger_distance     { 0 },
    _onset                  { 0 }
    {}


void Trigger::init_pattern_indexes(std::array<uint32_t, kGrid_Count> indexes) {
    _pattern_indexes = indexes;
}

uint32_t Trigger::next_pattern() {
    auto index = _pattern_indexes[uint32_t(_grid)];
    return set_pattern_index(++index);
}

uint32_t Trigger::prev_pattern() {
    auto index = _pattern_indexes[uint32_t(_grid)];
    return set_pattern_index(-- index);
}

uint32_t Trigger::set_pattern_index(uint32_t index) {
    auto step = _step;
    auto onsets = _onsets;
    uint32_t max_index = _grid == Grid::even ? EvenStepsCount - 1 : CWordsCount - 1;
    index = std::max(std::min(index, max_index), 0ul);

    _pattern_indexes[uint32_t(_grid)] = index;

    switch (_grid) {
        case Grid::even: step = EvenSteps[index]; break;
        case Grid::c_word: onsets = CWords[index]; step = kPPQN / 4; /*1/16*/ break;
    }

    if (step != _step || onsets != _onsets) {
        _step = step;
        _onsets = onsets;
        prepare_pattern();
    }

    return index;
}

void Trigger::set_grid(float normVal) {
    Grid grid = Grid(normVal * (kGrid_Count - 1));
    if (grid != _grid) {
        _grid = grid;
        set_pattern_index(_pattern_indexes[uint32_t(_grid)]);
    }
}

void Trigger::set_shift(float normVal) {
    auto shiftValue = static_cast<uint32_t>(normVal * kBeatsPerMeasure * kPPQN);
    _shift = shiftValue;
    prepare_pattern();
}

void Trigger::prepare_pattern() {
    if (_grid == Grid::c_word) {
        prepare_cword_pattern(_onsets, _shift);
    }
    else {
        prepare_meter_pattern(_step, _shift);
    }
    if (_on_pattern_changed) _on_pattern_changed(_step);
}

void Trigger::on_pattern_changed(std::function<void (uint32_t)> on_changed) {
    _on_pattern_changed = on_changed;
}

void Trigger::prepare_cword_pattern(uint32_t onsets, uint32_t shift) {
    _points_count = 0;
    _trigger_points.fill(0);
    const uint32_t size = 16;
    uint32_t y = onsets, a = y;
    uint32_t x = size - onsets, b = x;
    std::array<char, size> pattern;

    pattern[0] = 1;

    size_t i = 1;
    while (a != b) {
        if (a > b) {
            pattern[i] = 1;
            b += x;
        }
        else {
            pattern[i] = 0;
            a += y;
        }
        i++;
    }

    pattern[i] = 0;
    i++;

    if (i < size) {
        const auto offset = i;
        i = 0;
        while (i + offset < size) {
            pattern[i + offset] = pattern[i];
            i++;
        }
    }

    _beats_per_pattern = kBeatsPerMeasure;

    auto ticks_per_16th = kPPQN / 4;
    auto ticks_per_pattern = _beats_per_pattern * kPPQN;
    for (uint32_t i = 0; i < pattern.size(); i++) {
        if (!pattern[i]) continue;
        auto point = i * ticks_per_16th + shift;
        if (point >= ticks_per_pattern) {
            point -= ticks_per_pattern;
        }
        _trigger_points[_points_count] = point;
        _points_count ++;
    }
    adjust_repeats();
    adjust_iterator();
}

void Trigger::prepare_meter_pattern(uint32_t step, uint32_t shift) {
    _points_count = 0;
    _trigger_points.fill(0);
    uint32_t pattern_length { 0 };
    while (pattern_length % kPPQN || pattern_length < kPPQN * kBeatsPerMeasure) {
        _trigger_points[_points_count] = pattern_length;
        _points_count ++;
        pattern_length += step;
    }
    _beats_per_pattern = pattern_length / kPPQN;
    uint32_t ticks_per_pattern = _beats_per_pattern * kPPQN;
    for (uint32_t i = 0; i < _points_count; i++) {
        auto point = _trigger_points[i] + shift;
        if (point >= ticks_per_pattern) {
            point -= ticks_per_pattern;
        }
        _trigger_points[i] = point;
    }
    adjust_repeats();
    adjust_iterator();
}

void Trigger::adjust_repeats() {
    auto rnd = round(_raw.repeats * _points_count);
    _repeats = std::max(static_cast<uint32_t>(rnd), 1ul);
}

void Trigger::adjust_iterator() {
    adjustNextIndex(_trigger_points.data(), _points_count, _iterator, _next_point_index);
}

void Trigger::set_retrigger(const float norm_val) {
    _retrigger = round(norm_val * 16);
}

void Trigger::set_repeats(const float repeats) {
    _raw.repeats = repeats;
}

void Trigger::next(const bool engaged) {
    if (_ticks_till_unlock > 0) _ticks_till_unlock--;
    if (_iterator == _trigger_points[_next_point_index]) {
        if (engaged && _next_point_index < _repeats) {
            if (_retrigger) {
                _repeats_to_retrigger ++;
                _retrigger_distance += _trigger_points[_next_point_index];
                if (_repeats_to_retrigger % _retrigger == 0) {
                    _onset += static_cast<float>(_retrigger_distance) / kPPQN;
                    if (_onset >= 2048.f) _onset = 0;
                    _retrigger_distance = 0;
                    _repeats_to_retrigger = 0;
                }
            }
            _generator.activate_slice(_onset, 0);
            _ticks_till_unlock = 1;
        }
        _next_point_index = (_next_point_index + 1) % _points_count;
    }
    _iterator = (_iterator + 1) % (_beats_per_pattern * kPPQN);
}

void Trigger::reset() {
    _onset = 0;
    _repeats_to_retrigger = 0;
    _retrigger_distance = 0;
    _iterator = 0;
    _next_point_index = 0;
    _ticks_till_unlock = 0;
}

}
}
