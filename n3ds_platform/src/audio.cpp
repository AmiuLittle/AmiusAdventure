#include "audio.hpp"
#include <3ds.h>
#include <fstream>
#include <cstring>
#include <memory>
#include <iostream>
#include <algorithm>
#include "exitfuncs.hpp"
#include "error.hpp"

// Inspiration taken from https://github.com/EasyRPG/Player
// Go to their repo and check out src/platform/3ds/audio.cpp for their implementation

int fill_block = 0;
bool terminate_thread = false;

bool dsp_init = false;
LightLock audio_mutex;
LightEvent audio_event;
Thread audio_thread;

struct _SoundEffect {
    constexpr static int channels = 23;
    ndspWaveBuf buf[channels] = {0};
} SoundEffect;

struct _BackgroundMusic {
    constexpr static int bufSize = 8 * 1024;
    constexpr static int channel = SoundEffect.channels;
    constexpr static int numBufs = 3;
    ndspWaveBuf buf[numBufs] = {0};
    uint32_t* audioBuffer;
    std::unique_ptr<AudioDecoderBase> decoder = nullptr;
} BackgroundMusic;

AudioDecoderBase::AudioDecoderBase() {
    
}

bool setChannelFormat(int dspChn, AudioFormats format, int channels, AudioFormats& outFormat) {
    outFormat = format;
    bool res = true;

    if (channels < 1 || channels > 2) {
        setErr("setChannelFormat: Invalid number of channels '" + std::to_string(channels) + "'");
        return false;
    }

    switch (format) {
    case FORMAT_U8:
        res = false;
        outFormat = FORMAT_S8;
    case FORMAT_S8:
        ndspChnSetFormat(dspChn, channels == 1 ? NDSP_FORMAT_MONO_PCM8 : NDSP_FORMAT_STEREO_PCM8);
        break;
    case FORMAT_U16:
    case FORMAT_U32:
    case FORMAT_S32:
    case FORMAT_F32:
        res = false;
        outFormat = FORMAT_S16;
    case FORMAT_S16:
        ndspChnSetFormat(dspChn, channels == 1 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_STEREO_PCM16);
        break;
    default:
        setErr("setChannelFormat: Unhandled channel format");
        return false;
    }

    if (!res) {
        setErr("setChannelFormat: Unsupported format, using one close to instead");
    }

    return res;
}

void n3ds_dsp_callback(void* userdata) {
    AudioEngine* _audio = static_cast<AudioEngine*>(userdata);

    if (BackgroundMusic.buf[fill_block].status == NDSP_WBUF_DONE) {
        BackgroundMusic.buf[fill_block].status = NDSP_WBUF_FREE;
        LightEvent_Signal(&audio_event);
    }
}

void n3ds_audio_thread(void* userdata) {
    AudioEngine* audio = static_cast<AudioEngine*>(userdata);
    float mix[12] = {0};

    while (!terminate_thread) {
        LightEvent_Wait(&audio_event);
        
        int target_block = fill_block;
        ++fill_block;
        fill_block %= BackgroundMusic.numBufs;

        audio->lockMutex();

        if (BackgroundMusic.decoder) {
            BackgroundMusic.decoder->Decode(
                reinterpret_cast<uint8_t*>(BackgroundMusic.buf[target_block].data_pcm16),
                BackgroundMusic.bufSize
            );
            DSP_FlushDataCache(BackgroundMusic.buf[target_block].data_pcm16, BackgroundMusic.bufSize);
            mix[0] = mix[1] = BackgroundMusic.decoder->GetVolume() / 100.0f;
            ndspChnSetMix(BackgroundMusic.channel, mix);
            ndspChnWaveBufAdd(BackgroundMusic.channel, &BackgroundMusic.buf[target_block]);
        }
        else {
            BackgroundMusic.buf[target_block].status = NDSP_WBUF_DONE;
        }

        audio->unlockMutex();

        threadCheckPanic();
    }

    threadExit(0);
}

