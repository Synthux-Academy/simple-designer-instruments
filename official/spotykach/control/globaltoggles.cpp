#include "globaltoggles.h"

using namespace daisy;

void GlobalToggles::initialize(DaisySeed hw) {
    for (size_t i = 0; i < kTargetsCount; i++) {
        _switches[i].Init(
            pin(_targets[i]), 
            0, 
            Switch::Type::TYPE_TOGGLE,
            Switch::Polarity::POLARITY_INVERTED,
            Switch::Pull::PULL_UP);
    }
}

std::tuple<GlobalToggles::Target, GlobalToggles::IsOn> GlobalToggles::at(int index) {
    return { _targets[index], isOnAt(index) };
}

bool GlobalToggles::isOnAt(int index) {
    _switches[index].Debounce();
    return _switches[index].Pressed();
}
