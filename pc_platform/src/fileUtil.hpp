#pragma once
#ifndef PC_FILE_UTIL
#include <vector>
#include <string>
#include <fstream>

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        setErr("Failed to open file: " + filename);
        return {};
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

#define PC_FILE_UTIL
#endif // PC_FILE_UTIL