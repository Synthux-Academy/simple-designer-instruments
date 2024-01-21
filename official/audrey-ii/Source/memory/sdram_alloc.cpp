#include "sdram_alloc.h"
#include <cstdint>
#include <type_traits>
#include "dev/sdram.h"

#ifndef INFS_SDRAM_POOL_SIZE
    // 32 MiB (half of SDRAM) by default
    #define INFS_SDRAM_POOL_SIZE (32 * 1024 * 1024) 
#endif

namespace infrasonic
{
    std::aligned_storage<INFS_SDRAM_POOL_SIZE>::type DSY_SDRAM_BSS sdram_pool;
}

using namespace infrasonic;

// Implementation here inspired by Eurorack Blocks MonotonicMemoryPool but
// combined into one single, far less flexible interface
// https://github.com/ohmtech-rdi/eurorack-blocks/blob/main/include/erb/detail/MonotonicMemoryPool.hpp 

void* SDRAM::allocate_raw(size_t size, size_t alignment)
{
    // After aligning the pool offset, we will need at most
    // `size + (alignment -1)` bytes to have been allocated
    // in the pool.
    const size_t align_mask = (alignment - 1);
    const size_t max_size = size + align_mask;

    // Increment pool position by our max possible needed size
    const size_t pos = pool_pos_.fetch_add (max_size, std::memory_order_relaxed);

    if (pos + max_size > INFS_SDRAM_POOL_SIZE) {
        asm ("bkpt 255");
        return nullptr;
    }

    // Align the previous pool position (before new allocation)
    // to the requested alignment
    const size_t align_pos = (pos + align_mask) & ~align_mask;

    // Cast and return the pointer
    uint8_t *pool_ptr = static_cast<uint8_t*>(static_cast<void*>(&sdram_pool));
    return &pool_ptr[align_pos];
}