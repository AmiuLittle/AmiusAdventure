#pragma once
#ifndef N3DS_ASSET_PROVIDER
#include "asset_provider_interface.hpp"

class AssetProvider : public AmiusAdventure::AssetProviderInterface {
    public:
        AssetProvider();
        std::string getAssetLocation(std::string, AmiusAdventure::AssetType);
};

#define N3DS_ASSET_PROVIDER
#endif