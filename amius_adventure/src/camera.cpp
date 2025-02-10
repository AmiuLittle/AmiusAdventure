#include "amius_adventure.hpp"
#include <c3d/maths.h>

using namespace AmiusAdventure::Scene;

Camera::Camera(vec3 position, vec3 rotation, float zNear, float zFar, float fovY, float aspect, void(*tick)(Camera*, SceneCtx*, Input::InputState*)) : position{position[0], position[1], position[2]}, rotation{rotation[0], rotation[1], rotation[2]}, zNear(zNear), zFar(zFar), fovY(fovY), aspect(aspect), isDirty(true), tick(tick) {}

C3D_Mtx Camera::getTransform() {
    if (this->isDirty) {
        Mtx_Identity(&this->transform);
        Mtx_RotateX(&this->transform, this->rotation[0], false);
        Mtx_RotateY(&this->transform, this->rotation[1], false);
        Mtx_RotateZ(&this->transform, this->rotation[2], false);
        Mtx_Translate(&this->transform, this->position[0], this->position[1], this->position[2], false);
        this->isDirty = false;
    }
    return this->transform;
}

void getCameraFront(vec3 pos, vec3 rotation, vec3* out) {
    vec3 forward;
    forward[0] = -cos(rotation[0]) * sin(rotation[1]);
    forward[1] = -sin(rotation[0]);
    forward[2] = cos(rotation[0]) * cos(rotation[1]);


    // Normalize the result
    float length = sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    *out[0] = forward[0] / length;
    *out[1] = forward[1] / length;
    *out[2] = forward[2] / length;
}

/// @brief CALL THIS FUNCTION BEFORE `Camera::getTransform()`, it uses the `Camera::isDirty` variable to decide to calculate a new frustum
/// @return The camera frustum
AmiusAdventure::Math::Frustum Camera::generateFrustum() {
    if (this->isDirty) {
        const float halfVSide = zFar * tanf(fovY * .5f);
        const float halfHSide = halfVSide * aspect;
        vec3 forward;
        getCameraFront(this->position, this->rotation, &forward);
        vec3 frontMultFar;
        vec3_scale(frontMultFar, forward, zFar);
        vec3 frontMultNear;
        vec3_scale(frontMultNear, forward, zNear);

        vec3 result;
        vec3_add(result, frontMultNear, this->position);
        this->frustum.nearFace = {
            {result[0], result[1], result[2]},
            zNear
        };
        vec3_add(result, frontMultFar, this->position);
        this->frustum.farFace = {
            {result[0], result[1], result[2]},
            zFar
        };
    }
    return this->frustum;
}