#pragma once
#ifndef PC_SHAPES
#include "renderPrimitives.hpp"
#include <vector>

const std::vector<Vertex> rectangleVerticies = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}
};


const std::vector<uint16_t> rectangleIndices = {
    0, 1, 2, 2, 3, 0
};

#define PC_SHAPES
#endif // PC_SHAPES