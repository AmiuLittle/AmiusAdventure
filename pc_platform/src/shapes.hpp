#pragma once
#ifndef PC_SHAPES
#include "renderPrimitives.hpp"
#include <vector>

const std::vector<Vertex> triangleVerticies = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

#define PC_SHAPES
#endif // PC_SHAPES