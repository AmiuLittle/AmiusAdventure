#include "amius_adventure.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "cameraMove.hpp"
#include "error.hpp"

using namespace AmiusAdventure::Scene;

Scene::Scene(Camera* camera, AudioInterface* audio) {
    root = Object::Create();
    std::fill(uiObjects.begin(), uiObjects.end(), nullptr);
    this->ctx = SceneCtx {
        .deltaTime = std::chrono::microseconds(),
        .tickStart = std::chrono::high_resolution_clock::now(),
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
    this->ctx.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - this->ctx.tickStart);
    this->ctx.tickStart = std::chrono::high_resolution_clock::now();
    this->root->tickAll(&this->ctx, inputState);
    for (int i = 0; i < this->uiObjects.size(); i++) {
        if (this->uiObjects[i].get() != nullptr && (*this->uiObjects[i]).tick != nullptr) {
            (*this->uiObjects[i]).tick(&(*this->uiObjects[i]), &this->ctx, inputState);
        }
    }
    if (this->ctx.camera->tick != nullptr) {
        this->ctx.camera->tick(this->ctx.camera, &this->ctx, inputState);
    }
    this->ctx.animationTimer += this->ctx.deltaTime.count();
}

Object::Object() : data(RenderData()), position({0, 0, 0}), rotation({0, 0, 0}), scale({1, 1, 1}), tick(nullptr), isDirty(true) {}

Object::~Object() {
    
}

Object::Object(RenderData data, glm::vec3 position = glm::vec3{0, 0, 0}, glm::vec3 rotation = glm::vec3{0, 0, 0}, glm::vec3 scale = glm::vec3{1, 1, 1}, void(*tick)(Object*, SceneCtx*, Input::InputState*) = nullptr) : 
data(data), position{position.x, position.y, position.z}, rotation{rotation.x, rotation.y, rotation.z}, scale{scale.x, scale.y, scale.z}, tick(tick), isDirty(true) {}

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

glm::mat4x4 Object::getTransform() {
    if (this->isDirty) {
        if (parent.lock() == nullptr) {
            this->transform = glm::mat4x4(1.0f);
        }
        else {
            this->transform = parent.lock()->getTransform();
        }
        this->transform = glm::translate(this->transform, this->position);
        this->transform = glm::rotate(this->transform, this->rotation.x, glm::vec3(1.0, 0.0, 0.0));
        this->transform = glm::rotate(this->transform, this->rotation.y, glm::vec3(0.0, 1.0, 0.0));
        this->transform = glm::rotate(this->transform, this->rotation.z, glm::vec3(0.0, 0.0, 1.0));
        this->transform = glm::scale(this->transform, this->scale);
        this->isDirty = false;
    }
    return this->transform;
}

bool Object::isVisible(Math::Frustum* frustum) {
    return true;
}

std::shared_ptr<Object> Object::Create() {
    auto output = std::make_shared<Object>();
    output->self = output;
    return output;
}

std::shared_ptr<Object> Object::Create(RenderData data, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, void(*tick)(Object*, SceneCtx*, Input::InputState*)) {
    auto output = std::make_shared<Object>(data, pos, rot, scale, tick);
    output->self = output;
    return output;
}

bool Object::addChild(std::shared_ptr<Object> object) {
    if (object.get() != nullptr) {
        if (object->parent.lock() == nullptr) {
            object->parent = this->self;
            this->children.push_back(object);
        }
        else {
            setErr("Cannot add child that already has a parent");
            return false;
        }
    }
    else {
        setErr("Cannot add child that is null");
        return false;
    }
    return true;
}

void Object::markDirty() {
    this->isDirty = true;
    for (int i = 0; i < this->children.size(); i++) {
        if (this->children[i] != nullptr) {
            this->children[i]->markDirty();
        }
    }
}

void Object::tickAll(SceneCtx* ctx, AmiusAdventure::Input::InputState* inputState) {
    if (this->tick != nullptr) {
        this->tick(&(*this), ctx, inputState);
    }
    for (int i = 0; i < this->children.size(); i++) {
        if (this->children[i] != nullptr) {
            this->children[i]->tickAll(ctx, inputState);
        }
    }
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