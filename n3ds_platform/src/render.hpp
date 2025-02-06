#pragma once
#ifndef N3DS_BACKEND_RENDER
#include "amius_adventure.hpp"
#include <3ds.h>

/// @brief Start graphics
/// @return true if graphics start was successful
bool initGfx();

void gfxUpdate(AmiusAdventure::Scene::Scene* topScene, AmiusAdventure::Scene::Scene* bottomScene, float iod);

void exitGfx();

#define N3DS_BACKEND_RENDER
#endif