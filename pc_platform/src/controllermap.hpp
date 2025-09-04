#pragma once
#ifndef PC_CONTROLLERMAP
#include <SDL3/SDL.h>
#include "amius_adventure.hpp"

bool initControllerMap();
void tickControllerMap();
void processSDLKeyDownEvent(SDL_Keycode keyCode);
void processSDLKeyUpEvent(SDL_Keycode keyCode);
void processSDLLostFocusEvent();
bool flipScreen();
AmiusAdventure::Input::InputState getInputState();

#define PC_CONTROLLERMAP
#endif // PC_CONTROLLERMAP