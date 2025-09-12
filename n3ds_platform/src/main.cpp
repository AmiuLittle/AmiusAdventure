#include <iostream>
#include <fstream>
#include <tuple>
#include <3ds.h>
#include <c3d/maths.h>
#include <stdlib.h>
#include "amius_adventure.hpp"
#include "channel.hpp"
#include "n3dslink.hpp"
#include "exitfuncs.hpp"
#include "render.hpp"
#include "error.hpp"
#include "audio.hpp"
#include "assetProvider.hpp"

#ifndef TESTING_BUILD
int main() {
    cfguInit();
    if (!initGfx()) {
        softPanic("could not init GFX: " + getErr());
    }

    if (!init3dslinkStdio()) {
        softPanic("could not init SOC");
    }

    unlockCore1(); // necessary for sound

    Result rc = romfsInit();
    if (rc) {
        softPanic("Romfs init err: " + std::to_string(rc));
    }
    atexit([](){romfsExit();});

    AudioEngine* audioEngine = new AudioEngine();

    AmiusAdventure::Scene::Camera* topCamera = new AmiusAdventure::Scene::Camera(glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0}, 0.01f, 1000.0f, (float)C3D_AngleFromDegrees(40.0f), C3D_AspectRatioTop, nullptr);
    AmiusAdventure::Scene::Camera* bottomCamera = new AmiusAdventure::Scene::Camera(glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0}, 0.01f, 1000.0f, (float)C3D_AngleFromDegrees(40.0f), C3D_AspectRatioBot, nullptr);

    AmiusAdventure::Scene::Scene* topScene = new AmiusAdventure::Scene::Scene(topCamera, audioEngine);
    AmiusAdventure::Scene::Scene* bottomScene = new AmiusAdventure::Scene::Scene(bottomCamera, audioEngine);

    AssetProvider* assetProvider = new AssetProvider();
    AmiusAdventure::Engine* engine = new AmiusAdventure::Engine("3DS", softPanic, topScene, bottomScene, assetProvider);

    while (aptMainLoop()) {
        hidScanInput();
        float iod = osGet3DSliderState();

        if (engine->update(AmiusAdventure::Input::InputState {
            .kDown = hidKeysDown(),
            .kHeld = hidKeysHeld(),
            .kUp = hidKeysUp()
        },
        topScene,
        bottomScene
        )) {
            break;
        }

        gfxUpdate(topScene, bottomScene, iod / 32);

        checkPanic();

        gspWaitForVBlank();
    }

    exitGame();

    delete engine;
    delete bottomScene;
    delete topScene;
    delete audioEngine;

    return 0;
}
#endif
