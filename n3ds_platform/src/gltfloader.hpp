#pragma once
#ifndef N3DS_GLTFLOADER
#include <string>
#include <optional>
#include <map>
#include "3dtypes.hpp"

bool loadModel(std::string path, std::map<std::string, ModelData>* loadedModels);

#define N3DS_GLTFLOADER
#endif