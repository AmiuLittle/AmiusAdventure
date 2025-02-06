#include "exitfuncs.hpp"
#include <3ds.h>
#include <iostream>
#include "n3dslink.hpp"
#include "render.hpp"

u32 oldTimeLimit = UINT32_MAX;

void unlockCore1() {
    APT_GetAppCpuTimeLimit(&oldTimeLimit);
    APT_SetAppCpuTimeLimit(30);
}

void exitGame() {
    romfsExit();
    exit3dslink();
    gfxExit();
    cfguExit();

    if (oldTimeLimit != UINT32_MAX) {
        APT_SetAppCpuTimeLimit(oldTimeLimit);
    }
}

void softPanic(std::string reason) {
    gfxInitDefault();
    consoleSelect(consoleInit(GFX_TOP, NULL));

    std::cout << CONSOLE_ESC(44;37m);
    consoleClear();
    std::cout << ":<" << std::endl;
    std::cout << std::endl;
    std::cout << "The progam has encounted an error and cannot" << std::endl;
    std::cout << "continue." << std::endl;
    std::cout << std::endl;
    std::cout << "Reason: " + reason << std::endl;
    std::cout << std::endl;
    std::cout << "Press start to exit" << std::endl;
    std::cout << CONSOLE_RESET;

    while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) break;

        gspWaitForVBlank();
    }
    exitGame();
    exit(1);
}