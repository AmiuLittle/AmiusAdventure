#include "audio.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "exitfuncs.hpp"
#include "error.hpp"

AudioEngine::AudioEngine() {
    if (!MIX_Init()) {
        exitWithErrorWindow(std::string("Unable to start SDL3_mixer: ") + SDL_GetError());
    }
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    bgm_track = MIX_CreateTrack(mixer);
}

AudioEngine::~AudioEngine() {
    MIX_DestroyMixer(mixer);
    MIX_Quit();
}

bool AudioEngine::bgm_isPlaying() const {
    return MIX_TrackPlaying(bgm_track);
}

bool AudioEngine::bgm_play(uint16_t fadeInTimeMS, uint8_t targetVolume, std::string path) {
    MIX_Audio* audio = MIX_LoadAudio(mixer, path.c_str(), false);
    if (audio == nullptr) {
        setErr(std::string("Failed to load audio for bgm_track: " + path + ": " + SDL_GetError()));
        return false;
    }
    if (bgm_isPlaying()) {
        MIX_StopTrack(bgm_track, 0);
    }
    if (!MIX_SetTrackAudio(bgm_track, audio)) {
        setErr(std::string("Failed to set bgm_track audio: " + path + ": " + SDL_GetError()));
        return false;
    }
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    if (!MIX_PlayTrack(bgm_track, props)) {
        setErr(std::string("Unable to play bgm_track: ") + path + ": " + SDL_GetError());
        return false;
    }
    return true;
} 

bool AudioEngine::bgm_stop(uint16_t fadeOutTimeMS) {
    MIX_StopTrack(bgm_track, static_cast<Sint64>(fadeOutTimeMS));
    return true;
}

bool AudioEngine::bgm_setVolume(uint8_t volume) {
    setErr("bgm_setVolume: Not Implemented!");
    return false;
}

bool AudioEngine::bgm_pause(uint16_t fadeOutTimeMS) {
    MIX_PauseTrack(bgm_track);
    return true;
}

bool AudioEngine::bgm_resume(uint16_t fadeInTimeMS) {
    MIX_ResumeTrack(bgm_track);
    return true;
}

bool AudioEngine::bgm_fadeTo(uint8_t newVolume, uint16_t fadeTimeMS) {
    return false;
}

bool AudioEngine::bgm_setTime(/* TODO */) {
    return false;
}

bool AudioEngine::se_play(std::string path) {
    return false;
}

bool AudioEngine::se_reset() {
    return false;
}

int AudioEngine::se_numPlaying() {
    return 0;
}