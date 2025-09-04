#pragma once
#ifndef N3DS_BACKEND_RENDER
#include "amius_adventure.hpp"
#include <3ds.h>

/// @brief Start graphics
/// @return true if graphics start was successful
bool initGfx();

/// @brief Does all the processing needed to display one frame of gameplay
/// @param topScene the scene object for the top screen
/// @param bottomScene the scene object for the bottom screen
/// @param iod the interocular distance set by the 3DS's 3D slider
void gfxUpdate(AmiusAdventure::Scene::Scene* topScene, AmiusAdventure::Scene::Scene* bottomScene, float iod);

/// @brief Clean up graphics
void exitGfx();

#ifdef TESTING_BUILD
#include <citro3d.h>
C3D_Mtx mat4x4_to_C3D_Mtx(const glm::mat4x4&);
glm::mat4x4 C3D_Mtx_to_mat4x4(const C3D_Mtx&);
#endif

#define N3DS_BACKEND_RENDER
#endif