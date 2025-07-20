#pragma once

#include "timer.hpp"
#include <cstdio>
#include <climits>

typedef double f64;

struct Tester {
    f64 tryForTime;
    u64 cpuFreq;

    u64 minTime = INT_MAX;
    u64 maxTime;

    u64 timeSinceLastUpdate;

    u64 totalTime;
    u64 totalCount;
};

inline void addTimeToTester(Tester& tester, u64 elapsed) {
    if (tester.minTime > elapsed) {
        tester.minTime = elapsed;
        tester.timeSinceLastUpdate = 0;
    } else {
        tester.timeSinceLastUpdate += elapsed;
    }

    if (tester.maxTime < elapsed) {
        tester.maxTime = elapsed;
    }

    tester.totalTime += elapsed;
    tester.totalCount++;
}

inline bool shouldTest(const Tester& tester) {
    return tester.tryForTime >= ((f64)tester.timeSinceLastUpdate / (f64)tester.cpuFreq);
}

inline f64 printThroughput(f64 time, u64 size) {
    constexpr f64 megabyte = 1024. * 1024.;
    constexpr f64 gigabyte = megabyte * 1024.;

    f64 bytesPerSecond = (f64)size / time;
    f64 megabytes = (f64)size / megabyte;
    f64 gigabytesPerSecond = bytesPerSecond / gigabyte;

    std::printf("   %.3fmb at %.2fgb/s \n", megabytes, gigabytesPerSecond);
    return gigabytesPerSecond;
}

inline void printResult(const Tester& tester, const char* functionName, u64 processedData) {
    std::printf("---- %s ----\n", functionName);

    f64 minTimeInSec = (f64) tester.minTime / (f64) tester.cpuFreq;
    f64 avgTimeInSec = ((f64) tester.totalTime / (f64) tester.totalCount) / (f64) tester.cpuFreq;
    f64 maxTimeInSec = (f64) tester.maxTime / (f64) tester.cpuFreq;

    std::printf("Min time: %fms", minTimeInSec * 1000.);
    printThroughput(minTimeInSec, processedData);
    std::printf("Avg time: %fms", avgTimeInSec * 1000.);
    printThroughput(avgTimeInSec, processedData);
    std::printf("Max time: %fms", maxTimeInSec * 1000.);
    printThroughput(maxTimeInSec, processedData);
}

