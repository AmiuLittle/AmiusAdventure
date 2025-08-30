#include "controllermap.hpp"
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "simdjson.h"
#include "error.hpp"
#include "dirUtil.hpp"

std::unordered_map<SDL_Keycode, AmiusAdventure::Input::InputBits> keyMap = std::unordered_map<SDL_Keycode, AmiusAdventure::Input::InputBits> {
    {SDLK_M, AmiusAdventure::Input::KEY_A},
    {SDLK_N, AmiusAdventure::Input::KEY_B},
    {SDLK_BACKSLASH, AmiusAdventure::Input::KEY_SELECT},
    {SDLK_RETURN, AmiusAdventure::Input::KEY_START},
    {SDLK_D, AmiusAdventure::Input::KEY_DRIGHT},
    {SDLK_A, AmiusAdventure::Input::KEY_DLEFT},
    {SDLK_W, AmiusAdventure::Input::KEY_DUP},
    {SDLK_S, AmiusAdventure::Input::KEY_DDOWN},
    {SDLK_RSHIFT, AmiusAdventure::Input::KEY_R},
    {SDLK_LSHIFT, AmiusAdventure::Input::KEY_L},
    {SDLK_K, AmiusAdventure::Input::KEY_X},
    {SDLK_J, AmiusAdventure::Input::KEY_Y}
};

SDL_Keycode flipScreenKey = SDLK_SPACE;
bool flipScreenQueued = false;

AmiusAdventure::Input::InputState inputState = AmiusAdventure::Input::InputState {
    .kDown = 0,
    .kHeld = 0,
    .kUp = 0
};

bool inputStrToInputBit(const char* name, AmiusAdventure::Input::InputBits* output) {
    if (strcmp(name, "A") == 0) { *output = AmiusAdventure::Input::KEY_A; return true; }
    if (strcmp(name, "B") == 0) { *output = AmiusAdventure::Input::KEY_B; return true; }
    if (strcmp(name, "SELECT") == 0) { *output = AmiusAdventure::Input::KEY_SELECT; return true; }
    if (strcmp(name, "START") == 0) { *output = AmiusAdventure::Input::KEY_START; return true; }
    if (strcmp(name, "DRIGHT") == 0) { *output = AmiusAdventure::Input::KEY_DRIGHT; return true; }
    if (strcmp(name, "DLEFT") == 0) { *output = AmiusAdventure::Input::KEY_DLEFT; return true; }
    if (strcmp(name, "DUP") == 0) { *output = AmiusAdventure::Input::KEY_DUP; return true; }
    if (strcmp(name, "DDOWN") == 0) { *output = AmiusAdventure::Input::KEY_DDOWN; return true; }
    if (strcmp(name, "R") == 0) { *output = AmiusAdventure::Input::KEY_R; return true; }
    if (strcmp(name, "L") == 0) { *output = AmiusAdventure::Input::KEY_L; return true; }
    if (strcmp(name, "X") == 0) { *output = AmiusAdventure::Input::KEY_X; return true; }
    if (strcmp(name, "Y") == 0) { *output = AmiusAdventure::Input::KEY_Y; return true; }
    return false;
}

void replaceKeybind(SDL_Keycode key, AmiusAdventure::Input::InputBits inputBit) {
    if (key == SDLK_UNKNOWN) {
        setErr("Invalid key for replacement");
        return;
    }

    // Remove existing binding if it exists
    for (const auto pair : keyMap) {
        if (pair.second == inputBit) {
            keyMap.erase(pair.first);
            break; // Only remove the first matching binding
        }
    }

    // Add new binding
    keyMap.insert({key, inputBit});
}

