#pragma once
#include <charconv>
#include <fast_float/fast_float.h>

namespace fastcast {

template<typename Int>
inline bool parse_int(const char* s, Int& out) {
    const char* end = s + 24;
    auto res = std::from_chars(s, end, out, 10);
    return res.ec == std::errc();
}

template<typename Float>
inline bool parse_float(const char* s, Float& out)
{
    auto res = fast_float::from_chars(s, s + 31, out);
    return res.ec == std::errc();
}


}
