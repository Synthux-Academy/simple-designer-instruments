#pragma once

#include "daisy_seed.h"
#include "knob.h"
#include "mux8.h"
#include "globaltoggles.h"
#include "channeltoggles.h"
#include "core.h"
#include "descrete.sensor.h"
#include "leds.h"
#include "persistense.h"
#include "clock.h"

namespace blptls {
namespace spotykach {

class Controller {
public:
    Controller() = default;
    ~Controller() = default;

    void initialize(daisy::DaisySeed& hw, Core& core, Clock& clock);

    void set_parameters(Core& core, Leds& leds, Clock& clck);

    bool is_playing();

    bool holding_fwd_a() { return _holding_fwd_a; };
    bool holding_fwd_b() { return _holding_fwd_b; };
    bool holding_rev_a() { return _holding_rev_a; };
    bool holding_rev_b() { return _holding_rev_b; };

private:
    void init_sensor(Core& core, Clock &clock);
    void init_knobs(daisy::DaisySeed& hw);
    void init_toggles(daisy::DaisySeed& hw);
    void set_persisted(Core& core);
    void set_knob_parameters(Core &s, Clock& clck);
    void set_channel_toggles(Engine& e, ChannelToggles& ct, int i);
    void set_global_toggles(Core& s);
    void read_sensor(Core& core, Leds& leds, Clock& clck);

    void store_pattern_index_a(int index, Grid g);
    void store_pattern_index_b(int index, Grid g);

    DescreteSensor _sensor;
    std::array<Knob, 12> _knobs;
    std::array<ChannelToggles, 2> _channel_toggles;
    GlobalToggles _global_toggles;
    Persistence _store;

    bool _holding_fwd_a = false;
    bool _holding_fwd_b = false;
    bool _holding_rev_a = false;
    bool _holding_rev_b = false;
    bool _rec_a = false;
    bool _rec_b = false;
};
}
}
