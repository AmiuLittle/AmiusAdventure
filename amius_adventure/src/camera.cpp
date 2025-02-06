#include "amius_adventure.hpp"
#include <c3d/maths.h>

using namespace AmiusAdventure::Scene;

Camera::Camera(vec3 position, vec3 rotation, void(*tick)(Camera*, SceneCtx*, AmiusAdventure::Input::InputState*) = nullptr) : position{position[0], position[1], position[2]}, rotation{rotation[0], rotation[1], rotation[2]}, tick(tick), isDirty(true) {}

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