#pragma once

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;

u64 readOSTimer();

inline u64 readCPUTimer() {
    return __builtin_ia32_rdtsc();
}

u64 estimateCPUFreq(u64 ms);

