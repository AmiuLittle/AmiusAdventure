#include "rotateCube.hpp"
#include "amius_adventure.hpp"
#include "linmath.h"

void rotateCubeTick(AmiusAdventure::Scene::Object* obj, AmiusAdventure::Scene::SceneCtx* ctx) {
    obj->rotation[0] += 0.5 * (ctx->deltaTime.count() / 1000.0f);
    obj->rotation[1] += 0.5 * (ctx->deltaTime.count() / 1000.0f);
    obj->isDirty = true;
}