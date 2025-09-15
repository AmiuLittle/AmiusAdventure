#include "amius_adventure.hpp"
#include "object_macros.hpp"
#include "fpsCounter.hpp"
#include "rotateCube.hpp"
#include "cameraMove.hpp"
#include "frontPlayer.hpp"
#include "playMusic.hpp"

using namespace AmiusAdventure;

Engine::Engine(std::string platform, void(*softPanic)(std::string), Scene::Scene* topScene, Scene::Scene* bottomScene, AssetProviderInterface* assetProvider): platform(platform), softPanic(softPanic), assetProvider(assetProvider) {
    topScene->ctx.softPanic = softPanic;
    bottomScene->ctx.softPanic = softPanic;
    topScene->ctx.assetProvider = assetProvider;
    bottomScene->ctx.assetProvider = assetProvider;

    topScene->ctx.camera->position.z = 1;
    topScene->ctx.camera->tick = moveCamera;
    topScene->uiObjects[0] = NEW_UI_TEXT("Platform: " + platform, 1, 0xFFFFFFFF, Scene::UI::ALIGN_LEFT, 0, 0, 0, 0.0F, .5, .5, false, false, nullptr);
    topScene->uiObjects[1] = NEW_UI_TEXT("Delta Time: 0ms", 1, 0xFFFFFFFF, Scene::UI::ALIGN_LEFT, 0, .06, 0, 0, 0.5, 0.5, false, false, fpsCounterTick);

    topScene->root->addChild(NEW_MODEL(assetProvider->getAssetLocation("/models/3DSSuzanne", MODEL_ASSET_TYPE), assetProvider->getAssetLocation("/gfx/UBSuzanneTex", TEXTURE_ASSET_TYPE), 0, 0, -3.0, 0, 0, 0, 0.5, 0.5, 0.5, rotateCubeTick));
    topScene->root->children.back()->addChild(NEW_CUBE(assetProvider->getAssetLocation("/gfx/commonTex", TEXTURE_ASSET_TYPE), 1, 1, 1, 0, 0, -3.0, 0, 0, 0, 2, 2, 2, rotateCubeTick));
    // no assets yet
    // topScene->objects[2] = NEW_3D_SPRITE("romfs:/gfx/marioTex.t3x", 1, 1, 64, 64, 41, 25, 250, 12, 0, 0, 0, 1, 1, frontPlayerTick);

    topScene->root->addChild(NEW_EMPTY(0, 0, 0, 0, 0, 0, 1, 1, 1, playMusic));

    bottomScene->ctx.camera->position.z = 1;
    bottomScene->root->addChild(NEW_MODEL(assetProvider->getAssetLocation("/models/3DSLogo", MODEL_ASSET_TYPE), assetProvider->getAssetLocation("/gfx/3DSLogoTex", TEXTURE_ASSET_TYPE), 0, 0, -3.0, 0, -90 * (M_PI/180), 0, 1, 1, 1, rotateCubeTickOneAxis));
    bottomScene->uiObjects[0] = NEW_UI_TEXT("Press Start to Exit", 1, 0xFFFFFFFF, Scene::UI::ALIGN_CENTER, .5, .92, 0, 0, .5, .5, false, false, nullptr);
}

bool Engine::update(Input::InputState inputState, Scene::Scene* topScene, Scene::Scene* bottomScene) { // returns true if app should end
    if (inputState.kDown & Input::KEY_START) {
        return true;
    }
    if (inputState.kDown & Input::KEY_SELECT) {
        softPanic("User initiated test");
    }
    topScene->tick(&inputState);
    bottomScene->tick(&inputState);

    return false;
}