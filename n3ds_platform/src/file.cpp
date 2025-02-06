#include "file.hpp"
#include <fstream>
#include <vector>
#include "error.hpp"

const void* readFile(std::string path, size_t* fileSize) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        setErr("Failed to open file");
        return nullptr;
    }

    *fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::vector<char> content(*fileSize);
    file.read(content.data(), *fileSize);

    auto buffer = new char[*fileSize];
    std::copy(content.begin(), content.end(), buffer);
    return (const void*)buffer;
}