bool initControllerMap() {
    const char* configDir = getConfigDir();
    if (!configDir) {
        setErr("Failed to get config directory");
        return false;
    }
    std::filesystem::path configPath(configDir);
    std::filesystem::path controlMapPath = configPath / "control_map.json";
    std::filesystem::create_directories(std::filesystem::path(configDir));
    if (!std::filesystem::exists(controlMapPath)) {
        return true; // No custom control map, use defaults
    }

    if (!std::filesystem::is_regular_file(controlMapPath)) {
        setErr("Control map path is not a regular file: " + controlMapPath.string());
        return false;
    }

    std::ifstream data(controlMapPath);
    std::stringstream buffer;
    buffer << data.rdbuf();
    std::string json = buffer.str();
    data.close();

    simdjson::dom::parser parser = simdjson::dom::parser();
    simdjson::dom::element doc = parser.parse(json);

    if (parser.error != simdjson::SUCCESS) {
        setErr("Failed to parse control map JSON: " + std::string(simdjson::error_message(parser.error)));
        return false;
    }

    simdjson::simdjson_result<simdjson::dom::object> docObj = doc.get_object();

    if (docObj.error() != simdjson::SUCCESS) {
        setErr("Control map JSON root is not an object");
        return false;
    }

    simdjson::simdjson_result<simdjson::dom::element> map = docObj.value_unsafe()["map"];

    if (map.error() != simdjson::SUCCESS) {
        setErr("Control map JSON does not contain 'map' field");
        return false;
    }

    simdjson::simdjson_result<simdjson::dom::object> mapObjRes = map.value_unsafe().get_object();

    if (mapObjRes.error() != simdjson::SUCCESS) {
        setErr("Control map JSON 'map' field is not an object");
        return false;
    }

    simdjson::dom::object mapObj = mapObjRes.value_unsafe();

    for (const simdjson::dom::key_value_pair pair : mapObj) {
        AmiusAdventure::Input::InputBits inputBit;
        if (!inputStrToInputBit(pair.key.data(), &inputBit)) {
            if (strcmp(pair.key.data(), "Flip") == 0) {
                SDL_Keycode newFlipScreenKey = SDL_GetKeyFromName(pair.value.get_string().value_unsafe().data());
                if (flipScreenKey == SDLK_UNKNOWN) {
                    setErr("Control map JSON value for 'Flip' is not a valid SDL key");
                    return false;
                }
                flipScreenKey = newFlipScreenKey;
                continue;
            }
            setErr("Unknown input string in control map: " + std::string(pair.key.data()));
            return false;
        }
        simdjson::simdjson_result<std::string_view> scancodeName = pair.value.get_string();
        if (scancodeName.error() != simdjson::SUCCESS) {
            setErr("Control map JSON value for '" + std::string(pair.key.data()) + "' is not a string");
            return false;
        }
        SDL_Keycode keycode = SDL_GetKeyFromName(scancodeName.value_unsafe().data());
        if (keycode == SDLK_UNKNOWN) {
            setErr("Control map JSON value for '" + std::string(pair.key.data()) + "' is not a valid SDL scancode");
            return false;
        }

        replaceKeybind(keycode, inputBit);
    }

    return true;
}

void tickControllerMap() {
    inputState.kDown = 0;
    inputState.kUp = 0;
}

void processSDLKeyDownEvent(SDL_Keycode key) {
    auto it = keyMap.find(key);
    if (it != keyMap.end()) {
        inputState.kDown |= it->second;
        inputState.kHeld |= it->second;
        return;
    }
    if (key == flipScreenKey) flipScreenQueued = true;
}

void processSDLKeyUpEvent(SDL_Keycode key) {
    auto it = keyMap.find(key);
    if (it != keyMap.end()) {
        inputState.kUp |= it->second;
        inputState.kHeld &= ~it->second; // Remove from held state
    }
}

void processSDLLostFocusEvent() {
    inputState.kDown = 0;
    inputState.kHeld = 0;
    inputState.kUp = 0;
    flipScreenQueued = false;
}

bool flipScreen() {
    if (flipScreenQueued) {
        flipScreenQueued = false;
        return true;
    }
    return false;
}

AmiusAdventure::Input::InputState getInputState() {
    return inputState;
}