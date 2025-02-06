#include "fpsCounter.hpp"
#include "amius_adventure.hpp"
#include <chrono>

using namespace AmiusAdventure::Scene;

void fpsCounterTick(UI::UIObject* obj, SceneCtx* ctx) {
    obj->data.text = "Delta Time: " + std::to_string(ctx->deltaTime.count()) + "ms";
    if (ctx->deltaTime.count() > 33) obj->data.basecolor = 0xFF0000FF; else obj->data.basecolor = 0xFFFFFFFF;
}
