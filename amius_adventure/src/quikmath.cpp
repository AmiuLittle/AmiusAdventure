#include "quikmath.hpp"

float DegreesToRadians(float degrees) {
    return degrees * (M_PI / 180.0f);
}

glm::mat4x4 reverseRows(const glm::mat4x4& mat) {
    return glm::mat4x4(
        mat[3],
        mat[2],
        mat[1],
        mat[0]
    );
}