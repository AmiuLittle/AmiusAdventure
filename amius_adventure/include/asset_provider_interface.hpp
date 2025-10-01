#pragma once
#ifndef AMIUS_ADVENTURE_ASSET_PROVIDER_INTERFACE
#include <string>
#include "amius_adventure.hpp"

namespace AmiusAdventure {
    enum AssetType {
        TEXTURE_ASSET_TYPE,
        MODEL_ASSET_TYPE,
        SHADER_ASSET_TYPE,
        MUSIC_ASSET_TYPE,
        SFX_ASSET_TYPE
    };
    
    struct AssetProviderInterface {
        explicit AssetProviderInterface() {}
        virtual ~AssetProviderInterface() = default;
        virtual std::string getAssetLocation(std::string path, AssetType type) = 0;
        virtual bool loadMaterialAssets(const AmiusAdventure::Scene::Material& material) = 0; // Should also load shaders and textures along with material descriptor sets
        virtual bool loadModelAsset(std::string path) = 0;
        virtual bool loadMusicAsset(std::string path) = 0;
        virtual bool loadSFXAsset(std::string path) = 0;
    };
}

#define AMIUS_ADVENTURE_ASSET_PROVIDER_INTERFACE
#endif