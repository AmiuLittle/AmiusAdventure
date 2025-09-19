#include <GL/gl.h>
#include <SDL3/SDL.h>
#include <iostream>
#include "exitfuncs.hpp"
#include "controllermap.hpp"
#include "amius_adventure.hpp"
#include "quikmath.hpp"
#include "error.hpp"
#include "render.hpp"
#include "assetProvider.hpp"
#include "audio.hpp"

SDL_Window* mainWindow = nullptr;
AudioEngine* audioEngine = nullptr;

void exitGame(int exitCode) {
    if (audioEngine != nullptr) {
        delete audioEngine;
    }
    gfxQuit();
    if (mainWindow) {
        SDL_DestroyWindow(mainWindow);
    }
    SDL_Quit();
    exit(exitCode);
}

void exitWithErrorWindow(std::string err) {
    if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", err.c_str(), nullptr)) {
        // If the message box fails, we can only log the error
        std::cerr << "Error: " << err << std::endl;
    }
    exitGame(-1);
}

int main(int argc, char** argv) {
    if (!initControllerMap()) {
        exitWithErrorWindow("Failed to initialize controller map:" + getErr());
    }
    
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC)) {
        exitWithErrorWindow(std::string("Failed to initialize SDL: ") + SDL_GetError());
    }
    
    SDL_Window* mainWindow = SDL_CreateWindow("Amius Adventure", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE);
    if (!mainWindow) {
        exitWithErrorWindow(std::string("Failed to create SDL mainWindow: ") + SDL_GetError());
    }
    float aspectRatio = 800.0f / 600.0f; // Default aspect ratio
    
    SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    
    if (!initGfx(mainWindow)) {
        exitWithErrorWindow("Failed to initialize graphics: " + getErr());
    }
    
    SDL_ShowWindow(mainWindow);
    
    audioEngine = new AudioEngine();
    
    AmiusAdventure::Scene::Camera* topCamera = new AmiusAdventure::Scene::Camera(glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0}, 0.01f, 1000.0f, DegreesToRadians(40.0f), aspectRatio, nullptr);
    AmiusAdventure::Scene::Camera* bottomCamera = new AmiusAdventure::Scene::Camera(glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0}, 0.01f, 1000.0f, DegreesToRadians(40.0f), aspectRatio, nullptr);
    
    AmiusAdventure::Scene::Scene* topScene = new AmiusAdventure::Scene::Scene(topCamera, audioEngine);
    AmiusAdventure::Scene::Scene* bottomScene = new AmiusAdventure::Scene::Scene(bottomCamera, audioEngine);
    
    AssetProvider* assetProvider = new AssetProvider();
    AmiusAdventure::Engine* engine = new AmiusAdventure::Engine("PC", exitWithErrorWindow, topScene, bottomScene, assetProvider);
    
    bool running = true;
    bool useSecondScene = false;
    SDL_Event event;
    bool minimized = false;
    while (running) {
        tickControllerMap();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                processSDLKeyDownEvent(event.key.key);
            }
            else if (event.type == SDL_EVENT_KEY_UP) {
                processSDLKeyUpEvent(event.key.key);
            }
            else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
                processSDLLostFocusEvent();
            }
            else if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_METAL_VIEW_RESIZED) {
                setFramebufferResized();
            }
            else if (event.type == SDL_EVENT_WINDOW_MINIMIZED) {
                minimized = true;
            }
            else if (event.type == SDL_EVENT_WINDOW_RESTORED) {
                minimized = false;
            }
            else if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        
        if (engine->update(getInputState(), topScene, bottomScene)) {
            break;
        }

        if (!minimized) { // dont render if window is minimized
            if (flipScreen()) {
                useSecondScene = !useSecondScene;
            }
            gfxUpdate(useSecondScene ? bottomScene : topScene, !useSecondScene);
        }
    }

    exitGame(0);
}