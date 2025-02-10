#include "amius_adventure.hpp"
#include <c3d/maths.h>
#include "cameraMove.hpp"

using namespace AmiusAdventure::Scene;

Scene::Scene(Camera* camera) {
    std::fill(objects.begin(), objects.end(), std::nullopt);
    std::fill(uiObjects.begin(), uiObjects.end(), std::nullopt);
    this->ctx = SceneCtx {
        .deltaTime = std::chrono::milliseconds(),
        .tickStart = std::chrono::steady_clock::now(),
		.camera = camera,
        .animationTimer = 0
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

Object::Object(RenderData data, vec3 position = vec3{0, 0, 0}, vec3 rotation = vec3{0, 0, 0}, vec3 scale = vec3{1, 1, 1}, void(*tick)(Object*, SceneCtx*, Input::InputState*) = nullptr) : 
data(data), position{position[0], position[1], position[2]}, rotation{rotation[0], rotation[1], rotation[2]}, scale{scale[0], scale[1], scale[2]}, tick(tick) {}

AmiusAdventure::Scene::Handle* Object::getHandle() {
    if (handle == nullptr) {
        handle = new Handle {
            .valid = true,
            .data = &(*this)
        };
    }
    return handle;
}

void Object::setPosition(vec3 position) {
    this->position[0] = position[0];
    this->position[1] = position[1];
    this->position[2] = position[2];
    this->isDirty = true;
}

void Object::setRotation(quat rotation) {
    this->rotation[0] = rotation[0];
    this->rotation[1] = rotation[1];
    this->rotation[2] = rotation[2];
    this->rotation[3] = rotation[3];
    this->isDirty = true;
}

void Object::setScale(vec3 scale) {
    this->scale[0] = scale[0];
    this->scale[1] = scale[1];
    this->scale[2] = scale[2];
    this->isDirty = true;
}

C3D_Mtx Object::getTransform() {
    if (this->isDirty) {
        Mtx_Identity(&this->transform);
        Mtx_Scale(&this->transform, this->scale[0], this->scale[1], this->scale[2]);
        Mtx_RotateX(&this->transform, this->rotation[0], false);
        Mtx_RotateY(&this->transform, this->rotation[1], false);
        Mtx_RotateZ(&this->transform, this->rotation[2], false);
        Mtx_Translate(&this->transform, this->position[0], this->position[1], this->position[2], false);
        this->isDirty = false;
    }
    return this->transform;
}

bool Object::isVisible(Math::Frustum* frustum) {

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

UI::UIObject::UIObject(UIRenderData data, vec3 position = vec3{0, 0, 0}, float_t rotation = 0, vec2 scale = vec2{1, 1}, bool flip_vertical = false, bool flip_horizontal = false, void (*tick)(UIObject*, SceneCtx*, Input::InputState*) = nullptr) : 
data(data), position{position[0], position[1], position[2]}, rotation(rotation), scale{scale[0], scale[1]}, flip_vertical(flip_vertical), flip_horizontal(flip_horizontal), tick(tick) {}

UI::UIHandle* UI::UIObject::getHandle() {
    if (handle == nullptr) {
        handle = new UI::UIHandle {
            .valid = true,
            .data = &(*this)
        };
    }
    return handle;
}