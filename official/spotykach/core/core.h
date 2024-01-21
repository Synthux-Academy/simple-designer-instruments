#pragma once

#include <array>
#include <vector>
#include <memory>
#include "engine.h"
#include "globals.h"
#include "../control/clockable.h"

namespace blptls {
namespace spotykach {

struct PlaybackControls {
    bool ctns_a;
    bool ctns_b;
    bool rev_a;
    bool rev_b;
};

class Core: public Clockable {
public:
    Core();
    ~Core() = default;
    
    Engine& engineAt(int index) const;
    long enginesCount() const;

    void tick();

    void setMutex(bool mutex);
    
    void setSplit(bool value);
    
    void setVolumeBalance(float value);

    void set_pattern_balance(float value);

    void setCascade(bool value);
    
    void setJitterRate(float normVal);

    void set_playback_controls(PlaybackControls c);

    void initialize() const;
    void preprocess(PlaybackParameters p) const;
    void process(const float* const* inBuf, float** outBuf, int numFrames) const;
    
private:
    std::array<std::shared_ptr<Engine>, kEnginesCount> _engines;
    
    float _vol[kEnginesCount];
    float pattern_balance_;
    bool _split;
    bool _mutex;
    bool _cascade;
    PlaybackControls _p_ctrls;
    std::vector<std::shared_ptr<void>> _releasePool;
};
}
}
