#include "amius_adventure.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "cameraMove.hpp"

using namespace AmiusAdventure::Scene;

Scene::Scene(Camera* camera, AudioInterface* audio) {
    std::fill(objects.begin(), objects.end(), std::nullopt);
    std::fill(uiObjects.begin(), uiObjects.end(), std::nullopt);
    this->ctx = SceneCtx {
        .deltaTime = std::chrono::milliseconds(),
        .tickStart = std::chrono::steady_clock::now(),
		.camera = camera,
        .animationTimer = 0,
        .audio = audio,
        .softPanic = nullptr,
        .assetProvider = nullptr
    };
}

Scene::~Scene() {
	delete this->ctx.camera;
}

void Scene::tick(Input::InputState* inputState) {
    this->ctx.deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->ctx.tickStart);
    this->ctx.tickStart = std::chrono::steady_clock::now();
    for (int i = 0; i < this->objects.size(); i++) {
        if (this->objects[i].has_value() && (*this->objects[i]).tick != nullptr) {
            (*this->objects[i]).tick(&(*this->objects[i]), &this->ctx, inputState);
        }
    }
    for (int i = 0; i < this->uiObjects.size(); i++) {
        if (this->uiObjects[i].has_value() && (*this->uiObjects[i]).tick != nullptr) {
            (*this->uiObjects[i]).tick(&(*this->uiObjects[i]), &this->ctx, inputState);
        }
    }
    if (this->ctx.camera->tick != nullptr) {
        this->ctx.camera->tick(this->ctx.camera, &this->ctx, inputState);
    }
    this->ctx.animationTimer += this->ctx.deltaTime.count();
}

Object::Object() : data(RenderData {
    .type = RENDER_CUBE,
    .model = "none",
    .texture = "none",
    .dimension = {1, 1, 1},
}), position({0, 0, 0}), rotation({0, 0, 0}), scale({1, 1, 1}), tick(nullptr) {}

Object::~Object() {
    if (this->handle != nullptr) this->handle->valid = false;
}

Object::Object(RenderData data, glm::vec3 position = glm::vec3{0, 0, 0}, glm::vec3 rotation = glm::vec3{0, 0, 0}, glm::vec3 scale = glm::vec3{1, 1, 1}, void(*tick)(Object*, SceneCtx*, Input::InputState*) = nullptr) : 
data(data), position{position.x, position.y, position.z}, rotation{rotation.x, rotation.y, rotation.z}, scale{scale.x, scale.y, scale.z}, tick(tick) {}

AmiusAdventure::Scene::Handle* Object::getHandle() {
    if (handle == nullptr) {
        handle = new Handle {
            .valid = true,
            .data = &(*this)
        };
    }
    return handle;
}

void Object::setPosition(glm::vec3 position) {
    this->position.x = position.x;
    this->position.y = position.y;
    this->position.z = position.z;
    this->isDirty = true;
}

void Object::setRotation(glm::vec3 rotation) {
    this->rotation.r = rotation.x;
    this->rotation.g = rotation.y;
    this->rotation.b = rotation.z;
    this->isDirty = true;
}

void Object::setScale(glm::vec3 scale) {
    this->scale.x = scale.x;
    this->scale.y = scale.y;
    this->scale.z = scale.z;
    this->isDirty = true;
}

glm::mat4x4* Object::getTransform() {
    if (this->isDirty) {
        this->transform = glm::mat4x4(1.0f);
        this->transform = glm::scale(this->transform, this->scale);
        this->transform = this->transform * glm::mat4_cast(glm::quat(this->rotation));
        this->transform = glm::translate(this->transform, this->position);
        this->isDirty = false;
    }
    return &this->transform;
}

bool Object::isVisible(Math::Frustum* frustum) {
    return true;
}
 
UI::UIObject::UIObject() : data(UI::UIRenderData {
    .type = UI::RENDER_TEXT,
    .text = "Sample Text",
    .dimension = {1, 0},
    .basecolor = 0xFFFFFFFF,
    .align = UI::ALIGN_LEFT
}), position({0, 0, 0}), rotation(0), scale({1, 1}), flip_horizontal(false), flip_vertical(false), handle(nullptr), tick(nullptr) {}

UI::UIObject::~UIObject() {
    if (this->handle != nullptr) this->handle->valid = false;
}

UI::UIObject::UIObject(UIRenderData data, glm::vec3 position = glm::vec3{0, 0, 0}, float_t rotation = 0, glm::vec2 scale = glm::vec2{1, 1}, bool flip_vertical = false, bool flip_horizontal = false, void (*tick)(UIObject*, SceneCtx*, Input::InputState*) = nullptr) : 
data(data), position{position.x, position.y, position.z}, rotation(rotation), scale{scale.x, scale.y}, flip_vertical(flip_vertical), flip_horizontal(flip_horizontal), tick(tick) {}

UI::UIHandle* UI::UIObject::getHandle() {
    if (handle == nullptr) {
        handle = new UI::UIHandle {
            .valid = true,
            .data = &(*this)
        };
    }
    return handle;
}