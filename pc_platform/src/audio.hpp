#pragma once
#ifndef PC_PLATFORM_AUDIO
#include "audio_interface.hpp"
#include <SDL3_mixer/SDL_mixer.h>

enum AudioFormatMagics {
    OGG_MAGIC = 0x4F676753,
    VORBIS_MAGIC = 0x766F7262
};

class AudioEngine : public AudioInterface {
private:
    MIX_Mixer* mixer;
    MIX_Track* bgm_track;
public:
    AudioEngine();
    ~AudioEngine();
    bool bgm_play(uint16_t fadeInTimeMS, uint8_t targetVolume, std::string path) override;
    bool bgm_stop(uint16_t fadeOutTimeMS) override;
    bool bgm_isPlaying() const override;
    bool bgm_pause(uint16_t fadeOutTimeMS);
    bool bgm_resume(uint16_t fadeInTimeMS);
    bool bgm_fadeTo(uint8_t newVolume, uint16_t fadeTimeMS);
    bool bgm_setVolume(uint8_t newVolume);
    bool bgm_setTime(/* TODO */);

    bool se_play(std::string path);
    bool se_reset();
    int se_numPlaying();
};

#define PC_PLATFORM_AUDIO
#endif