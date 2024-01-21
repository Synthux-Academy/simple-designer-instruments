#include "channeltoggles.h"

using namespace daisy;

void ChannelToggles::initialize(DaisySeed hw, Channel ch) {
    for (size_t i = 0; i < kTargetsCount; i++) {
        _switches[i].Init(
            pin(_targets[i], ch),
            0,
            Switch::Type::TYPE_TOGGLE,
            Switch::Polarity::POLARITY_INVERTED,
            Switch::Pull::PULL_UP);
    }
}

bool ChannelToggles::isOnAt(int index) {
    _switches[index].Debounce();
    return _switches[index].Pressed();
}

std::tuple<ChannelToggles::Target, ChannelToggles::IsOn> ChannelToggles::at(int index) {
    return { _targets[index], isOnAt(index) };
};
