#pragma once
#ifndef AMIUS_ADVENTURE_OBJECT_MACROS
#include "amius_adventure.hpp"
#include <memory>

#define NEW_UI_TEXT(t, w, c, a, px, py, pz, r, sx, sy, fv, fh, ti) std::make_shared<AmiusAdventure::Scene::UI::UIObject>(AmiusAdventure::Scene::UI::UIRenderData { .type = AmiusAdventure::Scene::UI::RENDER_TEXT, .text = t, .dimension = {w, 0}, .basecolor = c, .align = a}, glm::vec3{px, py, pz}, r, glm::vec2{sx, sy}, fv, fh, ti)

#define NEW_CUBE(t, dx, dy, dz, px, py, pz, qx, qy, qz, sx, sy, sz, f) AmiusAdventure::Scene::Object::Create(AmiusAdventure::Scene::RenderData { .type = AmiusAdventure::Scene::RENDER_CUBE, .model = "none", .texture = t, .dimension = {dx, dy, dz}, std::nullopt }, glm::vec3{px, py, pz}, glm::vec3{qx, qy, qz}, glm::vec3{sx, sy, sz}, f)

#define NEW_MODEL(m, t, px, py, pz, qx, qy, qz, sx, sy, sz, f) AmiusAdventure::Scene::Object::Create(AmiusAdventure::Scene::RenderData { .type = AmiusAdventure::Scene::RENDER_MODEL, .model = m, .texture = t, .dimension = {0, 0, 0}, std::nullopt }, glm::vec3{px, py, pz}, glm::vec3{qx, qy, qz}, glm::vec3{sx, sy, sz}, f)

#define NEW_3D_SPRITE(t, dx, dy, sdx, sdy, sa, at, as, fpr, px, py, pz, sx, sy, f) AmiusAdventure::Scene::Object::Create(AmiusAdventure::Scene::RenderData { .type = AmiusAdventure::Scene::RENDER_3DSPRITE, .model = "none", .texture = t, .dimension = {dx, dy, 1.0}, AmiusAdventure::Scene::SpriteData { .spriteDimension = {sdx, sdy}, .currentAnimation = sa, .animationStart = 0, .animationTime = at, .animationStep = as, .framesPerRow = fpr } }, glm::vec3{px, py, pz}, glm::vec3{0, 0, 0}, glm::vec3{sx, sy, 1}, f)

#define NEW_EMPTY(px, py, pz, qx, qy, qz, sx, sy, sz, f) AmiusAdventure::Scene::Object::Create(AmiusAdventure::Scene::RenderData { .type = AmiusAdventure::Scene::RENDER_EMPTY, .model = "none", .texture = "none", .dimension = {1.0, 1.0, 1.0}, std::nullopt }, glm::vec3{px, py, pz}, glm::vec3{qx, qy, qz}, glm::vec3{sx, sy, sz}, f)

#define AMIUS_ADVENTURE_OBJECT_MACROS
#endif