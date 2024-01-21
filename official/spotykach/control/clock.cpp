#include "clock.h"
#include "../common/fcomp.h"
#include "../core/globals.h"

using namespace blptls;
using namespace spotykach;

void Clock::run(Clockable& clockable) {
    _clockable = &clockable;
    daisy::GPIO::Config cfg;
    cfg.pin = daisy::seed::D10;
	g.Init(cfg);
}

// Called by internal interrupt timer (audio callback in this implementation).
void Clock::tick() {
    if (!_is_running) return;
    emit_ticks();
}

/*
Derived from Maximum MIDI Music Applications in C++ by Paul Messick
This method generates internal ticks and also synchronises to the external clock.
So it's called both from internal interrupt timer (audio callback in this implementation) and upon external clock tick reception.
nticks - integer count of internal ticks
_fticks - fractional count of internal ticks
_tempo_ticks - integer ticks count since last external clock
_tempo_mks - tempo in microseconds / beat (quarter note)
_resync - flag to resync to external clock. Is set to true once external clock tick is received.
_hold - flag to stop advancing internal timeline if the number of internal ticks exceeded expected count of internal ticks per extrnal tick
kTRTime - internal resolution (ppqn) multiplied by interrupt interval.
*/
void Clock::emit_ticks() {
    uint32_t nticks = 0;

    //If we generated more internal ticks per extrnal tick as expected,
    //we don't advance internal "timeline", but only accumulate _tempo_ticks
    //in order to calculate and correct the tempo.
    //This flag is set to false upon reception of the external tick.
    if (_hold) {
        nticks = (_fticks + kTRtime) / _tempo_mks;
        _fticks += kTRtime - (nticks * _tempo_mks);
        _tempo_ticks += nticks;
        return;
    }

    //Once a tick of the extrnal clock is received,
    //we do resync, i.e. align inernal timeline with the external one
    //and adjust tempo.
    if (_resync) {
        _fticks = 0;
        nticks = kTicksPerClock - (_ticks - _ticks_at_last_clock);
        _ticks_at_last_clock = _ticks + nticks;
        _tempo_mks -= ((int32_t)kTicksPerClock - (int32_t)_tempo_ticks) * (int32_t)_tempo_mks / (int32_t)kPPQN;
        _tempo_ticks = 0;
        _resync = false;
    }
    //Regular mode. We generate internal ticks.
    else {
        nticks = (_fticks + kTRtime) / _tempo_mks;
        _fticks += kTRtime - nticks * _tempo_mks;
        if (external_clock()) {
            _tempo_ticks += nticks;
            //If there are more internal ticks per the external tick than 
            //expected, we set _hold to true effectively stopping advancing timeline
            //until next external tick
            if (_ticks - _ticks_at_last_clock + nticks >= kTicksPerClock) {
                nticks = kTicksPerClock - 1 - (_ticks - _ticks_at_last_clock);
                _hold = true;
            }
        }
    }

    //Accumulate ticks
    _ticks += nticks;

    //Advance timeline
    for (uint32_t i = 0; i < nticks; i++) _clockable->tick();
}

/*
In case of external clock sync this
method only schedules playback. Actual playback 
starts on the first tick of the external clock.
see clock_in_tick() below.
*/
void Clock::toggle_is_running() { 
    if (!_is_running) {
        if (external_clock()) _is_about_to_run = true;
        else _is_running = true;
    }
    else {
        _is_running = false;
        reset();
    }
}

/*
Setting tempo from internal control. 
Has no effect in case of syncing to extrnal clock.
*/
void Clock::set_tempo(float norm_value) {
    if (fcomp(norm_value, _raw_manual_tempo)) return;
    _raw_manual_tempo = norm_value;
    const auto clock_off_offset = 10;
    _manual_tempo = (kTempoMax - kTempoMin - clock_off_offset) * norm_value + kTempoMin - clock_off_offset;
    _tempo_mks = tempo_mks(_manual_tempo);
    if (external_clock()) {
        if (_is_running) {
            _is_running = false;
            _is_about_to_run = true;
        }
        else if (_is_about_to_run) {
            _is_running = true;
            _is_about_to_run = false;
        }
        reset();
    }
}

/*
Read external clock pin
*/
void Clock::pull(daisy::DaisySeed& hw) {
    auto new_state = g.Read();
    if (new_state && !_last_state) {
        external_clock_tick();
    }
    _last_state = new_state;
}

/*
External clock received
This method starts playback on first received clock
after playback was scheduled. After that, calls sync 
for every tick received.
*/
void Clock::external_clock_tick() {
    if (!external_clock()) return;
    if (!_is_running && !_is_about_to_run) return;

    if (_is_about_to_run) {
        _is_about_to_run = false;
        _is_running = true;
    }
    else {
        _resync = true;
        _hold = false;
        emit_ticks();
    }
}

void Clock::reset() {
    _fticks = 0;
    _ticks = 0;
    _ticks_at_last_clock = 0;
    _tempo_ticks = 0;
    _hold = false;
    _resync = false;
}
