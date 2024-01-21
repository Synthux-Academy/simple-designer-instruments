#include "controller.h"

using namespace blptls;
using namespace spotykach;

using namespace daisy;

void Controller::initialize(DaisySeed& hw, Core& core, Clock& clock) {
    
    init_knobs(hw);
    init_toggles(hw);
    
    hw.adc.Start();

    init_sensor(core, clock);

    _store.initialize(hw);
    if (_store.is_updated()) {
        set_persisted(core);
    }
    else {
        auto i_a = core.engineAt(0).trig().pattern_indexes();
        store_pattern_index_a(i_a[0], Grid::even);
        store_pattern_index_a(i_a[1], Grid::c_word);

        auto i_b = core.engineAt(1).trig().pattern_indexes();
        store_pattern_index_b(i_b[0], Grid::even);
        store_pattern_index_b(i_b[1], Grid::c_word);
    }
}

void Controller::init_knobs(DaisySeed& hw) {
    size_t knobs_count = _knobs.size();
    
    AdcChannelConfig conf[knobs_count];
    
    size_t i;
    for (i = 0; i < knobs_count; i++) _knobs[i].initialize(conf[i], i);
        
    hw.adc.Init(conf, knobs_count);
    
    for (auto& k: _knobs) k.configure(hw);
}

void Controller::init_toggles(DaisySeed& hw) {
    _channel_toggles[0].initialize(hw, 0);
    _channel_toggles[1].initialize(hw, 1);
    _global_toggles.initialize(hw);
}

using Target = DescreteSensor::Target;
void Controller::init_sensor(Core& core, Clock &clock) {
    _sensor.initialize();

    auto& t_a = core.engineAt(0).trig();
    auto& t_b = core.engineAt(1).trig();

    _sensor.set_on_touch([&clock] { clock.toggle_is_running(); }, Target::PlayStop);
    _sensor.set_on_touch([&t_a, this] { this->store_pattern_index_a(t_a.prev_pattern(), t_a.grid()); }, Target::PatternMinusA);
    _sensor.set_on_touch([&t_a, this] { this->store_pattern_index_a(t_a.next_pattern(), t_a.grid()); }, Target::PatternPlusA);
    _sensor.set_on_touch([&t_b, this] { this->store_pattern_index_b(t_b.prev_pattern(), t_b.grid()); }, Target::PatternMinusB);
    _sensor.set_on_touch([&t_b, this] { this->store_pattern_index_b(t_b.next_pattern(), t_b.grid()); }, Target::PatternPlusB);
}

void Controller::store_pattern_index_a(int index, Grid g) {
    switch (g) {
        case Grid::c_word: _store.set_cword_pattern_a(index); break;
        case Grid::even: _store.set_even_pattern_a(index); break;
    };
}

void Controller::store_pattern_index_b(int index, Grid g) {
    switch (g) {
        case Grid::c_word: _store.set_cword_pattern_b(index); break;
        case Grid::even: _store.set_even_pattern_b(index);  break;
    };
}

void Controller::set_persisted(Core& core) {
    auto& t_a = core.engineAt(0).trig();
    auto& t_b = core.engineAt(1).trig();
    t_a.init_pattern_indexes({ _store.even_pattern_a(), _store.cword_pattern_a() });
    t_b.init_pattern_indexes({ _store.even_pattern_b(), _store.cword_pattern_b() });
}

void Controller::set_parameters(Core& core, Leds& leds, Clock& clck) {
    for (int i = 0; i < core.enginesCount(); i++) set_channel_toggles(core.engineAt(i), _channel_toggles[i], i);
    set_global_toggles(core);


    read_sensor(core, leds, clck);

    set_knob_parameters(core, clck);
};

