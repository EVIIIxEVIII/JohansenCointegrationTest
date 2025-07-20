#include "profiling/profiler.hpp"
#include "profiling/timer.hpp"
#include <stdio.h>

#ifndef PROFILE
#define PROFILE 1
#endif

#if PROFILE
inline u32 globalProfilerParent = 0;
inline Profiler globalProfiler;

TimeScope::TimeScope(const char* label_, u32 anchorIndex_, u64 processedData_)
    : label(label_), processedData(processedData_), anchorIndex(anchorIndex_) {

    parentIndex = globalProfilerParent;
    Anchor *anchor = globalProfiler.anchors + anchorIndex;
    oldTSCElapsedInclusive = anchor->tscElapsedInclusive;
    anchor->processedData += processedData;

    globalProfilerParent = anchorIndex;
    startTSC = readCPUTimer();
}

TimeScope::~TimeScope() {
    u64 elapsed = readCPUTimer() - startTSC;
    globalProfilerParent = parentIndex;

    Anchor *parent = globalProfiler.anchors + parentIndex;
    Anchor *anchor = globalProfiler.anchors + anchorIndex;

    parent->tscElapsedExclusive -= elapsed;
    anchor->tscElapsedExclusive += elapsed;
    anchor->tscElapsedInclusive = oldTSCElapsedInclusive + elapsed;
    ++anchor->hitCount;

    anchor->label = label;
}
#else
Profiler globalProfiler;
#endif

static void printTimeElapsed(u64 totalTSCElapsed, Anchor *anchor, u64 cpuFreq) {
    f64 percent = 100.0 * ((f64)anchor->tscElapsedExclusive / (f64)totalTSCElapsed);
    printf("   %s[%lu]: %lu (%fms) (%.2f%%",
           anchor->label, anchor->hitCount,
           anchor->tscElapsedExclusive,
           (f64)anchor->tscElapsedExclusive / cpuFreq * 1000.,
           percent
    );

    if(anchor->tscElapsedInclusive != anchor->tscElapsedExclusive) {
        f64 percentWithChildren = 100.0 * ((f64)anchor->tscElapsedInclusive / (f64)totalTSCElapsed);
        printf(", %.2f%% w/children", percentWithChildren);
    }

    if(anchor->processedData) {
        f64 megabyte = 1024. * 1024.;
        f64 gigabyte = megabyte * 1024.;

        f64 seconds = (f64)anchor->tscElapsedInclusive / (f64)cpuFreq;
        f64 bytesPerSecond = (f64)anchor->processedData / seconds;
        f64 megabytes = (f64)anchor->processedData / (f64) megabyte;
        f64 gigabytesPerSecond = bytesPerSecond / gigabyte;

        printf("   %.3fmb at %.2fgb/s", megabytes, gigabytesPerSecond);
    }

    printf(")\n");
}

void startProfiling() {
    globalProfiler.startTSC = readCPUTimer();
}

void endProfilingAndPrint() {
    globalProfiler.endTSC = readCPUTimer();
    u64 cpuFreq = estimateCPUFreq(1000);
    u64 totalCPUElapsed = globalProfiler.endTSC - globalProfiler.startTSC;

    if(cpuFreq) {
        printf("\n Total time: %0.fms (CPU freq %lu)\n", 1000.0 * (f64)totalCPUElapsed / (f64)cpuFreq, cpuFreq);
    }

    for (u32 index = 0; index < (sizeof(globalProfiler.anchors) / sizeof(globalProfiler.anchors[0])); ++index) {
        Anchor *anchor = globalProfiler.anchors + index;
        if (anchor->tscElapsedInclusive) {
            printTimeElapsed(totalCPUElapsed, anchor, cpuFreq);
        }
    }
}

