#include "audio.hpp"
#include <opusfile.h>
#include <memory>
#include <fstream>
#include <cstring>
#include "error.hpp"
#include "adventureUtil.hpp"

// inspiration at taken from https://github.com/EasyRPG/Player
// Check out src/decoder_opus.cpp

static int vio_read_func(void* stream, unsigned char* ptr, int nbytes) {
	auto* f = reinterpret_cast<std::ifstream*>(stream);
	if (nbytes == 0) return 0;
	return (int)(f->read(reinterpret_cast<char*>(ptr), nbytes).gcount());
}

static int vio_seek_func(void* stream, opus_int64 offset, int whence) {
	auto* f = reinterpret_cast<std::ifstream*>(stream);
	if (f->eof()) f->clear(); // emulate behaviour of fseek

	f->seekg(offset, CSeekdirToCppSeekdir(whence));

	return 0;
}

static opus_int64 vio_tell_func(void* stream) {
	auto* f = reinterpret_cast<std::ifstream*>(stream);
	return static_cast<opus_int64>(f->tellg());
}

static int vio_close_func(void* stream) {
    auto* f = reinterpret_cast<std::ifstream*>(stream);
    f->close();
    return 0;
}

static OpusFileCallbacks vio = {
	vio_read_func,
	vio_seek_func,
	vio_tell_func,
	vio_close_func
};

OpusDecoder::OpusDecoder(std::ifstream* fileStream) : fileStream(fileStream) {
    musicType = "Opus";
}

OpusDecoder::~OpusDecoder() {
    if (oof) {
        op_free(oof);
    }
    delete fileStream;
}

bool OpusDecoder::Open() {
    int res;

	if (!fileStream->good()) {
		setErr("OpusDecoder: Invalid file stream");
		return false;
	}

    oof = op_open_callbacks(fileStream, &vio, nullptr, 0, &res);
    if (res != 0) {
        setErr("OpusDecoder: Error reading file: " + std::string(opus_strerror(res)));
        op_free(oof);
        return false;
    }

	const OpusTags* ot = op_tags(oof, -1);
	if (ot) {
		const char* str = opus_tags_query(ot, "LOOPSTART", 0);
		if (str) {
			auto total = op_pcm_total(oof, -1);
			loopState.start = std::min<int64_t>(atoi(str), total);
			if (loopState.start >= 0) {
				loopState.looping = true;
				loopState.end = total;
				str = opus_tags_query(ot, "LOOPLENGTH", 0);
				if (str) {
					int len = atoi(str);
					if (len >= 0) {
						loopState.end = std::min<int64_t>(loopState.start + len, total);
					}
					else {
						str = opus_tags_query(ot, "LOOPEND", 0);
						if (str) {
							int end = atoi(str);
							if (end >= 0) {
								loopState.end = Clamp((int64_t)end, loopState.start, (int64_t)total);
							}
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

int OpusDecoder::FillBuffer(uint8_t* buffer, int size) {
	if (!oof) {
		return -1;
	}

	memset(buffer, '\0', size);

	if (loopState.to_end) {
		return size;
	}

	int length_16 = size / 2;
	opus_int16* buffer_16 = reinterpret_cast<opus_int16*>(buffer);

	int read = 0;
	int to_read = length_16;

	do {
		read = op_read_stereo(oof, buffer_16 + (length_16 - to_read), to_read);

		if (read <= 0) break;

		if (loopState.looping) {
			auto pos = op_pcm_tell(oof);
			if (pos >= loopState.end) {
				finished = true;
				break;
			}
		}

		to_read -= read * 2;
	} while (to_read > 0);

	if (read == 0) finished = true;

	if (read < 0) {
		return -1;
	}

	return (length_16 - to_read) * 2;
}

bool OpusDecoder::IsFinished() {
	return this->finished;
}

bool OpusDecoder::Seek(std::streamoff offset, std::ios::seekdir origin) {
	if (offset == 0 && origin == std::ios::beg) {
		finished = false;

		if (oof) {
			op_pcm_seek(oof, loopState.start);
		}

		if (loopState.looping && loopState.start == loopState.end) {
			loopState.to_end = true;
		}

		return true;
	}

	return false;
}

void OpusDecoder::GetFormat(int& freq, AudioFormats& format, int& chans) {
	freq = frequency;
	format = FORMAT_S16;
	chans = channels;
}

void OpusDecoder::SetVolume(uint8_t volume) {
	this->volume = volume;
}

uint8_t OpusDecoder::GetVolume() {
	return this->volume;
}