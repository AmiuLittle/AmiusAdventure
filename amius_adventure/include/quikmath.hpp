#pragma once
#ifndef AMIUS_ADVENTURE_QUIK_MATH
#include <cmath>
#include <glm/glm.hpp>

float DegreesToRadians(float degrees);

glm::mat4x4 reverseRows(const glm::mat4x4&);

#define AMIUS_ADVENTURE_QUIK_MATH
#endif