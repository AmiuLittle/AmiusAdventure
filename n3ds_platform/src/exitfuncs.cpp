#include "exitfuncs.hpp"
#include <3ds.h>
#include <iostream>
#include "n3dslink.hpp"
#include "render.hpp"
#include "error.hpp"

u32 oldTimeLimit = UINT32_MAX;
u32 mainId = UINT32_MAX;
bool doPanic = false;

void unlockCore1() {
    svcGetThreadId(&mainId, CUR_THREAD_HANDLE);
    APT_GetAppCpuTimeLimit(&oldTimeLimit);
    APT_SetAppCpuTimeLimit(50);
}

void checkPanic() {
    if (doPanic) {
        softPanic(getErr());
    }
}

void threadCheckPanic() {
    if (doPanic) {
        threadExit(0);
    }
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
    doPanic = true;
    if (mainId != UINT32_MAX) {
        u32 id;
        svcGetThreadId(&id, CUR_THREAD_HANDLE);
        if (id != mainId) {
            setErr(reason);
            threadExit(0);
        }
    }

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