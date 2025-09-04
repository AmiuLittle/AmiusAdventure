#pragma once
#ifndef N3DS_SPRITE_MAKER_TYPES
#include <stdint.h>

#define N3DS_ANIMATION_LIST_MAGIC1 0x33445320 // "3DS " in ASCII
#define N3DS_ANIMATION_LIST_MAGIC2 0x414C5354 // "ALST" in ASCII

struct AlstHeader {
    uint32_t magic1; // should always be set to N3DS_ANIMATION_LIST_MAGIC1
    uint32_t magic2; // should always be set to N3DS_ANIMATION_LIST_MAGIC2
    uint8_t version;
};

namespace Version1 {
    
}

#define N3DS_SPRTIE_MAKER_TYPES
#endif