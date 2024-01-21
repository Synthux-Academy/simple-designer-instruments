#pragma once

#include "daisy_seed.h"

struct ChannelToggles {
public:
    enum class Target {
        Grid,
        Reverse
    };

    ChannelToggles() = default;
    ~ChannelToggles() = default;
    
    using Channel = int;
    void initialize(daisy::DaisySeed hw, Channel ch);

    size_t count() { return kTargetsCount; }

    using IsOn = bool;
    std::tuple<Target, IsOn> at(int index);

private:
    bool isOnAt(int index);

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreturn-type"
    daisy::Pin pin(Target t, Channel ch) {
        using namespace daisy;
        using namespace seed;
        auto ch_a { ch == 0 };
        switch (t) {
        case Target::Grid:      return ch_a ? D30 : D9;
        case Target::Reverse:   return ch_a ? D4 : D8;
        };
    };
    #pragma GCC diagnostic pop

    static const int kTargetsCount { 5 };
    constexpr static std::array<Target, kTargetsCount> _targets = {
        Target::Grid,
        Target::Reverse
    };

    std::array<daisy::Switch, kTargetsCount> _switches;
};
