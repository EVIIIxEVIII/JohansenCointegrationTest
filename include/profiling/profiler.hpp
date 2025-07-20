#pragma once

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef double f64;


struct Anchor {
    const char* label;
    u64 hitCount;
    u64 processedData;
    u64 tscElapsedExclusive;
    u64 tscElapsedInclusive;
};

#ifndef PROFILE
#define PROFILE 1
#endif

#if PROFILE
struct Profiler {
    Anchor anchors[4096];
    u64 startTSC;
    u64 endTSC;
};
#else
struct Profiler {
    Anchor anchors[1];
    u64 startTSC;
    u64 endTSC;
};
#endif

extern Profiler globalProfiler;
extern u32 globalProfilerParent;

#if PROFILE

struct TimeScope {
    TimeScope(const char* label_, u32 anchorIndex_, u64 processedData_);
    ~TimeScope();

    const char *label;
    u64 processedData;
    u64 oldTSCElapsedInclusive;
    u64 startTSC;
    u32 parentIndex;
    u64 anchorIndex;
};

#define TimeThroughput(name, size) TimeScope NameConcat(Block, __LINE__)(name, __COUNTER__ + 1, size)
#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(name) TimeThroughput(name, 0)
#define TimeFunction() TimeBlock(__func__)

#else

#define TimeThroughput(...)
#define TimeBlock(...)
#define TimeFunction(...)

#endif

void startProfiling();
void endProfilingAndPrint();

