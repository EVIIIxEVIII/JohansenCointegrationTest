#include <stdint.h>
#include <sys/time.h>
#include "profiling/timer.hpp"

typedef uint64_t u64;
typedef uint32_t u32;

u64 readOSTimer() {
    struct timeval value;
    gettimeofday(&value, 0);

    u64 result = 1e6 * (u64)value.tv_sec + (u64)value.tv_usec;
    return result;
}

u64 estimateCPUFreq(u64 ms) {
    u64 cpuStart  = readCPUTimer();
    u64 osStart   = readOSTimer();
    u64 osEnd     = 0;
    u64 osElapsed = 0;

    u64 osWaitTime = 1e6 * ms / 1000;

    while (osElapsed < osWaitTime) {
        osEnd = readOSTimer();
        osElapsed = osEnd - osStart;
    }

    u64 cpuEnd     = readCPUTimer();
    u64 cpuElapsed = cpuEnd - cpuStart;
    u64 cpuFreq    = 0;

    if (osElapsed) {
        cpuFreq = 1e6 * cpuElapsed / osElapsed;
    }

    return cpuFreq;
}