using namespace blptls;
using namespace spotykach;
using KT = Knob::Target;
void Controller::set_knob_parameters(Core& s, Clock& clck) {
    auto& a = s.engineAt(0);
    auto& b = s.engineAt(1);
    for (size_t i = 0; i < _knobs.size(); i++) {
        auto t = _knobs[i].target();
        auto v = _knobs[i].value();
        switch (t) {
            case KT::SlicePositionA:    a.set_slice_position(v);    break;
            case KT::SliceLengthA:      a.set_slice_length(v);      break;
            case KT::RetriggerA:        a.trig().set_retrigger(v);  break;
            case KT::JitterAmountA:     a.set_jitter_amount(v);     break;
            case KT::Tempo:             clck.set_tempo(v);           break;
            case KT::VolumeCrossfade:   s.setVolumeBalance(v);      break;
            case KT::PatternCrossfade:  s.set_pattern_balance(v);   break;
            case KT::SlicePositionB:    b.set_slice_position(v);    break;
            case KT::SliceLengthB:      b.set_slice_length(v);      break;
            case KT::RetriggerB:        b.trig().set_retrigger(v);  break;
            case KT::JitterAmountB:     b.set_jitter_amount(v);     break;
            case KT::Pitch:             
            { 
                a.set_pitch_shift(v); 
                b.set_pitch_shift(v);
                break;
            }
            default:{}
        }
    }   
}

void Controller::set_channel_toggles(Engine& e, ChannelToggles& ct, int ei) {
    for (size_t i = 0; i < ct.count(); i++) {
        auto toggle = ct.at(i);
        auto target = std::get<0>(toggle);
        auto isOn = std::get<1>(toggle);
        auto holding_fwd = ei == 0 ? _holding_fwd_a : _holding_fwd_b;
        auto holding_rev = ei == 0 ? _holding_rev_a : _holding_rev_b;
        using Target = ChannelToggles::Target;
        switch (target) {
            case Target::Grid:      e.trig().set_grid(isOn ? 1 : 0); break;
            case Target::Reverse:   e.set_reverse((isOn && !holding_fwd) || holding_rev); break;
            default: {}
        }
    }
}

void Controller::set_global_toggles(Core& s) {
    using Target = GlobalToggles::Target;
    auto cnt = _global_toggles.count();
    for (size_t i = 0; i < cnt; i++) {
        auto toggle = _global_toggles.at(i);
        auto target = std::get<0>(toggle);
        auto isOn = std::get<1>(toggle);
        switch (target) {
            case Target::Mutex: s.setMutex(isOn); break;
            case Target::Cascade: s.setCascade(isOn); break;
            case Target::Split: s.setSplit(isOn); break;
        }
    }
}

void Controller::read_sensor(Core& core, Leds& leds, Clock& clock) {
    _sensor.process();

    auto& e_a = core.engineAt(0);
    auto& e_b = core.engineAt(1);

    _rec_a = _sensor.is_on(Target::RecordA);
    _rec_b = _sensor.is_on(Target::RecordB);

    e_a.set_frozen(!_rec_a);
    e_b.set_frozen(!_rec_b);

    _holding_fwd_a = _sensor.is_on(Target::OneShotFwdA);
    _holding_fwd_b = _sensor.is_on(Target::OneShotFwdB);
    _holding_rev_a = !_rec_a && _sensor.is_on(Target::OneShotRevA);
    _holding_rev_b = !_rec_b && _sensor.is_on(Target::OneShotRevB);
    

    auto holding_a = _holding_fwd_a || _holding_rev_a;
    auto holding_b = _holding_fwd_b || _holding_rev_b;

    auto is_clock_running = clock.is_running();

    PlaybackControls pc;
    pc.ctns_a = !(_rec_a && is_clock_running) && (_holding_fwd_a || _holding_rev_a);
    pc.ctns_b = !(_rec_b && is_clock_running) && (_holding_fwd_b || _holding_rev_b);
    pc.rev_a = _holding_rev_a;
    pc.rev_b = _holding_rev_b;
    core.set_playback_controls(pc);

    leds.set_led_a_on(holding_a && !is_clock_running);
    leds.set_led_b_on(holding_b && !is_clock_running);
    leds.set_rec_on(_rec_a || _rec_b);
}
