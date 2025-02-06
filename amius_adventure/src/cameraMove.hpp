#pragma once
#ifndef AMIUS_ADVENTURE_CAMERA_MOVE
#include "amius_adventure.hpp"

void moveCamera(AmiusAdventure::Scene::Camera*, AmiusAdventure::Scene::SceneCtx*, AmiusAdventure::Input::InputState*);

#define AMIUS_ADVENTURE_CAMERA_MOVE
#endif