#include "dirUtil.hpp"
#include <SDL3/SDL.h>
#include <filesystem>

const char* getConfigDir(){
    const char* configDir = SDL_GetPrefPath("AmiuLittle", "AmiusAdventure");
    if (!configDir) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get config directory: %s", SDL_GetError());
        return nullptr;
    }
    if (!std::filesystem::is_directory(configDir)) {
        std::filesystem::remove_all(std::filesystem::path(configDir));
    }
    if (!std::filesystem::exists(configDir)) {
        std::filesystem::create_directories(std::filesystem::path(configDir));
    }
    return configDir;
}