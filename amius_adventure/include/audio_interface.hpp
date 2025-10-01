#pragma once
#ifndef AMIUS_ADVENTURE_AUDIO
#include <stdint.h>
#include <chrono>
#include <string>

// Inspiration taken from https://github.com/EasyRPG/Player/ 
// Go to their repo and check out src/audio.h for their implementation

struct AudioInterface {
    explicit AudioInterface() {}
    virtual ~AudioInterface() = default;
    
    /* BGM:
       - Only one audio file can be played at a time
       - Always loops
       - Fading
    */
    virtual bool bgm_play(uint16_t fadeInTimeMS, uint8_t targetVolume, std::string path) = 0;
    virtual bool bgm_pause(uint16_t fadeOutTimeMS) = 0;
    virtual bool bgm_resume(uint16_t fadeInTimeMS) = 0;
    virtual bool bgm_stop(uint16_t fadeOutTimeMS) = 0;
    virtual bool bgm_fadeTo(uint8_t newVolume, uint16_t fadeTimeMS) = 0;
    virtual bool bgm_setVolume(uint8_t newVolume) = 0;
    virtual bool bgm_setTime(/* TODO */) = 0;
    virtual bool bgm_isPlaying() const = 0;

    /* SE:
       - Multiple audio files
       - No looping
       - Can only stop all sound effects not just one
    */
    virtual bool se_play(std::string path) = 0;
    virtual bool se_reset() = 0; // stops all sound effects
    virtual int se_numPlaying() = 0;

    /* LOADING SCREEN:
         - The 3DS cannot load assets and submit gpu commands to display the loading screen at the same time
         - Therefore we use the audio thread to submit gpu commands while the main thread loads assets
         - On the 3DS this will stop all sound processing
            - For non 3DS platforms you can either stop sound for accuacy or add your own loading screen music
    */
    virtual bool ls_show() = 0; // stop all sound processing and start gpu command submission for displaying loading screen
    virtual bool ls_stop() = 0; // stop gpu command submission (THIS SHOULD NOT RETURN UNTIL AUDIO INTERFACE IS READY TO RECIEVE AUDIO COMMANDS AGAIN)
};

#define AMIUS_ADVENTURE_AUDIO
#endif