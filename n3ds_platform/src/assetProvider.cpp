#include "assetProvider.hpp"
#include <sstream>
#include <filesystem>

AssetProvider::AssetProvider() {}

std::string AssetProvider::getAssetLocation(std::string path, AmiusAdventure::AssetType type) {
    std::stringstream ss;
    ss << "romfs:" << path;
    switch (type) {
        case AmiusAdventure::TEXTURE_ASSET_TYPE:
            ss << ".t3x";
            break;
        case AmiusAdventure::MODEL_ASSET_TYPE:
            ss << ".glb";
            break;
        case AmiusAdventure::SHADER_ASSET_TYPE:
            return std::filesystem::path(path).filename().string();
            break;
        case AmiusAdventure::MUSIC_ASSET_TYPE:
            ss << ".ogg";
            break;
        case AmiusAdventure::SFX_ASSET_TYPE:
            ss << ".pcm";
            break;
    }
    return ss.str();
}