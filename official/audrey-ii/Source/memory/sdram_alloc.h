#pragma once
#ifndef INFS_SDRAM_ALLOC_H
#define INFS_SDRAM_ALLOC_H

#include <stddef.h>
#include <atomic>
#include <new>

namespace infrasonic
{
    /**
     * @brief
     * Provides an interface for monotonic, pseudo-dynamic allocation of memory
     * into Daisy SDRAM.
     *
     * @details
     * By default statically reserves 32MB of storage in SDRAM for runtime allocation.
     * Due to Daisy's STM32 lacking an MMU, please note that "dynamic" allocations
     * made this way will never actually be reclaimed during program lifetime.
     *
     * @warning
     * This only works when built on Daisy platform and does not (yet?)
     * handle allocations beyond the max available memory with anything more
     * than a breakpoint.
     *
     */
    class SDRAM {

        public:
            template<typename T, class... Args>
            static T* allocate(Args &&... args)
            {
                auto ptr = instance().allocate_raw(sizeof(T), alignof(T));
                return new (ptr) T(std::forward<Args>(args)...);
            }

            // NOTE: UNTESTED
            template<typename T>
            static T* allocate_buf(size_t size)
            {
                auto ptr = instance().allocate_raw(size * sizeof(T), alignof(T));
                return reinterpret_cast<T*>(ptr);
            }

        private:
            SDRAM() = default;
            ~SDRAM() = default;

            static SDRAM& instance() {
                static SDRAM sdram;
                return sdram;
            }

            void* allocate_raw(size_t size, size_t alignment);

            std::atomic_size_t pool_pos_ = {0};
    };

} // namespace infrasonic

#endif
