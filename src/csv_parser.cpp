#include "csv_parser.hpp"
#include "profiling/profiler.hpp"
#include "fast_cast.hpp"

#include <stdio.h>
#include <sys/stat.h>
#include <array>

size_t CsvParser::typeSize(enum ColumnType t) {
    switch (t) {
        case Type_Int32:   return sizeof(int32_t);
        case Type_Int64:   return sizeof(int64_t);
        case Type_uInt32:  return sizeof(uint32_t);
        case Type_uInt64:  return sizeof(uint64_t);
        case Type_Float32: return sizeof(float);
        case Type_Float64: return sizeof(double);
        default: return 0;
    }
}

void CsvParser::typeCast(ColumnType t, const char* value, void* out) {
    bool ok = false;

    switch (t) {
        case Type_Int32:
            ok = fastcast::parse_int(value, *reinterpret_cast<int32_t*>(out));
            break;
        case Type_Int64:
            ok = fastcast::parse_int(value, *reinterpret_cast<int64_t*>(out));
            break;
        case Type_uInt32:
            ok = fastcast::parse_int(value, *reinterpret_cast<uint32_t*>(out));
            break;
        case Type_uInt64:
            ok = fastcast::parse_int(value, *reinterpret_cast<uint64_t*>(out));
            break;
        case Type_Float32:
            ok = fastcast::parse_float(value, *reinterpret_cast<float*>(out));
            break;
        case Type_Float64:
            ok = fastcast::parse_float(value, *reinterpret_cast<double*>(out));
            break;
        default:
            fprintf(stderr, "Unknown ColumnType\n");
            return;
    }

    if (!ok) {
        fprintf(stderr, "Bad numeric token: \" %s \" \n", value);
    }
}


CsvParser::CsvParser(
    const std::string& path,
    Arena& arena,
    std::unordered_map<std::string, ColumnType>& fields
):
    arena_{arena},
    fields_{fields}
{
    loadCsv(path);
}


void CsvParser::loadCsv(const std::string& path) {
    FILE* file = fopen(path.data(), "rb");
    if (file == NULL) {
        throw std::runtime_error("File not found!");
    }

    char headerString[MAX_HEADER];
    if (!fgets(headerString, MAX_HEADER, file)) {
        fclose(file);
        throw std::runtime_error("Failed to read the header!");
    }

    std::array<std::array<char, MAX_FIELD_SIZE>, MAX_FIELDS_NUM> headers;
    std::array<int, MAX_FIELDS_NUM> headersOffsets{};
    std::array<char, MAX_FIELDS_NUM> header{};
    std::array<ColumnType, MAX_FIELDS_NUM> columnTypes{};

    bool hitNewline = false;
    size_t charIndex = 0;
    int headerIndex = 0;
    int headerSize = 0;
    int offset = 0;

    for (size_t i = 0; i < MAX_HEADER && !hitNewline; ++i) {
        char c = headerString[i];
        if (c == '\0') break;

        switch (c) {
            case '\r':
                break;

            case ',':
            case '\n':
                if(c == '\n') {
                    hitNewline = true;
                    headerSize = i + 1; // one for the new line
                }
                header[charIndex] = '\0';

                if (fields_.find(header.data()) != fields_.end()){
                    strcpy(headers[headerIndex].data(), header.data());

                    columnTypes[headerIndex] = static_cast<ColumnType>(fields_[header.data()]);
                    headersOffsets[headerIndex] = offset;
                    headerIndex++;
                }

                offset++;
                charIndex = 0;
                break;
            default:
                if (charIndex < MAX_FIELD_SIZE - 1) {
                    header[charIndex++] = c;
                }
                break;
        };

    }

    char buffer[BUF_SIZE];
    void* valuesStorages[MAX_FIELDS_NUM];
    size_t bytes_read = 0;
    size_t count      = 0;

    while ((bytes_read = fread(buffer, sizeof(char), BUF_SIZE, file)) > 0) {
        char* p = buffer;
        char* end = buffer + bytes_read;
        while ((p = (char*)memchr(p, '\n', end - p))) {
            ++count;
            ++p;
        }
    }

    for (int i = 0; i < headerIndex; ++i) {
        valuesStorages[i] = arena_.allocate(count * typeSize(columnTypes[i]));
    }

    fseek(file, headerSize, SEEK_SET);

    bytes_read = 0;
    offset = 0;
    charIndex = 0;

    char value[MAX_VALUE_SIZE];
    u32 rowNumber   = 0;
    int valueIndex  = 0;
    bool copyActive = false;
    int nextTarget  = headersOffsets[0];

    while ((bytes_read = fread(buffer, sizeof(char), BUF_SIZE, file)) > 0) {
        for (size_t i = 0; i < bytes_read; ++i) {
            char c = buffer[i];

            switch(c) {
                case ',':
                case '\n': {
                    if (copyActive) {
                        value[charIndex] = '\0';
                        void* dst = static_cast<char*>(valuesStorages[valueIndex]) +
                                     rowNumber * typeSize(columnTypes[valueIndex]);
                        typeCast(columnTypes[valueIndex], value, dst);
                        ++valueIndex;

                        nextTarget = (valueIndex < headerIndex)
                                     ? headersOffsets[valueIndex]
                                     : -1;
                        copyActive = false;
                    }

                    ++offset;
                    charIndex = 0;

                    if (c == '\n') {
                        valueIndex = 0;
                        offset     = 0;
                        nextTarget = headersOffsets[0];
                        ++rowNumber;
                    }
                    break;
                }

                default:
                    value[charIndex++] = c;
                    break;
            }

            if (!copyActive && offset == nextTarget)
                copyActive = true;
        }
    }

    fclose(file);

    for (int i = 0; i < headerIndex; ++i) {
        Column col = { columnTypes[i], rowNumber, {nullptr} };
        col.setData(valuesStorages[i], columnTypes[i]);

        parsedContent_[std::string(headers[i].data())] = col;
    }
}



