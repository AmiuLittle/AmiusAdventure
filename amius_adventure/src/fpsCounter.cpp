#include "fpsCounter.hpp"
#include "amius_adventure.hpp"
#include <chrono>

using namespace AmiusAdventure::Scene;

void fpsCounterTick(UI::UIObject* obj, SceneCtx* ctx, AmiusAdventure::Input::InputState* inputState) {
    obj->data.text = "Delta Time: " + std::to_string((int)(ctx->deltaTime.count() / 1000.0f)) + "ms";
    if (ctx->deltaTime.count() / 1000.0f > 34) obj->data.basecolor = 0xFF0000FF; else obj->data.basecolor = 0xFFFFFFFF;
}
