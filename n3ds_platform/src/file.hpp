#pragma once
#ifndef N3DS_PLATFORM_FILE
#include <string>

const void* readFile(std::string path, size_t* fileSize);

#define N3DS_PLATFORM_FILE
#endif