#include <iostream>
#include <fstream>
#include <tuple>
#include <3ds.h>
#include <c3d/maths.h>
#include <stdlib.h>
#include <iostream>
#include "amius_adventure.hpp"
#include "channel.hpp"
#include "n3dslink.hpp"
#include "exitfuncs.hpp"
#include "render.hpp"
#include "error.hpp"

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

    AmiusAdventure::Scene::Camera* topCamera = new AmiusAdventure::Scene::Camera(vec3{0, 0, 0}, vec3{0, 0, 0}, 0.01f, 1000.0f, (float)C3D_AngleFromDegrees(40.0f), C3D_AspectRatioTop, nullptr);
    AmiusAdventure::Scene::Camera* bottomCamera = new AmiusAdventure::Scene::Camera(vec3{0, 0, 0}, vec3{0, 0, 0}, 0.01f, 1000.0f, (float)C3D_AngleFromDegrees(40.0f), C3D_AspectRatioBot, nullptr);

    AmiusAdventure::Scene::Scene* topScene = new AmiusAdventure::Scene::Scene(topCamera);
    AmiusAdventure::Scene::Scene* bottomScene = new AmiusAdventure::Scene::Scene(bottomCamera);

    AmiusAdventure::Engine* engine = new AmiusAdventure::Engine("3DS", softPanic, topScene, bottomScene);

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

        gfxUpdate(topScene, bottomScene, iod / 12);

        gspWaitForVBlank();
    }

    exitGame();

    delete engine;
    delete bottomScene;
    delete topScene;

    return 0;
}
