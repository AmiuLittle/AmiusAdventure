#include <iostream>
#include <fstream>
#include <tuple>
#include <3ds.h>
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
    //consoleSelect(consoleInit(GFX_BOTTOM, NULL));

    if (!init3dslinkStdio()) {
        softPanic("could not init SOC");
    }
    std::cout << "Sanity Check\n";

    unlockCore1(); // necessary for sound

    Result rc = romfsInit();
    if (rc) {
        softPanic("Romfs init err: " + std::to_string(rc));
    }
    atexit([](){romfsExit();});

    AmiusAdventure::Scene::Scene* topScene = new AmiusAdventure::Scene::Scene();
    AmiusAdventure::Scene::Scene* bottomScene = new AmiusAdventure::Scene::Scene();

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
