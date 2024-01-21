#pragma once

#include "i.source.h"

namespace blptls {
namespace spotykach {

class Source: public ISource {
public:
    Source();
    void set_frozen(bool) override;
    bool is_frozen() override { return _rec_env_pos_inc != 1; }

    void set_antifreeze(bool) override;
    
    void set_cycle_start(size_t) override;

    void initialize() override;
    
    size_t length() override { return _buffer_length; };

    void set_recording(bool is_on);

    void write(float, float) override;
    size_t read_head() override { return _read_head; };
    
    void read(float&, float&, size_t) override;
    
    void reset() override;
    
private:
    static constexpr size_t kFadeLength = 600;

    float* _buffer[2];
    size_t _buffer_length;
    
    size_t _write_head;
    size_t _read_head;
    size_t _sycle_start;
    size_t _rec_env_pos;
    int32_t _rec_env_pos_inc;

    bool _antifreeze;
};

}
}
