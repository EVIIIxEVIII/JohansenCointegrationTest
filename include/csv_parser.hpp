#pragma once

#include <unordered_map>
#include <string>
#include <cstring>

#include "definitions.hpp"
#include "arena.hpp"

enum ColumnType {
    Type_Int32,
    Type_Int64,
    Type_uInt32,
    Type_uInt64,
    Type_Float32,
    Type_Float64,
};

#define COL_ASSERT(have, want) do {                       \
        if (have != (want)) abort();              \
    } while (0)

struct Column {
    ColumnType type;
    size_t     size;
    union {
        int32_t  *i32;
        int64_t  *i64;
        uint32_t *u32;
        uint64_t *u64;
        float    *f32;
        double   *f64;
        void     *raw;
    } data;

    void setData(void* ptr, ColumnType type) {
        switch (type) {
            case Type_Int32:   data.i32  = static_cast<int32_t*>(ptr); break;
            case Type_Int64:   data.i64  = static_cast<int64_t*>(ptr); break;
            case Type_uInt32:  data.u32  = static_cast<uint32_t*>(ptr); break;
            case Type_uInt64:  data.u64  = static_cast<uint64_t*>(ptr); break;
            case Type_Float32: data.f32  = static_cast<float*>(ptr); break;
            case Type_Float64: data.f64  = static_cast<double*>(ptr); break;
            default: data.raw = ptr;
        }
    }

    i32 *as_i32() { COL_ASSERT(type, Type_Int32);   return data.i32; }
    i64 *as_i64() { COL_ASSERT(type, Type_Int64);   return data.i64; }
    u32 *as_u32() { COL_ASSERT(type, Type_uInt32);  return data.u32; }
    u64 *as_u64() { COL_ASSERT(type, Type_uInt64);  return data.u64; }
    f32 *as_f32() { COL_ASSERT(type, Type_Float32); return data.f32; }
    f64 *as_f64() { COL_ASSERT(type, Type_Float64); return data.f64; }
};

class CsvParser {
public:
    CsvParser(
        const std::string& path,
        Arena& arena,
        std::unordered_map<std::string, ColumnType>& fields
    );

    std::unordered_map<std::string, Column> parsedContent() { return parsedContent_; }

private:
    static constexpr size_t BUF_SIZE   = 4096 * 10;
    static constexpr size_t MAX_HEADER = 1024;
    static constexpr size_t MAX_FIELD_SIZE = 64;
    static constexpr size_t MAX_FIELDS_NUM = 10;
    static constexpr size_t MAX_VALUE_SIZE = 32;

    void loadCsv(const std::string& path);

    inline static size_t typeSize(enum ColumnType t);
    inline static void typeCast(enum ColumnType t, const char* value, void* out);

    Arena& arena_;
    std::unordered_map<std::string, ColumnType>& fields_;
    std::unordered_map<std::string, Column> parsedContent_;
};
