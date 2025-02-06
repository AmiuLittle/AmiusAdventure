#include "amius_adventure.hpp"
#include "linmath.h"
#include "object_macros.hpp"
#include "fpsCounter.hpp"
#include "rotateCube.hpp"

using namespace AmiusAdventure;

Engine::Engine(std::string platform, void(*softPanic)(std::string), Scene::Scene* topScene, Scene::Scene* bottomScene) : platform(platform), softPanic(softPanic) {
    topScene->uiObjects[0] = NEW_UI_TEXT("Platform: " + platform, 1, 0xFFFFFFFF, Scene::UI::ALIGN_LEFT, 0, 0, 0, 0.0F, .5, .5, false, false, nullptr);
    topScene->uiObjects[1] = NEW_UI_TEXT("Delta Time: 0ms", 1, 0xFFFFFFFF, Scene::UI::ALIGN_LEFT, 0, .06, 0, 0, 0.5, 0.5, false, false, fpsCounterTick);

    topScene->objects[0] = NEW_CUBE("romfs:/gfx/commonTex.t3x", 1, 1, 1, 0.25, 0, -3.0, 0, 0, 0, 1, 1, 1, rotateCubeTick);
    topScene->objects[1] = NEW_MODEL("romfs:/models/3ds_test_model.glb", "romfs:/gfx/3ds_test_model.t3x", -0.25, 0, -3.0, 0, 0, 0, 0.5, 0.5, 0.5, rotateCubeTick);

    bottomScene->uiObjects[0] = NEW_UI_TEXT("Press Start to Exit", 1, 0xFFFFFFFF, Scene::UI::ALIGN_CENTER, .5, .92, 0, 0, .5, .5, false, false, nullptr);
}

bool Engine::update(Input::InputState inputState, Scene::Scene* topScene, Scene::Scene* bottomScene) { // returns true if app should end
    if (inputState.kDown & Input::KEY_START) {
        return true;
    }
    if (inputState.kDown & Input::KEY_SELECT) {
        softPanic("User initiated test");
    }

    topScene->tick();
    bottomScene->tick();

    return false;
}