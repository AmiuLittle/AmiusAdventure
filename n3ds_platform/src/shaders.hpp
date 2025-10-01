#pragma once
#ifndef N3DS_PLATFORM_SHADERS
#include <utility>
#include <string>
#include <stdint.h>
#include "missing_shbin.h"
#include "simple_textured_shbin.h"

// TODO: implement autogen for this function
std::pair<const uint8_t*, size_t> getShader(std::string name) {
    if (name.compare("missing") == 0) {
        return std::pair(missing_shbin, missing_shbin_size);
    }
    if (name.compare("simple_textured") == 0) {
        return std::pair(simple_textured_shbin, simple_textured_shbin_size);
    }
}

#define N3DS_PLATFORM_SHADERS
#endif