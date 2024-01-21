#pragma once

#include <array>
#include <math.h>

namespace blptls {
namespace spotykach {
    static const uint32_t kEnginesCount     { 2 };

    static const uint32_t kSlicesCount      { 3 };
    static const uint32_t kSliceMaxSeconds  { 2 };
    static const uint32_t kSourceMaxSeconds { 10 };

    static const uint32_t kChannelsCount    { 2 };
    static const uint32_t kSampleRate       { 48000 };
    static const uint32_t kBufferSize       { 4 };

    static const float kSecondsPerMinute    { 60.0 };

    static constexpr float logVolume(float value) {
        return (pow(10.0, value) - 1) / 9.0;
    }

    static const int kDirections_Count = 2;
    enum class Direction {
        forward,
        reverse
    };

    static const int kGrid_Count = 2;
    enum class Grid {
        even,
        c_word
    };

    static const uint32_t kBeatsPerMeasure  { 4 };
    static const uint32_t kPPQN             { 96 };

    using Step = int;
    static constexpr std::array<Step, 11> EvenSteps {{
        4 * kPPQN,      //1
        3 * kPPQN,      //1 / 2.
        2 * kPPQN,      //1 / 2
        3 * kPPQN / 2,  //1 / 4.
        2 * kPPQN / 3,  //1 / 4T
        1 * kPPQN,      //1 / 4
        1 * kPPQN / 2,  //1 / 8
        1 * kPPQN / 3,  //1 / 8T
        1 * kPPQN / 4,  //1 / 16
        1 * kPPQN / 6,  //1 / 16T
        1 * kPPQN / 8,  //1 / 32
    }};

    using CWordOnsets = int;
    static constexpr std::array<CWordOnsets, 9> CWords {{
        3,
        5,
        6,
        7,
        9,
        10,
        11,
        12,
        13
    }};

    static constexpr int EvenStepsCount = EvenSteps.size();
    static constexpr int CWordsCount = CWords.size();

    static const float kTempoMin = 30;
    static const float kTempoMax = 250;
}
}
