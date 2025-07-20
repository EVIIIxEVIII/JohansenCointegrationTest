#pragma once

#include <unordered_map>
#include <string>
#include "arena.hpp"


class CsvParser {
public:
    CsvParser(const std::string& path, Arena& arena);


private:
    void readFile(std::string& path);

    std::unordered_map<std::string, void*> parsedContent;
    const size_t LINE_BUFFER = 512;
};
