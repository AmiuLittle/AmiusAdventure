#pragma once
#ifndef PC_RENDER
#include "amius_adventure.hpp"
#include <SDL3/SDL.h>

void setFramebufferResized();

bool initGfx(SDL_Window*);

void gfxUpdate(AmiusAdventure::Scene::Scene*, bool);

void gfxQuit();

#define PC_RENDER
#endif