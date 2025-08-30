#pragma once
#ifndef AMIUS_ADVENTURE_ASSET_PROVIDER_INTERFACE
#include <string>

namespace AmiusAdventure {
    enum AssetType {
        TEXTURE_ASSET_TYPE,
        MODEL_ASSET_TYPE,
        MUSIC_ASSET_TYPE,
        SFX_ASSET_TYPE
    };
    
    struct AssetProviderInterface {
        virtual std::string getAssetLocation(std::string path, AssetType type);
    };
}

#define AMIUS_ADVENTURE_ASSET_PROVIDER_INTERFACE
#endif