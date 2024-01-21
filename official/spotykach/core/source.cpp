#include "source.h"
#include "buffers.h"

using namespace blptls;
using namespace spotykach;

Source::Source() :
    _buffer_length   { kSourceBufferLength },
    _antifreeze      { false },
    _write_head      { 0 },
    _read_head       { 0 },
    _sycle_start     { 0 },
    _rec_env_pos     { 0 },
    _rec_env_pos_inc { 0 }
    {}

void Source::set_frozen(bool frozen) { 
    _rec_env_pos_inc = frozen ? -1 : 1;
}

void Source::set_antifreeze(bool value) {
    _antifreeze = value;
    if (_antifreeze) set_frozen(false);
}

void Source::set_cycle_start(size_t start) {
    if (start >= _buffer_length) return;
    _write_head = start;
    _sycle_start = start;
}

void Source::initialize() {
    _buffer[0] = Buffers::pool().sourceBuffer();
    _buffer[1] = Buffers::pool().sourceBuffer();
    reset();
}

void Source::read(float& out0, float& out1, size_t frame) {
    frame %= _buffer_length;
    out0 = _buffer[0][frame];
    out1 = _buffer[1][frame];
}

void Source::write(float in0, float in1) {
      if (_rec_env_pos_inc > 0 && _rec_env_pos < kFadeLength
       || _rec_env_pos_inc < 0 && _rec_env_pos > 0) {
          _rec_env_pos += _rec_env_pos_inc;
      }

      if (_rec_env_pos > 0) {
        float rec_attenuation = static_cast<float>(_rec_env_pos) / static_cast<float>(kFadeLength);
        _buffer[0][_write_head] = in0 * rec_attenuation + _buffer[0][_write_head] * (1.f - rec_attenuation);
        _buffer[1][_write_head] = in1 * rec_attenuation + _buffer[1][_write_head] * (1.f - rec_attenuation);
        _read_head = _write_head;
        if (++_write_head >= _buffer_length) _write_head = 0;
      }
}

void Source::reset() {
    memset(_buffer[0], 0, _buffer_length * sizeof(float));
    memset(_buffer[1], 0, _buffer_length * sizeof(float));
    _write_head = 0;
    _read_head = 0;
    _sycle_start = 0;
}
