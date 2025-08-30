#pragma once
#ifndef AMIUS_ADVENTURE_UTIL
#include <ios>

constexpr std::ios_base::seekdir CSeekdirToCppSeekdir(int origin) {
	switch (origin) {
		case SEEK_SET:
			return std::ios_base::beg;
		case SEEK_CUR:
			return std::ios_base::cur;
		case SEEK_END:
			return std::ios_base::end;
		default:
			return std::ios_base::beg;
	}
}

template <typename T>
constexpr T Clamp(T value, const T& minv, const T& maxv) {
	return (value < minv) ? (minv) : ((value > maxv) ? maxv : value);
}

#define AMIUS_ADVENTURE_UTIL
#endif