AudioEngine::AudioEngine() {
    Result res = ndspInit();
    if(R_FAILED(res)) {
        if (R_SUMMARY(res) == RS_NOTFOUND && R_MODULE(res) == RM_DSP) {
            softPanic("Please dump your DSP firmware, guides are on https://3ds.hacks.guide");
        }
        else {
            softPanic("DSP init error: " + std::to_string(R_DESCRIPTION(res)));
        }
    }
    ndspSetOutputCount(1);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspSetClippingMode(NDSP_CLIP_NORMAL);

    for (int i = 0; i <= BackgroundMusic.channel; ++i) {
        ndspChnSetInterp(i, NDSP_INTERP_LINEAR);
    }

    dsp_init = true;

    LightLock_Init(&audio_mutex);
    LightEvent_Init(&audio_event, RESET_ONESHOT);

    BackgroundMusic.audioBuffer = reinterpret_cast<uint32_t*>(linearAlloc(BackgroundMusic.bufSize * BackgroundMusic.numBufs));
    for (int i = 0; i < BackgroundMusic.numBufs; ++i) {
        BackgroundMusic.buf[i].data_vaddr = &BackgroundMusic.audioBuffer[i * BackgroundMusic.bufSize];
        BackgroundMusic.buf[i].nsamples = BackgroundMusic.bufSize / 4;
        BackgroundMusic.buf[i].status = NDSP_WBUF_DONE;
    }

    ndspSetCallback(n3ds_dsp_callback, this);

    audio_thread = threadCreate(n3ds_audio_thread, this, 32768, 0x18, 1, true);
    if (!audio_thread) {
        softPanic("Could not create audio thread!");
    }
}

AudioEngine::~AudioEngine() {
    if (!dsp_init) {
        return;
    }

    terminate_thread = true;
    LightEvent_Signal(&audio_event);
    threadJoin(audio_thread, U64_MAX);
    LightEvent_Clear(&audio_event);
    ndspExit();

    linearFree(BackgroundMusic.audioBuffer);
    for (int i = 0; i < SoundEffect.channels; ++i) {
        if (SoundEffect.buf[i].data_pcm16 != nullptr) {
            linearFree(SoundEffect.buf[i].data_pcm16);
        }
    }
}

void AudioEngine::lockMutex() const {
    LightLock_Lock(&audio_mutex);

}

void AudioEngine::unlockMutex() const {
    LightLock_Unlock(&audio_mutex);
}

bool AudioEngine::bgm_isPlaying() const {
    return BackgroundMusic.decoder ? true : false;
}

bool AudioEngine::bgm_play(uint16_t fadeInTimeMS, uint8_t targetVolume, std::string path) {
    if (!dsp_init) {
        setErr("bgm_play: DSP not inited yet!");
        return false;
    }

    lockMutex();
    BackgroundMusic.decoder = createDecoder(path);
    if (BackgroundMusic.decoder && (*BackgroundMusic.decoder).Open()) {
        int freq;
        AudioFormats format, outFormat;
        int chans;

        (*BackgroundMusic.decoder).GetFormat(freq, format, chans);
        (*BackgroundMusic.decoder).SetVolume(targetVolume);
        (*BackgroundMusic.decoder).doLoop = true;

        if (!setChannelFormat(BackgroundMusic.channel, format, chans, outFormat)) {
            if (outFormat == format) { // similar format was not used, there was an error
                softPanic(getErr());
            }
            else {
                softPanic(getErr());
            }
        }
        ndspChnSetRate(BackgroundMusic.channel, freq);

        const int sampleSize = getSampleSizeForFormat(outFormat);
        const int nsamples = BackgroundMusic.bufSize / (sampleSize * chans);
        for (int i = 0; i < BackgroundMusic.numBufs; ++i) {
            BackgroundMusic.buf[i].nsamples = nsamples;
        }
    }
    else {
        return false;
    }

    unlockMutex();
    return true;
} 

