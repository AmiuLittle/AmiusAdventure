#pragma once
#ifndef LIB_ROTATE_CUBE
#include "amius_adventure.hpp"

void rotateCubeTick(AmiusAdventure::Scene::Object*, AmiusAdventure::Scene::SceneCtx*, AmiusAdventure::Input::InputState*);
void rotateCubeTickOneAxis(AmiusAdventure::Scene::Object*, AmiusAdventure::Scene::SceneCtx*, AmiusAdventure::Input::InputState*);

#define LIB_ROTATE_CUBE
#endif