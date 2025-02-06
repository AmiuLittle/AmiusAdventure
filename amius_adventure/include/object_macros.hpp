#pragma once
#ifndef AMIUS_ADVENTURE_OBJECT_MACROS
#include "amius_adventure.hpp"

#define NEW_UI_TEXT(t, w, c, a, px, py, pz, r, sx, sy, fv, fh, ti) AmiusAdventure::Scene::UI::UIObject(AmiusAdventure::Scene::UI::UIRenderData { .type = AmiusAdventure::Scene::UI::RENDER_TEXT, .text = t, .dimension = {w, 0}, .basecolor = c, .align = a}, vec3{px, py, pz}, r, vec3{sx, sy}, fv, fh, ti)

#define NEW_CUBE(t, dx, dy, dz, px, py, pz, qx, qy, qz, sx, sy, sz, f) AmiusAdventure::Scene::Object(AmiusAdventure::Scene::RenderData { .type = AmiusAdventure::Scene::RENDER_CUBE, .model = "none", .texture = t, .dimension = {dx, dy, dz} }, vec3{px, py, pz}, vec3{qx, qy, qz}, vec3{sx, sy, sz}, f)

#define NEW_MODEL(m, t, px, py, pz, qx, qy, qz, sx, sy, sz, f) AmiusAdventure::Scene::Object(AmiusAdventure::Scene::RenderData { .type = AmiusAdventure::Scene::RENDER_MODEL, .model = m, .texture = t, .dimension = {0, 0, 0} }, vec3{px, py, pz}, vec3{qx, qy, qz}, vec3{sx, sy, sz}, f)

#define AMIUS_ADVENTURE_OBJECT_MACROS
#endif