bool AudioEngine::bgm_stop(uint16_t fadeOutTimeMS) {
    return true;
}

bool AudioEngine::bgm_setVolume(uint8_t volume) {
    lockMutex();

    BackgroundMusic.decoder->SetVolume(volume);

    unlockMutex();
    return true;
}

bool AudioEngine::bgm_pause(uint16_t fadeOutTimeMS) {
    return false;
}

bool AudioEngine::bgm_resume(uint16_t fadeInTimeMS) {
    return false;
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

bool AudioEngine::ls_show() {
    setErr("ls_show: Not implemented yet!");
    return false;
}

bool AudioEngine::ls_stop() {
    setErr("ls_stop: Not implemented yet!");
    return false;
}

int AudioDecoderBase::Decode(uint8_t* buffer, int size) {
    return Decode(buffer, size, 0);
}

int AudioDecoderBase::Decode(uint8_t* buffer, int size, int recursion_depth) {
    int res = FillBuffer(buffer, size);

    if (res < 0) {
        memset(buffer, '\0', size);
    }
    else if (res < size) {
        memset(&buffer[res], '\0', size - res);
    }
    
    if (IsFinished() && doLoop && recursion_depth < 10) {
        ++loopCount;
        Rewind();
        if (size - res > 0) {
            int res2 = Decode(&buffer[res], size - res, ++recursion_depth);
            if (res2 <= 0) {
                return res;
            }
            return res + res2;
        }
    }

    if (recursion_depth >= 10) {
        softPanic("Decode: Max recursion depth exceeded!");
    }

    return res;
}

bool AudioDecoderBase::Rewind() {
    if (!Seek(0, std::ios::beg)) {
        softPanic("Rewind: Guaranteed function call threw an error!");
    }
    return true;
}

std::unique_ptr<AudioDecoderBase> createDecoder(std::string file) {
    std::ifstream* fileStream = new std::ifstream(file);
    uint32_t magic = 0;
    char charMagic[4] = {0};
    if (!fileStream || !fileStream->good()) {
        setErr("Given file not found: " + file);
        return nullptr;
    }
    fileStream->seekg(std::ios::beg);
    (*fileStream).read(charMagic, sizeof(charMagic));
    std::reverse(charMagic, charMagic + sizeof(charMagic));
    std::memcpy(&magic, charMagic, sizeof(charMagic));

    if (magic == OGG_MAGIC) {
        // VORBIS
        (*fileStream).seekg(29, std::ios::beg);
        (*fileStream).read(charMagic, sizeof(charMagic));
        std::reverse(charMagic, charMagic + sizeof(charMagic));
        std::memcpy(&magic, charMagic, sizeof(charMagic));
        if (magic == VORBIS_MAGIC) {
            fileStream->seekg(std::ios::beg);
            return std::make_unique<VorbisDecoder>(fileStream);
        }
        // OPUS
        (*fileStream).seekg(28, std::ios::beg);
        (*fileStream).read(charMagic, sizeof(charMagic));
        std::reverse(charMagic, charMagic + sizeof(charMagic));
        std::memcpy(&magic, charMagic, sizeof(charMagic));
        if (magic == OPUS_MAGIC) {
            fileStream->seekg(std::ios::beg);
            return std::make_unique<OpusDecoder>(fileStream);
        }
    }

    std::reverse(charMagic, charMagic + sizeof(charMagic));
    setErr("Given audio file is not in a supported format, last format magic tried: '" + std::string(charMagic, sizeof(charMagic)) + "' in file: " + file);
    return nullptr;
}

int getSampleSizeForFormat(AudioFormats format) {
    switch (format) {
    case FORMAT_S8:
    case FORMAT_U8:
        return 1;
    case FORMAT_S16:
    case FORMAT_U16:
        return 2;
    case FORMAT_F32:
    case FORMAT_S32:
    case FORMAT_U32:
        return 4;
    }

    softPanic("getSampleSizeForFormat: Unhandled format!");
    return -1;
}
