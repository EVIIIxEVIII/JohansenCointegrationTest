#include "arena.hpp"

#include <stdexcept>
#include <memory.h>

Arena::Arena(size_t size): size_(size) {
    constexpr size_t page_size = 4096;
    start_ = (void*) std::aligned_alloc(64, size * sizeof(char));

    char* start = (char*) start_;
    for (size_t i = 0; i < size; i += page_size) {
        start[i] = 0;
    }

    if (start_ == NULL) {
        throw std::runtime_error("Failed to allocate memory for the arena");
    }
}


void* Arena::allocate(size_t size) {
    if (offset_ + size > size_) {
        throw std::runtime_error("ERROR: Memory arena failed to allocate memory. Requested size is too large.");
        return NULL;
    }

    void* ptr = (char*)start_ + offset_;
    offset_ += size;

    return ptr;
}


char* Arena::allocate(const char* str) {
    size_t size = strlen(str) + 1;
    if (offset_ + size > size_) {
        throw std::runtime_error("ERROR: Memory arena failed to allocate memory. Requested size is too large.");
        return NULL;
    }

    char* ptr = (char*)start_ + offset_;
    memcpy(ptr, str, size);

    offset_ += size;

    return ptr;
}
