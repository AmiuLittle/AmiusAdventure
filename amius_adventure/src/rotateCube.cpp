#include "rotateCube.hpp"
#include "amius_adventure.hpp"

void rotateCubeTick(AmiusAdventure::Scene::Object* obj, AmiusAdventure::Scene::SceneCtx* ctx, AmiusAdventure::Input::InputState* inputState) {
    obj->rotation.x += 0.5 * (ctx->deltaTime.count() / 1000000.0f);
    obj->rotation.y += 0.5 * (ctx->deltaTime.count() / 1000000.0f);
    obj->markDirty();
}

void rotateCubeTickOneAxis(AmiusAdventure::Scene::Object* obj, AmiusAdventure::Scene::SceneCtx* ctx, AmiusAdventure::Input::InputState* inputState) {
    obj->rotation.y += 0.5 * (ctx->deltaTime.count() / 1000000.0f);
    obj->markDirty();
}