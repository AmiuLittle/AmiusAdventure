#include <3ds.h>
#include <iostream>
#include <map>
#include <vector>
#include "error.hpp"
#include "../src/n3dslink.hpp"
#include "../src/exitfuncs.hpp"
#include "testFunction.hpp"
#include "logToSDMC.hpp"
#include "graphicsMath.hpp"

#define SELECT_COLOR CONSOLE_ESC(47;34m)
#define RESET_COLORS CONSOLE_ESC(44;37m)
#define SUCCESS_COLOR CONSOLE_ESC(44;32m)
#define ERROR_COLOR CONSOLE_ESC(44;31m)

std::map<int, std::string> categories = {
    {0, "Graphics Math"}
};

std::vector<std::pair<testFunc, std::string>> allTests {
    {mat4x4_to_C3D_Mtx_test, "convert between glm::mat4x4 and C3D_Mtx"}
};

void exitTests(int code = 0) {
    exit3dslink();
    gfxExit();
    cfguExit();
    exit(code);
}

void drawMenu(int selection) {
    std::cout << CONSOLE_ESC(44;37m);
    consoleClear();
    std::cout << "AMIU'S ADVENTURE TESTS" << std::endl;
    std::cout << std::endl;
    std::cout << "Select an option:" << std::endl;
    if (selection == 0) {
        std::cout << SELECT_COLOR << "All Tests" << RESET_COLORS << std::endl;
    }
    else {
        std::cout << RESET_COLORS << "All Tests" << std::endl;
    }
    int i = 1;
    for (auto category : categories) {
        if (i == selection) {
            std::cout << SELECT_COLOR << category.second << RESET_COLORS << std::endl;
        }
        else {
            std::cout << RESET_COLORS << category.second << std::endl;
        }
        i++;
    }
}

int main() {
    cfguInit();
    Result rc = romfsInit();
    if (rc) {
        softPanic("Romfs init err: " + std::to_string(rc));
    }
    atexit([](){romfsExit();});

    if (!initSdmcLogger()) {
        softPanic("Failed to initialize sdmc logger: " + getErr());
    }

    gfxInitDefault();
    gfxSetWide(true);
    consoleSelect(consoleInit(GFX_TOP, NULL));
    
    std::cout << CONSOLE_ESC(44;37m);
    consoleClear();
    std::cout << "AMIU'S ADVENTURE TESTS" << std::endl;
    std::cout << std::endl;
    std::cout << "Select an option:" << std::endl;
    std::cout << SELECT_COLOR << "All Tests" << RESET_COLORS << std::endl;
    std::vector<int> options = {-1};
    for (auto category : categories) {
        options.push_back(category.first);
        std::cout << category.second << std::endl;
    }

    int selection = 0;
    while (true) {
        if (!aptMainLoop()) {
            exitTests();
        }
        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            exitTests();
        }
        if (kDown & KEY_DOWN && selection < categories.size()) {
            selection++;
            drawMenu(selection);
        }
        if (kDown & KEY_UP && selection > 0) {
            selection--;
            drawMenu(selection);
        }
        if (kDown & KEY_A) {
            break;
        }

        gspWaitForVBlank();
    }

    int i = 0;
    int endIdx = allTests.size();
    if (selection != 0) {
        i = options[selection];
        if (selection + 1 < options.size()) {
            endIdx = options[selection + 1];
        }
    }

    std::cout << CONSOLE_ESC(44;37m);
    consoleClear();
    for (; i < endIdx; i++) {
        std::cout << allTests[i].second << " - ";
        sdmcLogLn("--- " + allTests[i].second + " ---");
        sdmcLogLn();
        if ((*allTests[i].first)()) {
            std::cout << SUCCESS_COLOR << "Success!" << RESET_COLORS << std::endl;
            sdmcLogLn();
            sdmcLogLn("--- Final exit status: Success! ---");
        }
        else {
            std::cout << ERROR_COLOR << "ERROR" << RESET_COLORS << std::endl;
            sdmcLogLn();
            sdmcLogLn("--- Final exit status: ERROR ---");
        }
        sdmcLogLn();
    }

    std::cout << std::endl;
    std::cout << "Press start to exit..." << std::endl;
    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_START) break;
        gspWaitForVBlank();
    }

    exitTests();
}