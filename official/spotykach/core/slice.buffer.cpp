#include <stdio.h>
#include <algorithm>
#include <cstring>
#include "slice.buffer.h"
#include "buffers.h"

using namespace blptls;
using namespace spotykach;

SliceBuffer::SliceBuffer(): _size { kSliceBufferLength } {
}

void SliceBuffer::initialize() {
    _buffer[0] = Buffers::pool().sliceBuffer();
    _buffer[1] = Buffers::pool().sliceBuffer();
    reset();
}

float SliceBuffer::read(int channel, uint32_t frame) {
    if (frame >= _writeHead) return 0;
    return _buffer[channel][frame];
}

void SliceBuffer::write(float in0, float in1) {
    _buffer[0][_writeHead] = in0;
    _buffer[1][_writeHead] = in1;
    _writeHead ++;
}

void SliceBuffer::rewind() {
    _writeHead = 0;
}

bool SliceBuffer::isFull() {
    return _writeHead == _size;
}

void SliceBuffer::reset() {
    rewind();
}
