#pragma once

#include <stdlib.h>
#include <stdint.h>

struct Arena {
    void* start_   = 0;
    size_t size_   = 0;
    size_t offset_ = 0;

    Arena(size_t size);

    void* allocate(size_t size);
    char* allocate(const char* str);

    void reset() { offset_ = 0; }
    ~Arena() { free(start_); }
};


