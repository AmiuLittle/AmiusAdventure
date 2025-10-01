#pragma once 
#ifndef PC_GLTFLOADER
#include <string>
#include <map>
#include "renderPrimitives.hpp"

bool loadFromGLB(std::string, std::map<std::string, ModelData>* loadedModels);

#define PC_GLTFLOADER
#endif