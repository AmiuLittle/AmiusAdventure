#include "amius_adventure.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace AmiusAdventure::Scene;

Camera::Camera(glm::vec3 position, glm::vec3 rotation, float zNear, float zFar, float fovY, float aspect, void(*tick)(Camera*, SceneCtx*, Input::InputState*)) : position{position.x, position.y, position.z}, rotation{rotation.x, rotation.y, rotation.z}, zNear(zNear), zFar(zFar), fovY(fovY), aspect(aspect), isDirty(true), tick(tick) {}

glm::mat4x4 Camera::getTransform() {
    if (this->isDirty) {
        this->transform = glm::mat4x4(1.0f);
        this->transform = glm::translate(this->transform, this->position);
        this->transform = glm::rotate(this->transform, this->rotation.x, glm::vec3(1.0, 0.0, 0.0));
        this->transform = glm::rotate(this->transform, this->rotation.y, glm::vec3(0.0, 1.0, 0.0));
        this->transform = glm::rotate(this->transform, this->rotation.z, glm::vec3(0.0, 0.0, 1.0));
        this->transform = glm::inverse(this->transform);
        this->isDirty = false;
    }
    return this->transform;
}

void getCameraFront(glm::vec3 pos, glm::vec3 rotation, glm::vec3* out) {
    glm::vec3 forward;
    forward.x = -cos(rotation.x) * sin(rotation.y);
    forward.y = -sin(rotation.x);
    forward.z = cos(rotation.x) * cos(rotation.y);


    // Normalize the result
    float length = sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    out->x = forward.x / length;
    out->y = forward.y / length;
    out->z = forward.z / length;
}

/// @brief CALL THIS FUNCTION BEFORE `Camera::getTransform()`, it uses the `Camera::isDirty` variable to decide to calculate a new frustum
/// @return The camera frustum
AmiusAdventure::Math::Frustum Camera::generateFrustum() {
    if (this->isDirty) {
        const float halfVSide = zFar * tanf(fovY * .5f);
        const float halfHSide = halfVSide * aspect;
        glm::vec3 forward;
        getCameraFront(this->position, this->rotation, &forward);
        glm::vec3 frontMultFar = forward * zFar;
        glm::vec3 frontMultNear = forward * zNear;

        glm::vec3 result = frontMultNear + this->position;
        this->frustum.nearFace = {
            {result.x, result.y, result.z},
            zNear
        };
        glm::vec3 result2 = frontMultFar + this->position;
        this->frustum.farFace = {
            {result2.x, result2.y, result2.z},
            zFar
        };
    }
    return this->frustum;
}