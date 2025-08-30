#include "audio.hpp"
#include <tremor/ivorbisfile.h>
#include <tremor/ivorbiscodec.h>
#include <cstring>
#include "adventureUtil.hpp"
#include "error.hpp"

static size_t vio_read_func(void *ptr, size_t size,size_t nmemb,void* userdata) {
	auto* f = reinterpret_cast<std::ifstream*>(userdata);
	if (size == 0) return 0;
	return f->read(reinterpret_cast<char*>(ptr), size*nmemb).gcount()/size;
}

static int vio_seek_func(void* userdata, ogg_int64_t offset, int seek_type) {
	auto* f = reinterpret_cast<std::ifstream*>(userdata);
	if (f->eof()) f->clear(); //emulate behaviour of fseek

	f->seekg(offset, CSeekdirToCppSeekdir(seek_type));

	return f->tellg();
}

static long vio_tell_func(void* userdata) {
	auto* f = reinterpret_cast<std::ifstream*>(userdata);
	return f->tellg();
}

static int vio_close_func(void* stream) {
    auto* f = reinterpret_cast<std::ifstream*>(stream);
    f->close();
    return 0;
}

static ov_callbacks vio = {
	vio_read_func,
	vio_seek_func,
	vio_close_func,
	vio_tell_func
};

VorbisDecoder::VorbisDecoder(std::ifstream* fileStream) : fileStream(fileStream) {
    musicType = "Vorbis";
}

VorbisDecoder::~VorbisDecoder() {
    if (ovf) {
        ov_clear(ovf);
        delete ovf;
    }
    delete fileStream;
}

bool VorbisDecoder::Open() {
    finished = false;
    if (ovf) {
        ov_clear(ovf);
        delete ovf;
    }
    ovf = new OggVorbis_File;

    int res = ov_open_callbacks(fileStream, ovf, nullptr, 0, vio);
    if (res < 0) {
        setErr("VorbisDecoder: Error reading file");
        delete ovf;
        return false;
    }

    vorbis_info* vi = ov_info(ovf, -1);
    if (!vi) {
        setErr("VorbisDecoder: Error getting file info");
        ov_clear(ovf);
        delete ovf;
        return false;
    }

    frequency = vi->rate;
    channels = vi->channels;

    vorbis_comment* vc = ov_comment(ovf, -1);
    if (vc) {
        const char* str = vorbis_comment_query(vc, (char*)"LOOPSTART", 0);
        if (str) {
            auto total = ov_pcm_total(ovf, -1);
            loopState.start = std::min<int64_t>(atoi(str), total);
            if (loopState.start >= 0) {
                loopState.looping = true;
                loopState.end = total;
                str = vorbis_comment_query(vc, (char*)"LOOPLENGTH", 0);
                if (str) {
                    int len = atoi(str);
                    if (len >= 0) {
                        loopState.end = std::min<int64_t>(loopState.start + len, total);
                    }
                }
                else {
                    str = vorbis_comment_query(vc, (char*)"LOOPEND", 0);
                    if (str) {
                        int end = atoi(str);
                        if (end >= 0) {
                            loopState.end = Clamp<int64_t>(end, loopState.start, total);
                        }
                    }
                }

                if (loopState.start == total) {
                    loopState.end = total;
                }
            }
        }
    }

    if (!loopState.looping) {
        loopState.start = 0;
        loopState.end = -1;
    }

    return true;
}

bool VorbisDecoder::Seek(std::streamoff offset, std::ios::seekdir origin) {
    if (offset == 0 && origin == std::ios::beg) {
        finished = false;

        if (ovf) {
            ov_pcm_seek(ovf, loopState.start);
        }

        if (loopState.looping && loopState.start == loopState.end) {
            loopState.to_end = true;
        }

        return true;
    }

    return false;
}

bool VorbisDecoder::IsFinished() {
    if (!ovf) return false;

    if (loopState.to_end) return false;

    return finished;
}

void VorbisDecoder::GetFormat(int& freq, AudioFormats& format, int& chans) {
    freq = frequency;
    format = FORMAT_S16;
    chans = channels;
}

int VorbisDecoder::FillBuffer(uint8_t* buffer, int size) {
    if (!ovf) return -1;

    if (loopState.to_end) {
        memset(buffer, '\0', size);
        return size;
    }

    static int section = 0; // TRY THIS
    int read = 0;
    int to_read = size;

    do {
        read = ov_read(ovf, reinterpret_cast<char*>(buffer + size - to_read), to_read, &section);

        if (read <= 0) break;

        if (loopState.looping) {
            auto pos = ov_pcm_tell(ovf);
            if (pos >= loopState.end) {
                finished = true;
                break;
            }
        }

        to_read -= read;
    } while (to_read > 0);

    if (read == 0) finished = true;

    if (read < 0) {
        setErr("VorbisDecoder::FillBuffer: Error reading file");
        return -1;
    }

    return size - to_read;
}

void VorbisDecoder::SetVolume(uint8_t volume) {
    this->volume = volume;
}

uint8_t VorbisDecoder::GetVolume() {
    return volume;
}