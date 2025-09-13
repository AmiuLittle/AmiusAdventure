#include "cameraMove.hpp"
#include <iostream>

using namespace AmiusAdventure;

#define ANY_DPAD_OR_CIRCLE_PAD_DIRECTION (Input::KEY_UP | Input::KEY_DOWN | Input::KEY_RIGHT | Input::KEY_LEFT)

// Negative-Z Forward
// Positive-Y Up
// Positive-X Right
void moveCamera(Scene::Camera* camera, Scene::SceneCtx* ctx, Input::InputState* inputState) {
    if (inputState->kHeld & ANY_DPAD_OR_CIRCLE_PAD_DIRECTION) {
        camera->isDirty = true;
        if (inputState->kHeld & Input::KEY_UP) {
            camera->position.y += 0.5 * ((float)ctx->deltaTime.count() / 1000000.0f);
        }
        if (inputState->kHeld & Input::KEY_LEFT) {
            camera->position.x -= 0.5 * ((float)ctx->deltaTime.count() / 1000000.0f);
        }
        if (inputState->kHeld & Input::KEY_DOWN) {
            camera->position.y -= 0.5 * ((float)ctx->deltaTime.count() / 1000000.0f);
        }
        if (inputState->kHeld & Input::KEY_RIGHT) {
            camera->position.x += 0.5 * ((float)ctx->deltaTime.count() / 1000000.0f);
        }
    }
}
