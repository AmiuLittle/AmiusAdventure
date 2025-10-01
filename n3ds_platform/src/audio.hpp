#pragma once
#ifndef N3DS_PLATFORM_AUDIO
#include "audio_interface.hpp"
#include <string>
#include <fstream>
#include <memory>
#include <opusfile.h>
#include <tremor/ivorbisfile.h>

enum AudioFormatMagics {
    OGG_MAGIC = 0x4F676753,
    OPUS_MAGIC = 0x4F707573,
    VORBIS_MAGIC = 0x766F7262
};

class AudioEngine : public AudioInterface {
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

    bool ls_show();
    bool ls_stop();

    void lockMutex() const;
    void unlockMutex() const;
};

enum SupportedFormats {
    DETECT_FORMAT,
    OPUS_FORMAT,
    VORBIS_FORMAT
};

enum AudioFormats {
    FORMAT_S8,
    FORMAT_U8,
    FORMAT_S16,
    FORMAT_U16,
    FORMAT_S32,
    FORMAT_U32,
    FORMAT_F32
};

int getSampleSizeForFormat(AudioFormats format);

class AudioDecoderBase {
public:
    bool doLoop = false;
    int loopCount = 0;

    std::string musicType = "unknown";

    /// @brief Don't use this, I just need this constructor to stop the compiler from complaining
    explicit AudioDecoderBase();
    /// @brief Make a decoder for a file
    /// @param file_string the file to be decoded
    explicit AudioDecoderBase(std::unique_ptr<std::ifstream> file_string);
    virtual ~AudioDecoderBase() = default;

    int Decode(uint8_t* buffer, int size);
    int Decode(uint8_t* buffer, int size, int recursion_depth);
    bool Rewind();

    virtual bool Open() = 0;
    virtual int FillBuffer(uint8_t* buffer, int size) = 0;
    virtual bool IsFinished() = 0;
    virtual bool Seek(std::streamoff offset, std::ios::seekdir origin) = 0;
    virtual void GetFormat(int& freq, AudioFormats& format, int& chans) = 0;
    virtual void SetVolume(uint8_t valume) = 0;
    virtual uint8_t GetVolume() = 0;
};

/// @brief Detects the format of a file and returns the appropriate decoder
/// @param file the target sound file
/// @return an audio decoder
/// @note if you wish to use a specific decoder, initiate the class directly
std::unique_ptr<AudioDecoderBase> createDecoder(std::string file);

class OpusDecoder : public AudioDecoderBase {
private:
    std::ifstream* fileStream;
    bool finished = false;
    OggOpusFile* oof;
    uint8_t volume;

    const int frequency = 48000;
    const int channels = 2;

    struct {
        bool looping = false;
        bool to_end = false;
        int64_t start;
        int64_t end;
    } loopState;
public:
    OpusDecoder(std::ifstream* fileStream);
    ~OpusDecoder();

    bool Open() override;
    int FillBuffer(uint8_t* buffer, int size) override;
    bool IsFinished() override;
    bool Seek(std::streamoff offset, std::ios::seekdir origin) override;
    void GetFormat(int& freq, AudioFormats& format, int& chans) override;
    void SetVolume(uint8_t valume) override;
    uint8_t GetVolume() override;
};

class VorbisDecoder : public AudioDecoderBase {
private:
    std::ifstream* fileStream;
    bool finished = false;
    OggVorbis_File* ovf;
    uint8_t volume;
    int frequency = 48000;
    int channels = 2;

    struct {
        bool looping = false;
        bool to_end = false;
        int64_t start;
        int64_t end;
    } loopState;
public:
    VorbisDecoder(std::ifstream* fileStream);
    ~VorbisDecoder();

    bool Open() override;
    int FillBuffer(uint8_t* buffer, int size) override;
    bool IsFinished() override;
    bool Seek(std::streamoff offset, std::ios::seekdir origin) override;
    void GetFormat(int& freq, AudioFormats& format, int& chans) override;
    void SetVolume(uint8_t volume) override;
    uint8_t GetVolume() override;
};

#define N3DS_PLATFORM_AUDIO
#endif