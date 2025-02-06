#pragma once
#ifndef LIB_FPS_COUNTER
#include "amius_adventure.hpp"

void fpsCounterTick(AmiusAdventure::Scene::UI::UIObject*, AmiusAdventure::Scene::SceneCtx*, AmiusAdventure::Input::InputState*);

#define LIB_FPS_COUNTER
#endif